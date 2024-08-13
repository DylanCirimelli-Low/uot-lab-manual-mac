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
	NS_ASSERT(startTransmit.GetImpl() != 0);
	startTransmit(txVector);
}

void
WirelessPhyUpcalls::FinishTransmit(Ptr<const TransmissionVector> txVector)
{
	NS_ASSERT(finishTransmit.GetImpl() != 0);
	finishTransmit(txVector);
}

void
WirelessPhyUpcalls::StartReceive(Ptr<const TransmissionVector> txVector)
{
	NS_ASSERT(startReceive.GetImpl() != 0);
	if (startReceive.GetImpl() != 0) {
		startReceive(txVector);
	}
}

void
WirelessPhyUpcalls::FinishReceive(Ptr<const TransmissionVector> txVector)
{
	NS_ASSERT(finishReceive.GetImpl() != 0);
	if (finishReceive.GetImpl() != 0) {
		finishReceive(txVector);
	}
}

Ptr<NetDevice>
WirelessPhyUpcalls::GetDevice(void) const
{
	NS_ASSERT(getDevice.GetImpl() != 0);

	if (getDevice.GetImpl() != 0) {
		return getDevice();
	} else {
		return 0;
	}
}

Ptr<MobilityModel>
WirelessPhyUpcalls::GetMobility(void) const
{
	if (getMobility.GetImpl() != 0) {
		return getMobility();
	} else {
		return 0;
	}
}

} /* namespace ns3 */
