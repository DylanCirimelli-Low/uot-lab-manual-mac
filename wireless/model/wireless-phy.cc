#include <ns3/simulator.h>
#include "ns3/wireless-phy.h"
#include "ns3/wireless-plcp-header.h"
#include <ns3/boolean.h>
#include <ns3/double.h>

#define _1MBPS 1000000.0
#define _10MBPS 10000000.0

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WirelessPhy");
NS_OBJECT_ENSURE_REGISTERED (WirelessPhy);

enum {
    RX,
    TX,
    IDLE
};

TypeId
WirelessPhy::GetTypeId (void)
{
    static TypeId
    tid =   TypeId ("ns3::WirelessPhy")
            .SetParent<Object> ()
            .SetGroupName("Wireless")
            .AddConstructor<WirelessPhy>()
            .AddAttribute ("PacketErrorRate", 
                "The probability that a packet is corrupted in transmission",
                DoubleValue(0.0), 
                MakeDoubleAccessor (&WirelessPhy::m_per),
                MakeDoubleChecker<double>(0.0))
            .AddAttribute ("EnableCollisions",
                    "Whether collisions may occur",
                    BooleanValue (true),
                    MakeBooleanAccessor(&WirelessPhy::m_enableCollisions),
                    MakeBooleanChecker ());

    return tid;
}

WirelessPhy::WirelessPhy (void)
{
    m_channel = 0;
    m_device = 0;
    m_mobility = 0;
    m_macUpcalls = 0;
    m_sensing = false;

    m_phyUpcalls = Create<WirelessPhyUpcalls>(               
        MakeCallback (&WirelessPhy::StartTransmit, this),
        MakeCallback (&WirelessPhy::FinishTransmit, this),
        MakeCallback (&WirelessPhy::StartReceive, this),
        MakeCallback (&WirelessPhy::FinishReceive, this),
        MakeCallback (&WirelessPhy::GetDevice, this),
        MakeCallback (&WirelessPhy::GetMobility, this)
        );

    m_state = IDLE;
}

WirelessPhy::~WirelessPhy (void){
}

void 
WirelessPhy::SetChannel (Ptr<WirelessChannel> channel)
{
    NS_ASSERT(channel);
    m_channel = channel;
    channel->Attach(m_phyUpcalls);
}

Ptr<WirelessChannel> 
WirelessPhy::GetChannel (void) const
{
    NS_ASSERT(m_channel);
    return m_channel;
}

void
WirelessPhy::SetDevice (Ptr<NetDevice> device)
{
    m_device = device;
}

Ptr<NetDevice>
WirelessPhy::GetDevice (void) const
{
    NS_ASSERT(m_device);
    return m_device;
}

void
WirelessPhy::SetMobility (Ptr<MobilityModel> mobility)
{
    m_mobility = mobility;
}

Ptr<MobilityModel>
WirelessPhy::GetMobility (void) const
{
    NS_ASSERT(m_mobility);
    return m_mobility;
}

void
WirelessPhy::SetMacUpcalls (Ptr<WirelessMacUpcalls> upcalls)
{
    m_macUpcalls = upcalls;
}

void
WirelessPhy::SetPhyUpcalls (Ptr<WirelessPhyUpcalls> upcalls)
{
    m_phyUpcalls = upcalls;
}

bool
WirelessPhy::Send (Ptr<Packet> pkt)
{
    NS_ASSERT(m_channel);
    NS_LOG_DEBUG("Sending from Phy");

    auto copy = pkt->Copy();

    PlcpHeader phyHeader;
    copy->AddHeader(phyHeader);

    // Corrupt packet randomly based on PER of the PHY. If a packet is corrupted
    // it will not be successfully received by any receiver.
	static Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable> ();
	
    //bool shouldBeCorrupted = (m_per > rng->GetValue(0.0, 1.0));
    bool shouldBeCorrupted = false;

    NS_ASSERT(m_device);
    Ptr<TransmissionVector> txVector = Create<TransmissionVector> (
        copy, 
        m_device, 
        m_mobility,
        GetPacketTime(copy),
        shouldBeCorrupted
        ); 

    NS_ASSERT(m_phyUpcalls);
    m_channel->Send(m_phyUpcalls, txVector);
    return true;
}

void
WirelessPhy::StartReceive (Ptr<const TransmissionVector> rxVector)
{
    NS_LOG_DEBUG("Started Receiving rxVector");  
    
    m_transmissions.push_back(Create<TransmissionUnit>(rxVector));

    if (m_enableCollisions) {
        if((m_transmissions.size() > 1) || (m_state == TX)) {
            for(auto unit : m_transmissions) {
                unit->Corrupt();
            }
        }
    }

    if (m_state == IDLE) {
        m_state = RX;
        m_macUpcalls->StartCarrierSense();
        m_sensing = true;
    }
}

void
WirelessPhy::FinishReceive(Ptr<const TransmissionVector> rxVector)
{
    NS_LOG_DEBUG("Finished receiving rxVector");

    for(auto unit : m_transmissions) {
        if(unit->GetTransmissionVector() == rxVector) {
            if(!m_enableCollisions || !unit->IsCorrupted()) {
                PlcpHeader phyHeader;
                unit->GetTransmissionVector()->GetPacket()->RemoveHeader(phyHeader);
                m_macUpcalls->Receive(unit->GetTransmissionVector()->GetPacket());
            }

            m_transmissions.remove(unit);
            break;
        }
    }

    if(m_transmissions.empty() && m_state == RX){
        m_state = IDLE;
        m_macUpcalls->EndCarrierSense();
        m_sensing = false;
    }
}

void
WirelessPhy::StartTransmit(Ptr<const TransmissionVector> txVector)
{
    NS_LOG_DEBUG("Started transmission");
    NS_ASSERT(m_state != TX);

    m_state = TX;
    
    if (m_enableCollisions) {
        for(auto unit : m_transmissions) {
            unit->Corrupt();
        }
    }
}

void
WirelessPhy::FinishTransmit(Ptr<const TransmissionVector> txVector)
{
    NS_LOG_DEBUG("Finish Transmission");

    if (!m_transmissions.empty())
        m_state = RX;
    else
        m_state = IDLE;

    m_macUpcalls->FinishTransmit();
}

bool
WirelessPhy::IsIdle(void)
{
    return (m_state == IDLE);
}

bool
WirelessPhy::IsTransmitting(void)
{
    return (m_state == TX);
}

bool
WirelessPhy::IsReceiving(void)
{
    return (m_state == RX);
}

Time
WirelessPhy::GetPacketTime(Ptr<const Packet> packet)
{
    Time phyTime = Seconds(24 * 8.0 / _1MBPS);
    Time packetTime = m_channel->GetDataRate().CalculateBytesTxTime(packet->GetSize());
    return phyTime + packetTime;
}

Time
WirelessPhy::GetInterframeGap(void)
{
    return m_channel->GetDataRate().CalculateBytesTxTime(96 / 8);
}
	
} /* end namespace ns3 */