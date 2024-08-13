#ifndef WIRELESS_PHY_H
#define WIRELESS_PHY_H

#include <list>
#include <ns3/object.h>
#include <ns3/channel.h>
#include <ns3/mobility-model.h>

#include "ns3/wireless-transmission-vector.h"
#include "ns3/wireless-channel.h"
#include "ns3/wireless-mac-upcalls.h"
#include "ns3/wireless-phy-upcalls.h"
#include "ns3/wireless-transmission-unit.h"

namespace ns3 {

class WirelessPhy : public Object {

public:
	static TypeId GetTypeId (void);

	WirelessPhy (void);
	virtual ~WirelessPhy (void);

	void SetChannel (Ptr<WirelessChannel> channel);
	Ptr<WirelessChannel> GetChannel (void) const;

	void SetDevice (Ptr<NetDevice> device);
	Ptr<NetDevice> GetDevice (void) const;

	void SetMobility (Ptr<MobilityModel> mobility);
	Ptr<MobilityModel> GetMobility (void) const;

	void SetMacUpcalls(Ptr<WirelessMacUpcalls> upcalls);
    void SetPhyUpcalls(Ptr<WirelessPhyUpcalls> upcalls);
    
	bool Send(Ptr<Packet> pkt);

    void StartTransmit(Ptr<const TransmissionVector> txVector);
    void FinishTransmit(Ptr<const TransmissionVector> txVector);

    void StartReceive(Ptr<const TransmissionVector> rxVector);
    void FinishReceive(Ptr<const TransmissionVector> rxVector);

    static Time GetPacketTime(uint32_t size);

    bool IsIdle(void);
    bool IsTransmitting(void);
    bool IsReceiving(void);


private:
    std::list < Ptr<TransmissionUnit> > m_transmissions;
    Ptr<WirelessChannel> m_channel;
    Ptr<NetDevice> m_device;
    Ptr<MobilityModel> m_mobility;
    Ptr<WirelessMacUpcalls> m_macUpcalls;
    Ptr<WirelessPhyUpcalls> m_phyUpcalls;

    int m_state;
    bool m_sensing;
    double m_per;

}; 
} /* namespace ns3 */

#endif /* WIRELESS_PHY_H */
