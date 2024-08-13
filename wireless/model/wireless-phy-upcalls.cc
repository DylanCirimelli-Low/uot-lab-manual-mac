#include "wireless-phy-upcalls.h"

namespace ns3 {

WirelessPhyUpcalls::WirelessPhyUpcalls(
		TxCallback startTransmit,
		TxCallback finishTransmit,
		RxCallback startReceive,
		RxCallback finishReceive,
		GetDeviceCallback getDevice,
		GetMobilityCallback getMobility
		)
: startTransmit(startTransmit), finishTransmit(finishTransmit),
  startReceive(startReceive), finishReceive(finishReceive), 
  getDevice(getDevice), getMobility(getMobility)
{
}

WirelessPhyUpcalls::~WirelessPhyUpcalls() {
}

void
WirelessPhyUpcalls::StartTransmit(Ptr<const TransmissionVector> txVector)
{
	NS_ASSERT(startTransmit.GetImpl());
	startTransmit(txVector);
}

void
WirelessPhyUpcalls::FinishTransmit(Ptr<const TransmissionVector> txVector)
{
	NS_ASSERT(finishTransmit.GetImpl());
	finishTransmit(txVector);
}

void
WirelessPhyUpcalls::StartReceive(Ptr<const TransmissionVector> txVector)
{
	NS_ASSERT(startReceive.GetImpl());
	if (startReceive.GetImpl()) {
		startReceive(txVector);
	}
}

void
WirelessPhyUpcalls::FinishReceive(Ptr<const TransmissionVector> txVector)
{
	NS_ASSERT(finishReceive.GetImpl());
	if (finishReceive.GetImpl()) {
		finishReceive(txVector);
	}
}

Ptr<NetDevice>
WirelessPhyUpcalls::GetDevice(void) const
{
	NS_ASSERT(getDevice.GetImpl());

	if (getDevice.GetImpl()) {
		return getDevice();
	} else {
		return 0;
	}
}

Ptr<MobilityModel>
WirelessPhyUpcalls::GetMobility(void) const
{
	if (getMobility.GetImpl()) {
		return getMobility();
	} else {
		return 0;
	}
}

} /* namespace ns3 */
