from ns import ns

ns.LogComponentEnable("UdpEchoClientApplication", ns.LOG_LEVEL_INFO)
ns.LogComponentEnable("AlohaMac", ns.LOG_LEVEL_INFO)

nodes = ns.NodeContainer()
nodes.Create(3)

allocator = ns.CreateObject[ns.ListPositionAllocator]()
allocator.Add(ns.Vector(0,0,0))
allocator.Add(ns.Vector(50,0,0))
allocator.Add(ns.Vector(100,0,0))

mobility = ns.MobilityHelper()
mobility.SetPositionAllocator(allocator)
mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel")
mobility.Install(nodes)

aloha = ns.AlohaHelper()
aloha.SetChannelAttribute("TransmissionRange", ns.DoubleValue(100.0))
aloha.SetChannelAttribute("DataRate", ns.StringValue("10Mbps"))
aloha.SetDeviceAttribute("SinkAddress", ns.Mac48AddressValue("00:00:00:00:00:02"))
aloha.SetPhyAttribute("PacketErrorRate", ns.DoubleValue(0.0))
devices = aloha.Install(nodes)

internet = ns.InternetStackHelper()
internet.Install(nodes)

address = ns.Ipv4AddressHelper()
address.SetBase(ns.Ipv4Address("10.0.0.0"), ns.Ipv4Mask("255.255.255.0"))

interfaces = address.Assign(devices)
echoServer = ns.UdpEchoServerHelper(9)

serverApps = echoServer.Install(nodes.Get(1))
serverApps.Start(ns.Seconds(1.0))
serverApps.Stop(ns.Seconds(10.0))

sinkAddress = interfaces.GetAddress(1).ConvertTo()
echoClient = ns.UdpEchoClientHelper(sinkAddress, 9)
echoClient.SetAttribute("MaxPackets", ns.UintegerValue(10))
echoClient.SetAttribute("Interval", ns.TimeValue(ns.MilliSeconds(100.0)))
echoClient.SetAttribute("PacketSize", ns.UintegerValue(128))

clients = ns.NodeContainer()
clients.Add(nodes.Get(0))
clients.Add(nodes.Get(1))

clientApps = echoClient.Install(clients)
clientApps.Start(ns.Seconds(1.0))
clientApps.Stop(ns.Seconds(4.0))

ns.Simulator.Run()
ns.Simulator.Destroy()
