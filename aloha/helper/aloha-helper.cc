#include "aloha-helper.h"
#include "ns3/log.h"
#include "ns3/wireless-phy.h"
#include "ns3/aloha-mac.h"
#include "ns3/wireless-channel.h"
#include "ns3/aloha-net_device.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AlohaHelper");

std::map<uint32_t, Time> AlohaHelper::m_delays;

AlohaHelper::AlohaHelper() {
	m_deviceFactory.SetTypeId ("ns3::AlohaNetDevice");
	m_phyFactory.SetTypeId ("ns3::WirelessPhy");
	m_channelFactory.SetTypeId ("ns3::WirelessChannel");
}

AlohaHelper::~AlohaHelper() {
}

void
AlohaHelper::SetDeviceAttribute (std::string name, const AttributeValue &value)
{
	m_deviceFactory.Set (name, value);
}

void
AlohaHelper::SetPhyAttribute (std::string name, const AttributeValue &value)
{
    m_phyFactory.Set (name, value);
}

void
AlohaHelper::SetChannelAttribute (std::string name, const AttributeValue &value)
{
    m_channelFactory.Set (name, value);
}


NetDeviceContainer
AlohaHelper::Install (Ptr<Node> node, Ptr<WirelessChannel> channel) const
{
	NS_LOG_DEBUG("Node " << node->GetId() << " adding to channel " << channel);

	auto device = m_deviceFactory.Create<AlohaNetDevice>();
	device->SetAddress(Mac48Address::Allocate ());
	device->SetNode(node);

	auto phy = m_phyFactory.Create<WirelessPhy> ();
	phy->SetChannel(channel);
	phy->SetDevice(device);
	phy->SetMobility(node->GetObject<MobilityModel>());

	device->SetPhy(phy);
	node->AddDevice(device);
	
	return NetDeviceContainer(device);
}

NetDeviceContainer
AlohaHelper::Install (const NodeContainer &container, Ptr<WirelessChannel> channel) const
{
	NetDeviceContainer devs;
	for (NodeContainer::Iterator i = container.Begin (); i != container.End (); i++) {
		devs.Add (Install (*i, channel));
	}

	return devs;
}

NetDeviceContainer
AlohaHelper::Install (const NodeContainer &container) const
{
	auto channel = m_channelFactory.Create<WirelessChannel>();
	NS_LOG_DEBUG("Create channel " << channel);
	return Install(container, channel);
}

int64_t
AlohaHelper::AssignStreams (NetDeviceContainer c, int64_t stream)
{
	int64_t currentStream = stream;
	Ptr<NetDevice> netDevice;
	for (NetDeviceContainer::Iterator i = c.Begin (); i != c.End (); ++i) {
		netDevice = (*i);
		Ptr<AlohaNetDevice> alohadevice = DynamicCast<AlohaNetDevice> (netDevice);
		
		NS_ASSERT(alohadevice);
		if (alohadevice) {
			currentStream += alohadevice->AssignStreams (currentStream);
		}
	}
	return (currentStream - stream);
}

void AlohaHelper::ReceiveSinkWithContext(Ptr<OutputStreamWrapper> stream,
                                                std::string context,
                                                Ptr<const Packet> p)
{	
	AlohaMacPacketTag tag;
	p->PeekPacketTag(tag);

	NS_ASSERT_MSG(m_delays.count(tag.GetPacketUid()) == true, "This packet should have been enqueued at a node.");

	Time delay = Simulator::Now() - m_delays.at(tag.GetPacketUid());
    NS_LOG_FUNCTION(stream << p);
    *stream->GetStream() << "r " << Simulator::Now().GetSeconds() << " " << context << " " << delay.GetSeconds() << " " << tag.GetPacketSize()
                         << std::endl;

	
}

void AlohaHelper::EnqueueSinkWithContext(Ptr<OutputStreamWrapper> stream,
                                                std::string context,
                                                Ptr<const Packet> p)
{	
	AlohaMacPacketTag tag;
	p->PeekPacketTag(tag);

    NS_LOG_FUNCTION(stream << p);
    *stream->GetStream() << "+ " << Simulator::Now().GetSeconds() << " " << context << " "
                         << std::endl;
	
	NS_ASSERT_MSG(m_delays.count(p->GetUid()) == false, "We have already enqueued this packet UID somewhere");
	m_delays[p->GetUid()] = Simulator::Now();
}

void
AlohaHelper::EnableAsciiInternal(Ptr<OutputStreamWrapper> stream,
                                std::string prefix,
                                Ptr<NetDevice> nd,
                                bool explicitFilename)
{
    //
    // All of the ascii enable functions vector through here including the ones
    // that are wandering through all of devices on perhaps all of the nodes in
    // the system.  We can only deal with devices of type CsmaNetDevice.
    //
    Ptr<AlohaNetDevice> device = nd->GetObject<AlohaNetDevice>();
    if (!device)
    {
        NS_LOG_INFO("AlohaHelper::EnableAsciiInternal(): Device "
                    << device << " not of type ns3::AlohaNetDevice");
        return;
    }

    //
    // Our default trace sinks are going to use packet printing, so we have to
    // make sure that is turned on.
    //
    // Packet::EnablePrinting();

    //
    // If we are not provided an OutputStreamWrapper, we are expected to create
    // one using the usual trace filename conventions and do a Hook*WithoutContext
    // since there will be one file per context and therefore the context would
    // be redundant.
    //
    if (!stream)
    {
		NS_FATAL_ERROR("No OutputStreamWrapper provided");
    }

	AsciiTraceHelper asciiTraceHelper;

	auto mac = device->GetMac();
	NS_ASSERT_MSG(mac, "Attempted to attach trace to uninitialized device");

	std::string nodeName = std::to_string(nd->GetNode()->GetId());

    bool result = mac->TraceConnect("AckReceive",
                                       nodeName,
                                       MakeBoundCallback(&AlohaHelper::ReceiveSinkWithContext, stream));
    NS_ASSERT_MSG(result == true,
                  "Unable to hook \""
                      << "AckReceive" << "\"");

	result = mac->TraceConnect("Enqueue",
										nodeName,
										MakeBoundCallback(&AlohaHelper::EnqueueSinkWithContext, stream));
	NS_ASSERT_MSG(result == true,
				" Unable to hook \""
					<< "AckReceive" << "\"");

	// asciiTraceHelper.HookDefaultReceiveSinkWithContext<AlohaMac>(mac, nodeName, "AckReceive", stream);
	// asciiTraceHelper.HookDefaultEnqueueSinkWithContext<AlohaMac>(mac, nodeName, "Enqueue", stream);
	// asciiTraceHelper.HookDefaultDropSinkWithoutContext<AlohaMac>(mac, "Drop", stream);
	// asciiTraceHelper.HookDefaultDequeueSinkWithoutContext<AlohaMac>(mac, "Dequeue", stream);
}

} /* namespace ns3 */
