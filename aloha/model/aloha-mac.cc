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
#include "ns3/aloha-mac.h"
#include "ns3/aloha-header.h"
#include "ns3/wireless-mac-upcalls.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AlohaMac");
NS_OBJECT_ENSURE_REGISTERED (AlohaMac);

TypeId
AlohaMac::GetTypeId (void)
{
    static TypeId
    tid =   TypeId ("ns3::AlohaMac")
            .SetParent<Object> ()
            .SetGroupName("Aloha")
            .AddConstructor<AlohaMac>()
            .AddTraceSource("Enqueue",
                            "Trace fired when a packet is attempted to be enqueued at the node",
                            MakeTraceSourceAccessor(&AlohaMac::m_enqueueTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("MacTx",
                            "Trace source indicating a packet has been transmited",
                            MakeTraceSourceAccessor(&AlohaMac::m_macTxTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("AckReceive",
                            "Trace fired when an ACK destined for the node is received",
                            MakeTraceSourceAccessor(&AlohaMac::m_ackTrace),
                            "ns3::Packet::TracedCallback")
            .AddAttribute ("UsePriorityAck",
                    "Whether priority ACKs are used",
                    BooleanValue (false),
                    MakeBooleanAccessor(&AlohaMac::m_usePriorityAcks),
                    MakeBooleanChecker ())
            .AddAttribute ("UseCarrierSensing",
                    "Whether carrier sensing is used",
                    BooleanValue (false),
                    MakeBooleanAccessor(&AlohaMac::m_useCarrierSensing),
                    MakeBooleanChecker ());

    return tid;
}

AlohaMac::AlohaMac()
{  
    /* 
     * These are upcalls from the PHY that alert the MAC 
     * when important events happen. 
     * 
     * The state of the PHY can be queried directly so
     * there is no need to maintain a state machine
     * in the MAC.
     * 
     */

    m_macUpcalls = Create<WirelessMacUpcalls>(
        MakeCallback (&AlohaMac::Receive, this),
        MakeCallback (&AlohaMac::StartCarrierSense, this),
        MakeCallback (&AlohaMac::EndCarrierSense, this),
        MakeCallback (&AlohaMac::FinishTransmit, this)
        );

    /* 
     * DropTailQueues will throw away any packets 
     * enqueued after its internal buffer is full.
     * 
     */

    m_packetQueue = Create<DropTailQueue<Packet>>();
    m_rand = CreateObject<UniformRandomVariable>();

    m_transmissionTimer = Timer(Timer::CANCEL_ON_DESTROY);
    m_transmissionTimer.SetFunction(&AlohaMac::Transmit, this);

    m_ackTimer = Timer(Timer::CANCEL_ON_DESTROY);
    m_ackTimer.SetFunction(&AlohaMac::AckTimeout, this);
}

void
AlohaMac::DoDispose() 
{
    m_rand = 0;
    m_packetQueue = 0;
}

void
AlohaMac::SetAddress(Address address)
{
    m_macAddress = Mac48Address::ConvertFrom(address);    
}

Address
AlohaMac::GetAddress(void) const
{
    return m_macAddress;
}

void
AlohaMac::SetPhy(Ptr<WirelessPhy> phy)
{
    m_phy = phy;
    m_phy->SetMacUpcalls(m_macUpcalls);
}

Ptr<WirelessPhy>
AlohaMac::GetPhy(void) const
{
    NS_ASSERT(m_phy);
    return m_phy;
}

void
AlohaMac::Transmit(void)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_transmissionTimer.IsExpired()); 

    if (m_useCarrierSensing) {
        if (m_phy->IsReceiving()) {
            // schedule backoff
            StartBackoff();
            return;
        }
    }
    
    NS_ASSERT(m_packetQueue->IsEmpty() == false);
    auto packet = m_packetQueue->Peek()->Copy();
    AlohaHeader header (m_macAddress, m_sinkAddress);
    packet->AddHeader(header);

    NS_LOG_INFO("sending data " << packet << " to PHY");
    m_macTxTrace(packet);
    m_phy->Send(packet);

    // NS_LOG_INFO("Starting ACK timer for packet time = " << m_phy->GetTransmissionTime(packet) << ", ack time = " << GetAckTime() << ", prop delays=" << MicroSeconds(2));
    m_ackTimer.Schedule (m_phy->GetTransmissionTime(packet) + GetAckTime() + MicroSeconds(2));
}

bool 
AlohaMac::Send(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);

    if (m_packetQueue->IsEmpty()) 
    {
        NS_ASSERT(m_transmissionTimer.IsExpired());
        NS_ASSERT(m_ackTimer.IsExpired());
        // Start the transmission timer if we now have data to send
        Time delay = MicroSeconds(m_rand->GetInteger(0, std::pow(2, m_backoffExponent) * 1000));
        Time jitter = MicroSeconds(m_rand->GetInteger(0, 1000));
        m_transmissionTimer.Schedule(delay + jitter);
        NS_LOG_INFO("Arrival in empty tx queue. Scheduling transmission for " << delay + jitter + Simulator::Now());
    }

    m_enqueueTrace(packet);
    return m_packetQueue->Enqueue(packet);
    
}

void
AlohaMac::Receive(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);

    AlohaHeader header;
    packet->RemoveHeader(header);

    // send ACK if we are the sink node and receive data
    if (m_macAddress == m_sinkAddress) {
        TransmitAck( packet, header.GetSrc() );
    }

    // cancel ACK timer if we are the intended receiver of the ACK
    else if (m_macAddress == header.GetDst()) {
        NS_LOG_INFO("Received ack for self.");
        m_ackTrace(packet);
        NS_ASSERT( m_transmissionTimer.IsRunning() == false);
        m_ackTimer.Cancel();
        m_backoffExponent = m_minBackoffExponent;

        auto packet = m_packetQueue->Dequeue();
        // schedule next transmission if we have more data to send
        if (!m_packetQueue->IsEmpty()) {
            Time delay = MicroSeconds(m_rand->GetInteger(0, std::pow(2, m_backoffExponent) * 1000));
            m_transmissionTimer.Schedule(delay);
        }
         m_netDeviceReceive(packet, header.GetSrc());
    } else if (m_usePriorityAcks) {
        if (m_transmissionTimer.IsRunning()) {
            Time currentDelay = m_transmissionTimer.GetDelayLeft();
            m_transmissionTimer.Cancel();
            m_transmissionTimer.Schedule (currentDelay + GetAckTime() + MicroSeconds(2));
        }
    }
}

void 
AlohaMac::TransmitAck(Ptr<const Packet> p, Mac48Address dst)
{
    NS_LOG_FUNCTION(this << dst);

    AlohaMacPacketTag tag(p->GetUid(), p->GetSize());

    AlohaHeader ack = AlohaHeader(m_macAddress, dst);
    Ptr<Packet> packet = Create<Packet>(0);
    packet->AddHeader(ack);
    packet->AddPacketTag(tag);
    
    NS_LOG_INFO("sending ACK " << packet << " to PHY");
    m_phy->Send(packet);
}

void
AlohaMac::StartBackoff(void)
{
    NS_LOG_FUNCTION(this);

    /* give up on waiting for the ACK and do backoff */
    NS_ASSERT(m_transmissionTimer.IsRunning() == false);
    NS_ASSERT(m_ackTimer.IsRunning() == false);
    m_backoffExponent = std::min( (++m_backoffExponent) , m_maxBackoffExponent);
    Time delay = MicroSeconds(m_rand->GetInteger(0, std::pow(2, m_backoffExponent) * 1000));
    m_transmissionTimer.Schedule(delay);
    NS_LOG_INFO("Next transmission at " << Simulator::Now() + delay << ". (backoff exp = " << m_backoffExponent << ")");
}

void
AlohaMac::AckTimeout(void)
{
    NS_LOG_INFO("Ack timeout");
    StartBackoff();
}

Time
AlohaMac::GetAckTime(void) const
{
    Ptr<Packet> ack = Create<Packet>();
    AlohaHeader header;
    ack->AddHeader(header);
    return m_phy->GetTransmissionTime(ack);

}
void
AlohaMac::StartCarrierSense(void){
}

void
AlohaMac::EndCarrierSense(void){
}

void
AlohaMac::FinishTransmit(void) {
}

/*
 * We can use these callbacks to pass information
 * from the MAC up to the NetDevice.
 */

void
AlohaMac::SetReceiveCallback(NetDeviceReceiveCallback callback)
{
    m_netDeviceReceive = callback;
}

AlohaMac::NetDeviceReceiveCallback
AlohaMac::GetReceiveCallback (void) const
{
	return m_netDeviceReceive;
}

int64_t
AlohaMac::AssignStreams(int64_t stream)
{
    m_rand->SetStream(stream);
    return 1;
}

void
AlohaMac::SetMinBackoffExponent(uint32_t minBackoffExp)
{
    m_minBackoffExponent = minBackoffExp;
    m_backoffExponent = m_minBackoffExponent;
}


void
AlohaMac::SetMaxBackoffExponent(uint32_t maxBackoffExp)
{
    m_maxBackoffExponent = maxBackoffExp;
}

void 
AlohaMac::SetSinkAddress(Mac48Address sinkAddress)
{
    m_sinkAddress = sinkAddress;
}

} /* namespace ns3 */