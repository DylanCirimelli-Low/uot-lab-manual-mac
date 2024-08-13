#include "ns3/simulator.h"
#include "ns3/callback.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/boolean.h"
#include "ns3/log.h"
#include "ns3/object-base.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/enum.h"
#include "ns3/ipv4-address.h"
#include "ns3/mac16-address.h"
#include "ns3/ipv4-l3-protocol.h"

#include "ns3/aloha-net_device.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AlohaNetDevice");
NS_OBJECT_ENSURE_REGISTERED (AlohaNetDevice);

TypeId
AlohaNetDevice::GetTypeId (void)
{
    static TypeId
    tid = TypeId ("ns3::AlohaNetDevice")
    .SetParent<NetDevice> ()
    .SetGroupName("Wireless")
    .AddConstructor<AlohaNetDevice> ()
	.AddAttribute ("Phy",
			"The PHY layer attached to this device.",
			PointerValue (),
			MakePointerAccessor (&AlohaNetDevice::GetPhy, &AlohaNetDevice::SetPhy),
			MakePointerChecker<WirelessPhy> ())
	.AddAttribute ("Channel",
			"The channel attached to this device.",
			PointerValue (),
			MakePointerAccessor (&AlohaNetDevice::GetChannel, &AlohaNetDevice::Attach),
			MakePointerChecker<WirelessChannel> ())
    .AddAttribute ("MinBackoffExponent",
            "The minimum exponent of the backoff window",
            UintegerValue(4),
            MakeUintegerAccessor (&AlohaNetDevice::SetMinBackoffExponent),
            MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxBackoffExponent",
            "The max exponent of the backoff window",
            UintegerValue(8),
            MakeUintegerAccessor (&AlohaNetDevice::SetMaxBackoffExponent),
            MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("SinkAddress",
            "The address of the sink node",
            Mac48AddressValue("00:00:00:00:00:01"),
            MakeMac48AddressAccessor (&AlohaNetDevice::SetSinkAddress),
            MakeMac48AddressChecker())
    ;

    return (tid);
}

AlohaNetDevice::AlohaNetDevice (void)
{
    m_linkUp = false;
    m_ifIndex = 0;
    m_mtu = 1500;
    m_mac = CreateObject<AlohaMac>();
    m_mac->SetReceiveCallback (MakeCallback (&AlohaNetDevice::Receive, this));
}

AlohaNetDevice::~AlohaNetDevice (void){
}

void
AlohaNetDevice::DoDispose (void)
{
    m_node = 0;
    m_mac = 0;
    NetDevice::DoDispose();
}

void
AlohaNetDevice::Receive(Ptr<const Packet> packet, const Address& address)
{
    NS_LOG_DEBUG("Recevied packet " << packet << " from " << address);
    m_networkUpcall(this, packet, 0, address);  
}

bool
AlohaNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
    NS_LOG_DEBUG("sending packet " << *packet << " from " << GetAddress() << " to address " << dest << "(proto=" << protocolNumber << ")");
    NS_ASSERT(m_mac);
    
    return m_mac->Send(packet);
}

void
AlohaNetDevice::SetPhy(Ptr<WirelessPhy> phy)
{
    m_mac->SetPhy(phy);
}

Ptr<WirelessPhy>
AlohaNetDevice::GetPhy (void) const
{
    NS_ASSERT(m_mac->GetPhy());
    return m_mac->GetPhy();
}

Ptr<AlohaMac>
AlohaNetDevice::GetMac(void) const
{
    return m_mac;
}

Address
AlohaNetDevice::GetMulticast (Ipv4Address addr) const
{
   	Ipv4Address a = addr.CombineMask(Ipv4Mask(16));
	Ipv4Address expected = Ipv4Address(0xE0030000);
	if (expected.Get() == a.Get())
        return addr;
     else {
		NS_FATAL_ERROR("Multicast group must be 224.3.0.0/16 group");
	}
}

Address
AlohaNetDevice::GetMulticast (Ipv6Address addr) const
{
    NS_FATAL_ERROR("IPv6 multicast not supported");
}

void
AlohaNetDevice::SetIfIndex(const uint32_t index)
{
    m_ifIndex = index;
}

uint32_t
AlohaNetDevice::GetIfIndex (void) const
{
    return m_ifIndex;
}

Ptr<Channel>
AlohaNetDevice::GetChannel (void) const
{   
    NS_ASSERT(m_mac->GetPhy());
    NS_ASSERT(m_mac->GetPhy()->GetChannel());

    return m_mac->GetPhy()->GetChannel();
}

void
AlohaNetDevice::Attach(Ptr<WirelessChannel> channel)
{
    NS_ASSERT(m_mac);
	m_mac->GetPhy()->SetChannel(channel);
}

bool
AlohaNetDevice::SetMtu (const uint16_t mtu)
{
    m_mtu = mtu;
}

uint16_t
AlohaNetDevice::GetMtu (void) const
{
    return m_mtu;
}

void
AlohaNetDevice::SetMinBackoffExponent(uint32_t minBackoffExp)
{
    NS_ASSERT(m_mac);
    m_mac->SetMinBackoffExponent(minBackoffExp);
}

void
AlohaNetDevice::SetMaxBackoffExponent(uint32_t maxBackoffExp)
{
    NS_ASSERT(m_mac);
    m_mac->SetMaxBackoffExponent(maxBackoffExp);
}

void
AlohaNetDevice::SetSinkAddress(Mac48Address sinkAddress)
{
    NS_ASSERT(m_mac);
    m_mac->SetSinkAddress(sinkAddress);
}

bool
AlohaNetDevice::IsLinkUp (void) const
{
    return ((m_mac->GetPhy()) && m_linkUp);
}

bool
AlohaNetDevice::IsBroadcast (void) const
{
    return false;
}

Address
AlohaNetDevice::GetBroadcast (void) const
{
    return Mac48Address ("FF:FF:FF:FF:FF:FF");
}

bool
AlohaNetDevice::IsMulticast (void) const
{
    return true;
}

bool
AlohaNetDevice::IsPointToPoint (void) const
{
    return false;
}

void
AlohaNetDevice::SetNode (Ptr<Node> node)
{
    m_node = node;
}

Ptr<Node>
AlohaNetDevice::GetNode (void) const
{
    return m_node;
}

bool
AlohaNetDevice::NeedsArp (void) const
{
    return false;
}

void
AlohaNetDevice::SetReceiveCallback (ReceiveCallback callback)
{
    m_networkUpcall = callback;
}

void
AlohaNetDevice::SetPromiscReceiveCallback(PromiscReceiveCallback callback)
{
    m_promiscuousReceive = callback;
}

void
AlohaNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
    NS_FATAL_ERROR("Not implemented");
}

bool
AlohaNetDevice::SupportsSendFrom (void) const
{
    return false;
}

bool
AlohaNetDevice::SendFrom(Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
    NS_FATAL_ERROR("Not implemented");
}

void 
AlohaNetDevice::SetAddress(Address address)
{
    NS_ASSERT(m_mac);
    NS_LOG_DEBUG("" << Mac48Address::ConvertFrom(address));
    m_mac->SetAddress(address);
}

Address
AlohaNetDevice::GetAddress(void) const
{
    NS_ASSERT(m_mac);
    return m_mac->GetAddress();
}

bool
AlohaNetDevice::IsBridge(void) const
{
    return false;
}

int64_t
AlohaNetDevice::AssignStreams(int64_t stream)
{
        int64_t currentStream = stream;
        currentStream += m_mac->AssignStreams(stream);
        return (currentStream - stream);
}

} /* namespace ns3 */