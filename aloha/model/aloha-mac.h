#ifndef SLOTTED_ALOHA_MAC_H
#define SLOTTED_ALOHA_MAC_H

#include <iostream>
#include <list>
#include <iterator>
#include <deque>

#include <ns3/log.h>
#include <ns3/packet.h>
#include <ns3/simulator.h>
#include <ns3/node.h>

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/mac48-address.h"
#include "ns3/output-stream-wrapper.h"

#include "ns3/wireless-phy.h"
#include "ns3/wireless-mac-upcalls.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/timer.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

class AlohaMac : public Object {

public:

    static TypeId GetTypeId (void);
    AlohaMac();   

    void SetPhy(Ptr<WirelessPhy> phy);
    Ptr<WirelessPhy> GetPhy(void) const;

    void SetAddress(Address address);
    Address GetAddress (void) const;

    bool Send(Ptr<Packet> packet);
    void Receive(Ptr<Packet> packet);

    virtual void DoDispose() override;
    virtual int64_t AssignStreams(int64_t stream);

    typedef Callback<void, Ptr<const Packet>, const Address &> NetDeviceReceiveCallback;
    void SetReceiveCallback(NetDeviceReceiveCallback callback);
    NetDeviceReceiveCallback GetReceiveCallback (void) const;

    /* Attribute Setters */
    void SetMinBackoffExponent (uint32_t minBackoffExp);  
    void SetMaxBackoffExponent (uint32_t maxBackoffExp);  
    void SetSinkAddress (Mac48Address sinkAddress);
    
protected:

    /* PHY Upcalls */
    void StartCarrierSense(void);
    void EndCarrierSense(void);
    void FinishTransmit(void);

    void Transmit(void);
    void TransmitAck(Mac48Address dst);

    void AckTimeout(void);

private:

    NetDeviceReceiveCallback m_netDeviceReceive;
    Mac48Address m_macAddress;
    Mac48Address m_sinkAddress;
    Ptr<WirelessPhy> m_phy;
    Ptr<WirelessMacUpcalls> m_macUpcalls;
    Ptr<DropTailQueue<Packet>> m_packetQueue;

    uint32_t m_backoffExponent;
    uint32_t m_minBackoffExponent;
    uint32_t m_maxBackoffExponent;

    Ptr<UniformRandomVariable> m_rand;
    Timer m_transmissionTimer;
    Timer m_ackTimer;

    TracedCallback<Mac48Address> m_enqueueTrace;
    TracedCallback<Mac48Address> m_ackTrace;

    
}; /* class AlohaMac */
} /* namespace ns3 */

#endif /* SLOTTED_ALOHA_MAC_H */
