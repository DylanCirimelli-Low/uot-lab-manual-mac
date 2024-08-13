#include "aloha-helper.h"
#include "ns3/log.h"
#include "ns3/wireless-phy.h"
#include "ns3/aloha-mac.h"
#include "ns3/wireless-channel.h"
#include "ns3/aloha-net_device.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AlohaHelper");

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

} /* namespace ns3 */
