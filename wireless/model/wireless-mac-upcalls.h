#ifndef WIRELESS_MAC_UPCALLS_H
#define WIRELESS_MAC_UPCALLS_H

#include <stdint.h>
#include <ns3/simple-ref-count.h>
#include <ns3/ptr.h>
#include <ns3/tag.h>
#include <ns3/packet.h>

namespace ns3 {

class WirelessMacUpcalls : public SimpleRefCount<WirelessMacUpcalls>{
public:

    typedef Callback<void, Ptr<Packet>> RxMacCallback;
    typedef Callback<void> CarrierSenseCallback;
    typedef Callback<void> FinishTransmitCallback;

    WirelessMacUpcalls(RxMacCallback rxCallback, CarrierSenseCallback startSenseCallback, CarrierSenseCallback endSenseCallback, FinishTransmitCallback finishTransmitCallback);
    virtual ~WirelessMacUpcalls();

    void Receive(Ptr<Packet> packet);
    void StartCarrierSense(void);
    void EndCarrierSense(void);
    void FinishTransmit(void);

private:
    RxMacCallback rxCallback;
    CarrierSenseCallback startSenseCallback;
    CarrierSenseCallback endSenseCallback;
    FinishTransmitCallback finishTransmitCallback;
};

} /* namespace ns3 */

#endif /* WIRELESS_MAC_UPCALLS_H */