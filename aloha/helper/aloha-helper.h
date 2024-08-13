#ifndef SLOTTED_ALOHA_HELPER_H
#define SLOTTED_ALOHA_HELPER_H

#include "ns3/trace-helper.h"
#include "ns3/attribute.h"
#include "ns3/object-factory.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/wireless-channel.h"
#include "ns3/wireless-phy.h"

namespace ns3 {

class AlohaHelper {
public:
	AlohaHelper();
	virtual ~AlohaHelper();

	void SetDeviceAttribute (std::string name, const AttributeValue &value);
    void SetPhyAttribute (std::string name, const AttributeValue &value);
	void SetChannelAttribute (std::string name, const AttributeValue &value);

	NetDeviceContainer Install (Ptr<Node> node, Ptr<WirelessChannel> channel) const;
	NetDeviceContainer Install (const NodeContainer &container) const;
	NetDeviceContainer Install (const NodeContainer &container, Ptr<WirelessChannel> channel) const;

	int64_t AssignStreams (NetDeviceContainer c, int64_t stream);

private:
	ObjectFactory m_deviceFactory;
	ObjectFactory m_phyFactory;
	ObjectFactory m_channelFactory;
};

} /* namespace ns3 */

#endif /* SLOTTED_ALOHA_HELPER_H */
