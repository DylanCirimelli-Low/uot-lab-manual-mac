#ifndef SLOTTED_ALOHA_NET_DEVICE_H
#define SLOTTED_ALOHA_NET_DEVICE_H

#include "ns3/net-device.h"
#include "ns3/event-id.h"
#include "ns3/mac48-address.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/callback.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/boolean.h"

#include "ns3/trace-source-accessor.h"
#include "ns3/enum.h"
#include "ns3/ipv4-address.h"
#include "ns3/mac16-address.h"
#include "ns3/ipv4-l3-protocol.h"

#include "ns3/wireless-mac-upcalls.h"
#include "ns3/aloha-mac.h"

namespace ns3 {

class Node;
class Channel;

class AlohaNetDevice : public NetDevice {

public:
    static TypeId GetTypeId (void);
    AlohaNetDevice (void);
    virtual ~AlohaNetDevice (void);

	virtual void SetIfIndex (const uint32_t index);
	virtual uint32_t GetIfIndex (void) const;

	void Attach(Ptr<WirelessChannel> channel);
	virtual Ptr<Channel> GetChannel (void) const;

	virtual void SetAddress (Address address);
	virtual Address GetAddress (void) const;

	virtual bool SetMtu (const uint16_t mtu);
	virtual uint16_t GetMtu (void) const;

	virtual bool IsLinkUp (void) const;

	virtual void AddLinkChangeCallback (Callback<void> callback);
	virtual bool IsBroadcast (void) const;
	virtual Address GetBroadcast (void) const;

	virtual bool IsMulticast (void) const;
	virtual Address GetMulticast (Ipv4Address addr) const;
	virtual Address GetMulticast (Ipv6Address addr) const;

	virtual bool IsBridge (void) const;
	virtual bool IsPointToPoint (void) const;

	virtual bool SupportsSendFrom (void) const;
	virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
    virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
    
	void Receive (Ptr<const Packet>, const Address& address); 

	virtual Ptr<Node> GetNode (void) const;
	virtual void SetNode (Ptr<Node> node);

	virtual bool NeedsArp (void) const;

	virtual void SetReceiveCallback (ReceiveCallback cb);
	virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);

	virtual void DoDispose (void);
	virtual int64_t AssignStreams (int64_t stream);

	void SetPhy(Ptr<WirelessPhy> phy);
	Ptr<WirelessPhy> GetPhy(void) const;
	Ptr<AlohaMac> GetMac(void) const;

	void SetMinBackoffExponent (uint32_t minBackoffExp);
	void SetMaxBackoffExponent (uint32_t maxBackoffExp);
	void SetSinkAddress(Mac48Address sinkAddress);

private:

	Ptr<Node> m_node;
	Ptr<AlohaMac> m_mac;
	
	NetDevice::ReceiveCallback m_networkUpcall;
	NetDevice::PromiscReceiveCallback m_promiscuousReceive;

	uint16_t m_mtu;
	uint32_t m_ifIndex;
	bool m_linkUp;
	Ptr<Object> m_mobility;

}; /* CLASS AlohaNetDevice */
} /* NAMESPACE NS3 */

#endif /* ALOHA_NET_DEVICE_H */
