#include "ns3/wireless-transmission-vector.h"
#include "ns3/wireless-plcp-header.h"
#include "ns3/log.h"

#define _1MBPS 1000000.0
#define _10MBPS 10000000.0
namespace ns3 {
    
NS_LOG_COMPONENT_DEFINE ("WirelessTransmissionVector");

TransmissionVector::TransmissionVector(
    Ptr<Packet> pkt,
    Ptr<NetDevice> device,
    Ptr<MobilityModel> mobility,
    bool shouldBeCorrupted)
    {
        m_pkt = pkt;
        m_device = device;
        m_mobility = mobility;
        m_shouldBeCorrupted = shouldBeCorrupted;
    }

TransmissionVector::~TransmissionVector (void) {
}

Ptr<Packet>
TransmissionVector::GetPacket (void) const
{
    NS_ASSERT(m_pkt != 0);
    return m_pkt;
}

Ptr<NetDevice>
TransmissionVector::GetDevice (void) const
{
    NS_ASSERT(m_device != 0);
    return m_device;
}

Ptr<MobilityModel>
TransmissionVector::GetMobility (void) const
{
    NS_ASSERT(m_mobility != 0);
    return m_mobility;
}

bool
TransmissionVector::ShouldBeCorrupted (void) const 
{
    return m_shouldBeCorrupted;
}

Time
TransmissionVector::GetDuration (void) const
{
    NS_ASSERT(m_pkt != 0);

    Ptr<Packet> packet = m_pkt->Copy();
    PlcpHeader plcp;
    packet->RemoveHeader(plcp);

    Time phyTime = Seconds(plcp.GetSerializedSize() * 8.0 / _1MBPS);
    Time packetTime = Seconds(packet->GetSize() * 8.0 / _10MBPS);

    return (phyTime + packetTime);

    

}

} /* namespace ns3 */