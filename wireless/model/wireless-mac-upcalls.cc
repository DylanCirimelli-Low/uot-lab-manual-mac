#include "wireless-mac-upcalls.h"

namespace ns3 {

WirelessMacUpcalls::WirelessMacUpcalls(RxMacCallback rxCallback, CarrierSenseCallback startSenseCallback, CarrierSenseCallback endSenseCallback, FinishTransmitCallback finishTransmitCallback)
:rxCallback(rxCallback), startSenseCallback(startSenseCallback), endSenseCallback(endSenseCallback), finishTransmitCallback(finishTransmitCallback){
}

WirelessMacUpcalls::~WirelessMacUpcalls(){
}

void
WirelessMacUpcalls::Receive(Ptr<Packet> pkt)
{
	NS_ASSERT(rxCallback.GetImpl());
	rxCallback(pkt);
}

void
WirelessMacUpcalls::StartCarrierSense(void)
{
	if (startSenseCallback.GetImpl()) {
		startSenseCallback();
	}
}

void
WirelessMacUpcalls::EndCarrierSense(void)
{
	if (endSenseCallback.GetImpl()) {
		endSenseCallback();
	}
}

void
WirelessMacUpcalls::FinishTransmit(void)
{
	if (finishTransmitCallback.GetImpl()) {
		finishTransmitCallback();
	}
}

} /* namespace ns3 */