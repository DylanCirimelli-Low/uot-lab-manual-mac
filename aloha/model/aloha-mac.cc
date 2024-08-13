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
            .AddTraceSource("EnqueueTx",
                            "Trace fired when a packet is attempted to be enqueued at the node",
                            MakeTraceSourceAccessor(&AlohaMac::m_enqueueTrace),
                            "ns3::AlohaMac::TracedCallback")
            .AddTraceSource("AckReceive",
                            "Trace fired when an ACK destined for the node is received",
                            MakeTraceSourceAccessor(&AlohaMac::m_ackTrace),
                            "ns3::AlohaMac::TracedCallback");

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

    m_transmissionTimer = Timer();
    m_transmissionTimer.SetFunction(&AlohaMac::Transmit, this);

    m_ackTimer = Timer();
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

    auto packet = m_packetQueue->Peek()->Copy();
    AlohaHeader header (m_macAddress, m_sinkAddress);
    packet->AddHeader(header);

    NS_LOG_INFO("sending data " << packet << " to PHY");
    m_phy->Send(packet);

    m_ackTimer.Schedule ( MilliSeconds(10) );
}

bool 
AlohaMac::Send(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);

    if (m_packetQueue->IsEmpty()) {
        // Start the transmission timer if we now have data to send
        auto delay = m_rand->GetInteger(0, std::pow(2, m_backoffExponent));
        m_transmissionTimer.Schedule( MilliSeconds(delay) );
        NS_LOG_INFO("Arrival in empty tx queue. Scheduling transmission for " << MilliSeconds(delay) + Simulator::Now());
    }

    m_enqueueTrace(m_macAddress);
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
        TransmitAck( header.GetSrc() );
    }

    // cancel ACK timer if we are the intended receiver of the ACK
    else if (m_macAddress == header.GetDst()) {
        NS_LOG_INFO("Received ack for self.");
        m_ackTrace(m_macAddress);
        NS_ASSERT( m_transmissionTimer.IsRunning() == false);
        m_ackTimer.Cancel();
        m_backoffExponent = m_minBackoffExponent;

        auto packet = m_packetQueue->Dequeue();
        // schedule next transmission if we have more data to send
        if (!m_packetQueue->IsEmpty()) {
            auto delay = m_rand->GetInteger(0, std::pow(2, m_backoffExponent));
            m_transmissionTimer.Schedule( MilliSeconds(delay) );
        }
    }
}

void 
AlohaMac::TransmitAck(Mac48Address dst)
{
    NS_LOG_FUNCTION(this << dst);

    AlohaHeader ack = AlohaHeader(m_macAddress, dst);
    Ptr<Packet> packet = Create<Packet>(0);
    packet->AddHeader(ack);
    NS_LOG_INFO("sending ACK " << packet << " to PHY");
    m_phy->Send(packet);
}

void
AlohaMac::AckTimeout(void)
{
    NS_LOG_FUNCTION(this);

    /* give up on waiting for the ACK and do backoff */
    NS_ASSERT( m_transmissionTimer.IsRunning() == false);
    m_backoffExponent = std::min( (++m_backoffExponent) , m_maxBackoffExponent);
    auto delay = m_rand->GetInteger(0, std::pow(2, m_backoffExponent));
    m_transmissionTimer.Schedule( MilliSeconds(delay) );
    NS_LOG_INFO("Ack timeout. Next transmission at " << Simulator::Now() + MilliSeconds(delay) << ". (backoff exp = " << m_backoffExponent << ")");
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