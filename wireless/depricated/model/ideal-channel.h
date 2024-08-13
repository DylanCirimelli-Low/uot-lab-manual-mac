#ifndef IDEAL_CHANNEL_H
#define IDEAL_CHANNEL_H

#include <list>
#include "ns3/channel.h"
#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/nstime.h"

#include "ns3/ideal-phy-upcalls.h"
#include "ns3/ideal-transmission-vector.h"

namespace ns3 {

class IdealChannel : public Channel
{
public:

	static TypeId GetTypeId (void);
	virtual std::size_t GetNDevices(void) const;
	virtual Ptr<NetDevice> GetDevice(std::size_t i) const;

	IdealChannel();
	virtual ~IdealChannel (void);

	void SetPropagationDelayModel (const Ptr<PropagationDelayModel> delay);

	void Attach(Ptr<IdealPhyUpcalls> phy);
	virtual int64_t AssignStreams (int64_t stream);

	void Send (Ptr<IdealPhyUpcalls> sender, Ptr<const TransmissionVector> txVector);

protected:

	void StartReceive(Ptr<IdealPhyUpcalls> receiver, Ptr<const TransmissionVector> rxVector);
	void FinishReceive(Ptr<IdealPhyUpcalls> receiver, Ptr<const TransmissionVector> rxVector);

	void StartTransmit(Ptr<IdealPhyUpcalls> sender, Ptr<const TransmissionVector> txVector);
	void FinishTransmit(Ptr<IdealPhyUpcalls> sender, Ptr<const TransmissionVector> txVector);

	void SendTo(Ptr<IdealPhyUpcalls> sender,
				Ptr<IdealPhyUpcalls> receiver,
				Ptr<const TransmissionVector> txVector);

private:

	Ptr<PropagationDelayModel> m_delay;
	std::list< Ptr<IdealPhyUpcalls> > m_attached;
	double m_range;
};

} // namespace ns3

#endif /* WIRELESS_CHANNEL_H */
