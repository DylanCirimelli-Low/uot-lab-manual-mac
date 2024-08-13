#include "wireless-net_device.h"
#include "wireless-channel.h"
#include "wireless-phy.h"
#include "wireless-mac-upcalls.h"

#include "ns3/boolean.h"
#include "ns3/enum.h"
#include "ns3/error-model.h"
#include "ns3/ethernet-header.h"
#include "ns3/ethernet-trailer.h"
#include "ns3/llc-snap-header.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"
#include "ns3/drop-tail-queue.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("WirelessNetDevice");

NS_OBJECT_ENSURE_REGISTERED(WirelessNetDevice);

TypeId
WirelessNetDevice::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::WirelessNetDevice")
            .SetParent<NetDevice>()
            .SetGroupName("Wireless")
            .AddConstructor<WirelessNetDevice>()
            .AddAttribute("Address",
                          "The MAC address of this device.",
                          Mac48AddressValue(Mac48Address("ff:ff:ff:ff:ff:ff")),
                          MakeMac48AddressAccessor(&WirelessNetDevice::m_address),
                          MakeMac48AddressChecker())
            .AddAttribute("Mtu",
                          "The MAC-level Maximum Transmission Unit",
                          UintegerValue(DEFAULT_MTU),
                          MakeUintegerAccessor(&WirelessNetDevice::SetMtu, &WirelessNetDevice::GetMtu),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("TxQueue",
                          "A queue to use as the transmit queue in the device.",
                          PointerValue(),
                          MakePointerAccessor(&WirelessNetDevice::m_queue),
                          MakePointerChecker<Queue<Packet>>());
    return tid;
}

WirelessNetDevice::WirelessNetDevice()
    : m_linkUp(false)
{
    NS_LOG_FUNCTION(this);
    m_txMachineState = READY;
    m_phy = nullptr;
    m_queue = CreateObject<DropTailQueue<Packet>>();
    m_macUpcalls = Create<WirelessMacUpcalls> ( MakeCallback  (&WirelessNetDevice::Receive, this),
                                                MakeNullCallback<void> (),
                                                MakeNullCallback<void> (),
                                                MakeCallback (&WirelessNetDevice::TransmitCompleteEvent, this));
}

WirelessNetDevice::~WirelessNetDevice()
{
    NS_LOG_FUNCTION_NOARGS();
    m_queue = nullptr;
}

void
WirelessNetDevice::DoDispose()
{
    NS_LOG_FUNCTION_NOARGS();
    m_phy = nullptr;
    m_node = nullptr;
    m_queue = nullptr;
    NetDevice::DoDispose();
}

bool
WirelessNetDevice::SetMtu(uint16_t mtu)
{
    NS_LOG_FUNCTION(this << mtu);
    m_mtu = mtu;
    NS_LOG_LOGIC("m_mtu = " << m_mtu);

    return true;
}

uint16_t
WirelessNetDevice::GetMtu() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_mtu;
}

void
WirelessNetDevice::AddHeader(Ptr<Packet> p,
                         Mac48Address source,
                         Mac48Address dest,
                         uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(p << source << dest << protocolNumber);

    EthernetHeader header(false);
    header.SetSource(source);
    header.SetDestination(dest);

    EthernetTrailer trailer;

    NS_LOG_LOGIC("p->GetSize () = " << p->GetSize());
    NS_LOG_LOGIC("m_mtu = " << m_mtu);

    uint16_t lengthType = 0;

    NS_LOG_LOGIC("Encapsulating packet as LLC (length interpretation)");

    LlcSnapHeader llc;
    llc.SetType(protocolNumber);
    p->AddHeader(llc);

    //
    // This corresponds to the length interpretation of the lengthType
    // field but with an LLC/SNAP header added to the payload as in
    // IEEE 802.2
    //
    lengthType = p->GetSize();

    //
    // All Ethernet frames must carry a minimum payload of 46 bytes.  The
    // LLC SNAP header counts as part of this payload.  We need to padd out
    // if we don't have enough bytes.  These must be real bytes since they
    // will be written to pcap files and compared in regression trace files.
    //
    if (p->GetSize() < 46)
    {
        uint8_t buffer[46];
        memset(buffer, 0, 46);
        Ptr<Packet> padd = Create<Packet>(buffer, 46 - p->GetSize());
        p->AddAtEnd(padd);
    }

    NS_ASSERT_MSG(p->GetSize() <= GetMtu(),
                "WirelessNetDevice::AddHeader(): 802.3 Length/Type field with LLC/SNAP: "
                "length interpretation must not exceed device frame size minus overhead");
 

    NS_LOG_LOGIC("header.SetLengthType (" << lengthType << ")");
    header.SetLengthType(lengthType);
    p->AddHeader(header);

    if (Node::ChecksumEnabled())
    {
        trailer.EnableFcs(true);
    }
    trailer.CalcFcs(p);
    p->AddTrailer(trailer);
}

void
WirelessNetDevice::TransmitStart()
{
    NS_LOG_FUNCTION_NOARGS();

    //
    // This function is called to start the process of transmitting a packet.  We
    // expect that the packet to transmit will be found in m_currentPkt.
    //
    NS_ASSERT_MSG(m_currentPkt, "WirelessNetDevice::TransmitStart(): m_currentPkt not set");

    NS_LOG_LOGIC("m_currentPkt = " << m_currentPkt);
    NS_LOG_LOGIC("UID = " << m_currentPkt->GetUid());

    NS_ASSERT_MSG((m_txMachineState == READY),
                  "Must be READY to transmit. Tx state is: " << m_txMachineState);

    m_phy->Send(m_currentPkt);
    m_txMachineState = BUSY;
}

void
WirelessNetDevice::TransmitCompleteEvent()
{
    NS_LOG_FUNCTION_NOARGS();

    NS_ASSERT_MSG(m_txMachineState == BUSY,
                  "WirelessNetDevice::transmitCompleteEvent(): Must be BUSY if transmitting");
    m_txMachineState = GAP;
    m_currentPkt = nullptr;
    Simulator::Schedule(m_phy->GetInterframeGap(), &WirelessNetDevice::TransmitReadyEvent, this);
}

void
WirelessNetDevice::TransmitReadyEvent()
{
    NS_LOG_FUNCTION_NOARGS();

    //
    // This function is called to enable the transmitter after the interframe
    // gap has passed.  If there are pending transmissions, we use this opportunity
    // to start the next transmit.
    //
    NS_ASSERT_MSG(m_txMachineState == GAP,
                  "WirelessNetDevice::TransmitReadyEvent(): Must be in interframe gap");
    m_txMachineState = READY;

    //
    // We expect that the packet we had been transmitting was cleared when the
    // TransmitCompleteEvent() was executed.
    //
    NS_ASSERT_MSG(!m_currentPkt, "WirelessNetDevice::TransmitReadyEvent(): m_currentPkt nonzero");

    //
    // Get the next packet from the queue for transmitting
    //
    if (m_queue->IsEmpty())
    {
        return;
    }
    else
    {
        Ptr<Packet> packet = m_queue->Dequeue();
        NS_ASSERT_MSG(packet,
                      "WirelessNetDevice::TransmitReadyEvent(): IsEmpty false but no Packet on queue?");
        m_currentPkt = packet;
        TransmitStart();
    }
}

bool
WirelessNetDevice::Attach(Ptr<WirelessChannel> ch)
{
    NS_LOG_FUNCTION(this << &ch);

    m_channel = ch;

    //
    // This device is up whenever a channel is attached to it.
    //
    NotifyLinkUp();
    return true;
}

void
WirelessNetDevice::SetQueue(Ptr<Queue<Packet>> q)
{
    NS_LOG_FUNCTION(q);
    m_queue = q;
}

void
WirelessNetDevice::Receive(Ptr<Packet> packet)
{
    NS_LOG_LOGIC("UID is " << packet->GetUid());

    // //
    // // We never forward up packets that we sent.  Real devices don't do this since
    // // their receivers are disabled during send, so we don't.
    // //
    // if (senderDevice == this)
    // {
    //     return;
    // }

    Ptr<Packet> originalPacket = packet->Copy();

    EthernetTrailer trailer;
    packet->RemoveTrailer(trailer);
    if (Node::ChecksumEnabled())
    {
        trailer.EnableFcs(true);
    }

    bool crcGood = trailer.CheckFcs(packet);
    if (!crcGood)
    {
        NS_LOG_INFO("CRC error on Packet " << packet);
        return;
    }

    EthernetHeader header(false);
    packet->RemoveHeader(header);

    NS_LOG_LOGIC("Pkt source is " << header.GetSource());
    NS_LOG_LOGIC("Pkt destination is " << header.GetDestination());

    uint16_t protocol;
    //
    // If the length/type is less than 1500, it corresponds to a length
    // interpretation packet.  In this case, it is an 802.3 packet and
    // will also have an 802.2 LLC header.  If greater than 1500, we
    // find the protocol number (Ethernet type) directly.
    //
    if (header.GetLengthType() <= 1500)
    {
        NS_ASSERT(packet->GetSize() >= header.GetLengthType());
        uint32_t padlen = packet->GetSize() - header.GetLengthType();
        NS_ASSERT(padlen <= 46);
        if (padlen > 0)
        {
            packet->RemoveAtEnd(padlen);
        }

        LlcSnapHeader llc;
        packet->RemoveHeader(llc);
        protocol = llc.GetType();
    }
    else
    {
        protocol = header.GetLengthType();
    }

    //
    // Classify the packet based on its destination.
    //
    PacketType packetType;

    if (header.GetDestination().IsBroadcast())
    {
        packetType = PACKET_BROADCAST;
    }
    else if (header.GetDestination().IsGroup())
    {
        packetType = PACKET_MULTICAST;
    }
    else if (header.GetDestination() == m_address)
    {
        packetType = PACKET_HOST;
    }
    else
    {
        packetType = PACKET_OTHERHOST;
    }

    //
    // For all kinds of packetType we receive, we hit the promiscuous sniffer
    // hook and pass a copy up to the promiscuous callback.  Pass a copy to
    // make sure that nobody messes with our packet.
    //
    if (!m_promiscRxCallback.IsNull())
    {
        m_promiscRxCallback(this,
                            packet,
                            protocol,
                            header.GetSource(),
                            header.GetDestination(),
                            packetType);
    }

    //
    // If this packet is not destined for some other host, it must be for us
    // as either a broadcast, multicast or unicast.  We need to hit the mac
    // packet received trace hook and forward the packet up the stack.
    //
    if (packetType != PACKET_OTHERHOST)
    {
        m_rxCallback(this, packet, protocol, header.GetSource());
    }
}

Ptr<Queue<Packet>>
WirelessNetDevice::GetQueue() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_queue;
}

void
WirelessNetDevice::NotifyLinkUp()
{
    NS_LOG_FUNCTION_NOARGS();
    m_linkUp = true;
    m_linkChangeCallbacks();
}

void
WirelessNetDevice::SetIfIndex(const uint32_t index)
{
    NS_LOG_FUNCTION(index);
    m_ifIndex = index;
}

uint32_t
WirelessNetDevice::GetIfIndex() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_ifIndex;
}

Ptr<Channel>
WirelessNetDevice::GetChannel() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_channel;
}

void
WirelessNetDevice::SetAddress(Address address)
{
    NS_LOG_FUNCTION_NOARGS();
    m_address = Mac48Address::ConvertFrom(address);
}

Address
WirelessNetDevice::GetAddress() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_address;
}

bool
WirelessNetDevice::IsLinkUp() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_linkUp;
}

void
WirelessNetDevice::AddLinkChangeCallback(Callback<void> callback)
{
    NS_LOG_FUNCTION(&callback);
    m_linkChangeCallbacks.ConnectWithoutContext(callback);
}

bool
WirelessNetDevice::IsBroadcast() const
{
    NS_LOG_FUNCTION_NOARGS();
    return true;
}

Address
WirelessNetDevice::GetBroadcast() const
{
    NS_LOG_FUNCTION_NOARGS();
    return Mac48Address("ff:ff:ff:ff:ff:ff");
}

bool
WirelessNetDevice::IsMulticast() const
{
    NS_LOG_FUNCTION_NOARGS();
    return true;
}

Address
WirelessNetDevice::GetMulticast(Ipv4Address multicastGroup) const
{
    NS_LOG_FUNCTION(multicastGroup);

    Mac48Address ad = Mac48Address::GetMulticast(multicastGroup);

    //
    // Implicit conversion (operator Address ()) is defined for Mac48Address, so
    // use it by just returning the EUI-48 address which is automagically converted
    // to an Address.
    //
    NS_LOG_LOGIC("multicast address is " << ad);

    return ad;
}

bool
WirelessNetDevice::IsPointToPoint() const
{
    NS_LOG_FUNCTION_NOARGS();
    return false;
}

bool
WirelessNetDevice::IsBridge() const
{
    NS_LOG_FUNCTION_NOARGS();
    return false;
}

bool
WirelessNetDevice::Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(packet << dest << protocolNumber);
    return SendFrom(packet, m_address, dest, protocolNumber);
}

bool
WirelessNetDevice::SendFrom(Ptr<Packet> packet,
                        const Address& src,
                        const Address& dest,
                        uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(packet << src << dest << protocolNumber);
    NS_LOG_LOGIC("packet =" << packet);
    NS_LOG_LOGIC("UID is " << packet->GetUid() << ")");

    NS_ASSERT(IsLinkUp());

    Mac48Address destination = Mac48Address::ConvertFrom(dest);
    Mac48Address source = Mac48Address::ConvertFrom(src);
    AddHeader(packet, source, destination, protocolNumber);

    //
    // Place the packet to be sent on the send queue.  Note that the
    // queue may fire a drop trace, but we will too.
    //
    if (m_queue->Enqueue(packet) == false)
    {
        return false;
    }

    //
    // If the device is idle, we need to start a transmission. Otherwise,
    // the transmission will be started when the current packet finished
    // transmission (see TransmitCompleteEvent)
    //
    if (m_txMachineState == READY)
    {
        if (m_queue->IsEmpty() == false)
        {
            Ptr<Packet> packet = m_queue->Dequeue();
            NS_ASSERT_MSG(packet,
                          "WirelessNetDevice::SendFrom(): IsEmpty false but no Packet on queue?");
            m_currentPkt = packet;
            TransmitStart();
        }
    }
    return true;
}

Ptr<Node>
WirelessNetDevice::GetNode() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_node;
}

void
WirelessNetDevice::SetNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(node);

    m_node = node;
}

bool
WirelessNetDevice::NeedsArp() const
{
    NS_LOG_FUNCTION_NOARGS();
    return true;
}

void
WirelessNetDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb)
{
    NS_LOG_FUNCTION(&cb);
    m_rxCallback = cb;
}

Address
WirelessNetDevice::GetMulticast(Ipv6Address addr) const
{
    Mac48Address ad = Mac48Address::GetMulticast(addr);

    NS_LOG_LOGIC("MAC IPv6 multicast address is " << ad);
    return ad;
}

void
WirelessNetDevice::SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb)
{
    NS_LOG_FUNCTION(&cb);
    m_promiscRxCallback = cb;
}

bool
WirelessNetDevice::SupportsSendFrom() const
{
    NS_LOG_FUNCTION_NOARGS();
    return true;
}

int64_t
WirelessNetDevice::AssignStreams(int64_t stream)
{
    return 0;
}

void
WirelessNetDevice::SetPhy(Ptr<WirelessPhy> phy)
{
    NS_ASSERT(!m_phy);
    m_phy = phy;
    m_phy->SetMacUpcalls(m_macUpcalls);
}

Ptr<WirelessPhy>
WirelessNetDevice::GetPhy(void) const 
{
    return m_phy;
}



} // namespace ns3
