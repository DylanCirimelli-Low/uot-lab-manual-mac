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

class AlohaHelper : public AsciiTraceHelperForDevice {
public:
	AlohaHelper();
	virtual ~AlohaHelper();

	void SetDeviceAttribute (std::string name, const AttributeValue &value);
    void SetPhyAttribute (std::string name, const AttributeValue &value);
	void SetChannelAttribute (std::string name, const AttributeValue &value);

	NetDeviceContainer Install (Ptr<Node> node, Ptr<WirelessChannel> channel) const;
	NetDeviceContainer Install (const NodeContainer &container) const;
	NetDeviceContainer Install (const NodeContainer &container, Ptr<WirelessChannel> channel) const;

    static void ReceiveSinkWithContext(Ptr<OutputStreamWrapper> stream,
                                                    std::string context,
                                                    Ptr<const Packet> p);
                                                    
    static void EnqueueSinkWithContext(Ptr<OutputStreamWrapper> stream,
                                                    std::string context,
                                                    Ptr<const Packet> p);    
    /**
     * \brief Enable ascii trace output on the indicated net device.
     *
     * NetDevice-specific implementation mechanism for hooking the trace and
     * writing to the trace file.
     *
     * \param stream The output stream object to use when logging ascii traces.
     * \param prefix Filename prefix to use for ascii trace files.
     * \param nd Net device for which you want to enable tracing.
     * \param explicitFilename Treat the prefix as an explicit filename if true
     */
    void EnableAsciiInternal(Ptr<OutputStreamWrapper> stream,
                             std::string prefix,
                             Ptr<NetDevice> nd,
                             bool explicitFilename) override;

	int64_t AssignStreams (NetDeviceContainer c, int64_t stream);

    static std::map<uint32_t, Time> m_delays;
    
private:
                   
	ObjectFactory m_deviceFactory;
	ObjectFactory m_phyFactory;
	ObjectFactory m_channelFactory;
};

} /* namespace ns3 */

#endif /* SLOTTED_ALOHA_HELPER_H */
