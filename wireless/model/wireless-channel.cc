#include "ns3/node.h"
#include "ns3/assert.h"
#include "ns3/net-device.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include <ns3/mobility-model.h>
#include <ns3/simulator.h>
#include <ns3/propagation-loss-model.h>
#include "ns3/double.h"
#include <ns3/data-rate.h>
#include "ns3/boolean.h"
#include "ns3/wireless-channel.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WirelessChannel");
NS_OBJECT_ENSURE_REGISTERED (WirelessChannel);

TypeId
WirelessChannel::GetTypeId (void)
{
	static auto delayModel = CreateObject<ConstantSpeedPropagationDelayModel>();
	delayModel->SetSpeed(299792458);

	static TypeId tid = TypeId ("ns3::WirelessChannel")
    				.SetParent<Channel> ()
					.SetGroupName ("Wireless")
					.AddConstructor<WirelessChannel> ()
					.AddAttribute ("PropagationDelayModel", "Delay model pointer",
							PointerValue (delayModel),
							MakePointerAccessor (&WirelessChannel::m_delay),
							MakePointerChecker<PropagationDelayModel> ())
					.AddAttribute ("TransmissionRange", 
							"Maximum range in m that a signal will propagate",
							DoubleValue(50.0), 
							MakeDoubleAccessor (&WirelessChannel::m_range),
							MakeDoubleChecker<double> ())
					.AddAttribute ("DataRate", 
							"Data rate of the channel",
							DataRateValue( DataRate("10Mb/s") ), 
							MakeDataRateAccessor (&WirelessChannel::m_bps),
							MakeDataRateChecker ());
	return tid;
}

WirelessChannel::WirelessChannel ()
{

}

WirelessChannel::~WirelessChannel (void)
{
	m_delay = 0;
}

void
WirelessChannel::SetPropagationDelayModel (const Ptr<PropagationDelayModel> delay)
{
	NS_LOG_FUNCTION (this << delay);
	m_delay = delay;
}

void
WirelessChannel::Attach(Ptr<WirelessPhyUpcalls> phy)
{
	m_attached.push_back (phy);
}

std::size_t
WirelessChannel::GetNDevices(void) const
{
	return m_attached.size();
}

Ptr<NetDevice>
WirelessChannel::GetDevice(std::size_t index) const
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
WirelessChannel::StartReceive(Ptr<WirelessPhyUpcalls> receiver, Ptr<const TransmissionVector> rxVector)
{
	receiver->StartReceive(rxVector);
}

void
WirelessChannel::FinishReceive(Ptr<WirelessPhyUpcalls> receiver, Ptr<const TransmissionVector> rxVector)
{
	receiver->FinishReceive(rxVector);
}

void
WirelessChannel::StartTransmit(Ptr<WirelessPhyUpcalls> sender, Ptr<const TransmissionVector> txVector)
{
	sender->StartTransmit(txVector);
}

void
WirelessChannel::FinishTransmit(Ptr<WirelessPhyUpcalls> sender, Ptr<const TransmissionVector> txVector)
{
	sender->FinishTransmit(txVector);
}

void
WirelessChannel::SendTo(Ptr<WirelessPhyUpcalls> sender,
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
		txVector->GetMobility(),
		txVector->GetDuration(),
		txVector->ShouldBeCorrupted()
	);

	NS_ASSERT(m_range > 0);
	NS_LOG_DEBUG("range= " << senderMobility->GetDistanceFrom(receiverMobility));

	if(senderMobility->GetDistanceFrom(receiverMobility) <= m_range)
	{
		auto dstNode = receiver->GetDevice()->GetNode()->GetId();
		
		Simulator::ScheduleWithContext (
			dstNode,
			delay,
			&WirelessChannel::StartReceive,
			this,
			receiver,
			rxVector);

		Simulator::ScheduleWithContext (
			dstNode,
			duration + delay,
			&WirelessChannel::FinishReceive,
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
		txVector->GetDuration(),
		txVector->ShouldBeCorrupted()
		);

	return clone;
}

void
WirelessChannel::Send(Ptr<WirelessPhyUpcalls> sender, Ptr<const TransmissionVector> txVector)
{
	for (auto i = m_attached.begin (); i != m_attached.end (); ++i)
	{
		if (sender != *i)
		{
			SendTo(sender, *i, clone(txVector));
		}
	}

	auto senderNode = sender->GetDevice()->GetNode()->GetId();

	NS_LOG_DEBUG("Transmission (" << txVector->GetPacket()->GetSize() << ") from " << senderNode << " until " << Simulator::Now() + txVector->GetDuration());
	Simulator::ScheduleWithContext (
		senderNode,
		Time(0),
		&WirelessChannel::StartTransmit,
		this,
		sender,
		txVector);

	Simulator::ScheduleWithContext (
		senderNode,
		txVector->GetDuration(),
		&WirelessChannel::FinishTransmit,
		this,
		sender,
		txVector);
}

int64_t
WirelessChannel::AssignStreams (int64_t stream)
{
	NS_FATAL_ERROR("Not implemented");
}

const DataRate 
WirelessChannel::GetDataRate (void) 
{
	return m_bps;
}
} // namespace ns3
