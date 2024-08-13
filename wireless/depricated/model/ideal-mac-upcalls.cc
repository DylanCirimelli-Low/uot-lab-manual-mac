#include "wireless-mac-upcalls.h"

namespace ns3 {

WirelessMacUpcalls::WirelessMacUpcalls(RxMacCallback rxCallback)
:rxCallback(rxCallback) {
}

WirelessMacUpcalls::~WirelessMacUpcalls(){
}

void
WirelessMacUpcalls::Receive(Ptr<Packet> pkt)
{
	NS_ASSERT(rxCallback.GetImpl());
	rxCallback(pkt);
}

} /* namespace ns3 */