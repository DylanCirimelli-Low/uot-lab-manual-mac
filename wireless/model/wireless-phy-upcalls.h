#ifndef WIRELESS_PHY_UPCALLS_H
#define WIRELESS_PHY_UPCALLS_H

#include <ns3/simple-ref-count.h>
#include <ns3/net-device.h>

#include "ns3/wireless-transmission-vector.h"

namespace ns3 {

class WirelessPhyUpcalls : public SimpleRefCount<WirelessPhyUpcalls> {
public:

	typedef Callback<void, Ptr<const TransmissionVector> > TxCallback;
	typedef Callback<void, Ptr<const TransmissionVector> > RxCallback;
	typedef Callback< Ptr<NetDevice> > GetDeviceCallback;
	typedef Callback< Ptr<MobilityModel> > GetMobilityCallback;


	WirelessPhyUpcalls(
			TxCallback startTransmit,
			TxCallback finishTransmit,
			RxCallback startReceive,
			RxCallback finishReceive,
			GetDeviceCallback getDevice,
			GetMobilityCallback getMobility
			);

	virtual ~WirelessPhyUpcalls();

	void StartTransmit(Ptr<const TransmissionVector> txVector);
	void FinishTransmit(Ptr<const TransmissionVector> txVector);
	void StartReceive(Ptr<const TransmissionVector> rxVector);
	void FinishReceive(Ptr<const TransmissionVector> rxVector);
	Ptr<NetDevice> GetDevice(void) const;
	Ptr<MobilityModel> GetMobility(void) const;

private:
	TxCallback startTransmit;
	TxCallback finishTransmit;
	RxCallback startReceive;
	RxCallback finishReceive;
	GetDeviceCallback getDevice;
	GetMobilityCallback getMobility;
};

} /* namespace ns3 */

#endif /* WIRELESS_PHY_UPCALLS_H_ */
