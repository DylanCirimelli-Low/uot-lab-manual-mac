#include "ns3/node.h"
#include "ns3/assert.h"
#include "ns3/net-device.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include <ns3/mobility-model.h>
#include <ns3/simulator.h>
#include <ns3/propagation-loss-model.h>
#include "ns3/double.h"

#include "ns3/ideal-channel.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("IdealChannel");
NS_OBJECT_ENSURE_REGISTERED (IdealChannel);

TypeId
IdealChannel::GetTypeId (void)
{
	static auto delayModel = CreateObject<ConstantSpeedPropagationDelayModel>();
	delayModel->SetSpeed(299792458);

	static TypeId tid = TypeId ("ns3::IdealChannel")
    				.SetParent<Channel> ()
					.SetGroupName ("IdealPhy")
					.AddConstructor<IdealChannel> ()
					.AddAttribute ("PropagationDelayModel", "Delay model pointer",
							PointerValue (delayModel),
							MakePointerAccessor (&IdealChannel::m_delay),
							MakePointerChecker<PropagationDelayModel> ())
					.AddAttribute ("TransmissionRange", 
							"Maximum range in m that a signal will propagate",
							DoubleValue(2000.0), 
							MakeDoubleAccessor (&IdealChannel::m_range),
							MakeDoubleChecker<double>(0));
	return tid;
}

IdealChannel::IdealChannel ()
{

}

IdealChannel::~IdealChannel (void)
{
		m_delay = 0;
}

void
IdealChannel::SetPropagationDelayModel (const Ptr<PropagationDelayModel> delay)
{
	NS_LOG_FUNCTION (this << delay);
	m_delay = delay;
}

void
IdealChannel::Attach(Ptr<WirelessPhyUpcalls> phy)
{
	m_attached.push_back (phy);
}

std::size_t
IdealChannel::GetNDevices(void) const
{
	return m_attached.size();
}

Ptr<NetDevice>
IdealChannel::GetDevice(std::size_t index) const
{
	std::size_t j = 0;
	for (auto iter = m_attached.begin (); iter != m_attached.end (); ++iter)
	{
		if (j == index)
		{
			return (*iter)->GetDevice();
		}
		j++;
	}

	NS_FATAL_ERROR ("Unable to get device");
	return 0;
}

void
IdealChannel::FinishReceive(Ptr<WirelessPhyUpcalls> receiver, Ptr<const TransmissionVector> rxVector)
{
	receiver->FinishReceive(rxVector);
}

void
IdealChannel::SendTo(Ptr<WirelessPhyUpcalls> sender,
						Ptr<WirelessPhyUpcalls> receiver,
						Ptr<const TransmissionVector> txVector)
{	

	Ptr<MobilityModel> senderMobility = sender->GetMobility();
	Ptr<MobilityModel> receiverMobility = receiver->GetMobility();

	Time delay = m_delay->GetDelay (senderMobility, receiverMobility);
	Time duration = txVector->GetDuration();

	Ptr<TransmissionVector> rxVector = Create<TransmissionVector>(
		txVector->GetPacket(),
		txVector->GetDevice(),
		txVector->GetMobility()
	);

	NS_ASSERT(m_range > 0);
	NS_LOG_DEBUG("range= " << senderMobility->GetDistanceFrom(receiverMobility));

	if(senderMobility->GetDistanceFrom(receiverMobility) <= m_range)
	{
		auto dstNode = receiver->GetDevice()->GetNode()->GetId();

		Simulator::ScheduleWithContext (
			dstNode,
			duration + delay,
			&IdealChannel::FinishReceive,
			this,
			receiver,
			rxVector);
	}
}

Ptr<TransmissionVector> clone(Ptr<const TransmissionVector> txVector)
{
	Ptr<Packet> packet = txVector->GetPacket();
	
	Ptr<TransmissionVector> clone = Create<TransmissionVector>(
		Create<Packet>(*packet),
		txVector->GetDevice(),
		txVector->GetMobility(),
		txVector->ShouldBeCorrupted()
		);

	return clone;
}

void
IdealChannel::Send(Ptr<WirelessPhyUpcalls> sender, Ptr<const TransmissionVector> txVector)
{
	for (auto i = m_attached.begin (); i != m_attached.end (); ++i)
	{
		if (sender != *i)
		{
			SendTo(sender, *i, clone(txVector));
		}
	}

	auto senderNode = sender->GetDevice()->GetNode()->GetId();

	Simulator::ScheduleWithContext (
		senderNode,
		txVector->GetDuration(),
		&IdealChannel::StartTransmit,
		this,
		sender,
		txVector);

	Simulator::ScheduleWithContext (
		senderNode,
		txVector->GetDuration(),
		&IdealChannel::FinishTransmit,
		this,
		sender,
		txVector);
}

int64_t
IdealChannel::AssignStreams (int64_t stream)
{
	NS_FATAL_ERROR("Not implemented");
}

} // namespace ns3
