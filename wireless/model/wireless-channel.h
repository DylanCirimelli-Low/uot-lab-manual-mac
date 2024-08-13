#ifndef ALOHA_WIRELESS_CHANNEL_H
#define ALOHA_WIRELESS_CHANNEL_H

#include <list>
#include "ns3/channel.h"
#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/nstime.h"
#include "ns3/wireless-phy-upcalls.h"
#include "ns3/wireless-transmission-vector.h"
#include "ns3/data-rate.h"

namespace ns3 {

class WirelessChannel : public Channel
{
public:

	static TypeId GetTypeId (void);
	virtual std::size_t GetNDevices(void) const;
	virtual Ptr<NetDevice> GetDevice(std::size_t i) const;

	WirelessChannel();
	virtual ~WirelessChannel (void);

	void SetPropagationDelayModel (const Ptr<PropagationDelayModel> delay);

	void Attach(Ptr<WirelessPhyUpcalls> phy);
	virtual int64_t AssignStreams (int64_t stream);

	void Send (Ptr<WirelessPhyUpcalls> sender, Ptr<const TransmissionVector> txVector);

    const DataRate GetDataRate();

protected:

	void StartReceive(Ptr<WirelessPhyUpcalls> receiver, Ptr<const TransmissionVector> rxVector);
	void FinishReceive(Ptr<WirelessPhyUpcalls> receiver, Ptr<const TransmissionVector> rxVector);

	void StartTransmit(Ptr<WirelessPhyUpcalls> sender, Ptr<const TransmissionVector> txVector);
	void FinishTransmit(Ptr<WirelessPhyUpcalls> sender, Ptr<const TransmissionVector> txVector);

	void SendTo(Ptr<WirelessPhyUpcalls> sender,
				Ptr<WirelessPhyUpcalls> receiver,
				Ptr<const TransmissionVector> txVector);

private:

	Ptr<PropagationDelayModel> m_delay;
	std::list< Ptr<WirelessPhyUpcalls> > m_attached;
	double m_range;
	DataRate m_bps;
};

} // namespace ns3

#endif /* WIRELESS_CHANNEL_H */
