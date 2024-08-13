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
    Time duration,
    bool shouldBeCorrupted)
    {
        m_pkt = pkt;
        m_device = device;
        m_mobility = mobility;
        m_duration = duration;
        m_shouldBeCorrupted = shouldBeCorrupted;
    }

TransmissionVector::~TransmissionVector (void) {
}

Ptr<Packet>
TransmissionVector::GetPacket (void) const
{
    NS_ASSERT(m_pkt);
    return m_pkt;
}

Ptr<NetDevice>
TransmissionVector::GetDevice (void) const
{
    NS_ASSERT(m_device);
    return m_device;
}

Ptr<MobilityModel>
TransmissionVector::GetMobility (void) const
{
    NS_ASSERT(m_mobility);
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
    return m_duration;
}

} /* namespace ns3 */