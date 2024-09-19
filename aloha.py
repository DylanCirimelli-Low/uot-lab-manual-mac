from ns import ns
from ctypes import c_char_p, create_string_buffer
import sys

# ns.LogComponentEnable("UdpEchoClientApplication", ns.LOG_LEVEL_INFO)
ns.LogComponentEnable("AlohaMac", ns.LOG_LEVEL_INFO | ns.LOG_PREFIX_NODE | ns.LOG_PREFIX_TIME | ns.LOG_PREFIX_FUNC)
# ns.LogComponentEnable("AlohaNetDevice", ns.LOG_LEVEL_INFO | ns.LOG_PREFIX_NODE | ns.LOG_PREFIX_TIME | ns.LOG_PREFIX_FUNC)
# ns.LogComponentEnable("WirelessPhy", ns.LOG_LEVEL_DEBUG | ns.LOG_PREFIX_NODE | ns.LOG_PREFIX_TIME | ns.LOG_PREFIX_FUNC)
# ns.LogComponentEnable("WirelessChannel", ns.LOG_LEVEL_DEBUG | ns.LOG_PREFIX_NODE | ns.LOG_PREFIX_TIME | ns.LOG_PREFIX_FUNC)

BUFFLEN = 4096
topologyBuffer = create_string_buffer(b"", BUFFLEN)
topologyFilePtr = c_char_p(topologyBuffer.raw)

cmd = ns.CommandLine(__file__)
cmd.AddValue(
    "topology",
    "The topology file containing the coordinates of each node",
    topologyFilePtr,
    BUFFLEN
)
cmd.Parse(sys.argv)

topologyFilePath = topologyFilePtr.value.decode()
assert topologyFilePath != ""

ns.Config.SetDefault("ns3::ConfigStore::Filename", ns.StringValue("attributes.txt"))
ns.Config.SetDefault("ns3::ConfigStore::FileFormat", ns.StringValue("RawText"))
ns.Config.SetDefault("ns3::ConfigStore::Mode", ns.StringValue("Load"))
outputConfig = ns.ConfigStore()
outputConfig.ConfigureDefaults()
outputConfig.ConfigureAttributes()



numNodes = len(open(topologyFilePath, "r").readlines())
nodes = ns.NodeContainer(numNodes)
allocator = ns.CreateObject[ns.ListPositionAllocator]()

for line in open(topologyFilePath, "r").readlines():
    xyz = line.strip().split(' ')
    allocator.Add(ns.Vector3D(
        float(xyz[0]),
        float(xyz[1]),
        float(xyz[2])
    ))

mobility = ns.MobilityHelper()
mobility.SetPositionAllocator(allocator)
mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel")
mobility.Install(nodes)

aloha = ns.AlohaHelper()
ascii = ns.AsciiTraceHelper()
stream = ascii.CreateFileStream("aloha.tr")
devices = aloha.Install(nodes)
aloha.EnableAsciiAll(stream)

internet = ns.InternetStackHelper()
internet.Install(nodes)

address = ns.Ipv4AddressHelper()
address.SetBase(ns.Ipv4Address("10.0.0.0"), ns.Ipv4Mask("255.255.255.0"))

interfaces = address.Assign(devices)

sinkAddress = interfaces.GetAddress(0).ConvertTo()
echoClient = ns.UdpEchoClientHelper(sinkAddress, 9)
clientApps = echoClient.Install(nodes)

ns.Simulator.Stop(ns.Seconds(10))
ns.Simulator.Run()
ns.Simulator.Destroy()
