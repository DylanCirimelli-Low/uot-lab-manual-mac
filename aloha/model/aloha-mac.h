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

class AlohaMacPacketTag : public Tag
{
  public:
    /**
     * \brief Constructor
     * \param uid the packet uid
     */
    AlohaMacPacketTag(uint32_t uid = -1, uint32_t size = -1)
        : Tag(),
          m_uid(uid),
          m_size(size)
    {
    }

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */ 
    static TypeId GetTypeId()
    {
        static TypeId tid = TypeId("ns3::Aloha::AlohaMacPacketTag")
                                .SetParent<Tag>()
                                .SetGroupName("Aloha")
                                .AddConstructor<AlohaMacPacketTag>();
        return tid;
    }

    TypeId GetInstanceTypeId() const override
    {
        return GetTypeId();
    }

    uint32_t GetPacketUid() const
    {
        return m_uid;
    }

    uint32_t GetPacketSize() const
    {
        return m_size;
    }

    uint32_t GetSerializedSize() const override
    {
        return sizeof(uint32_t) * 2;
    }

    void Serialize(TagBuffer i) const override
    {
        i.WriteU32(m_uid);
        i.WriteU32(m_size);
    }

    void Deserialize(TagBuffer i) override
    {
        m_uid = i.ReadU32();
        m_size = i.ReadU32();
    }

    void Print(std::ostream& os) const override
    {
        os << "" << m_uid << ", " << m_size;
    }

  private:
    uint32_t m_uid;
    uint32_t m_size;
};

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
    void TransmitAck(Ptr<const Packet> p, Mac48Address dst);

    void StartBackoff(void);
    void AckTimeout(void);

private:

    Time GetAckTime(void) const;

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

    TracedCallback<Ptr<ns3::Packet const>> m_enqueueTrace;
    TracedCallback<Ptr<ns3::Packet const>> m_ackTrace;
    TracedCallback<Ptr<ns3::Packet const>> m_macTxTrace;

    bool m_usePriorityAcks;
    bool m_useCarrierSensing;

    
}; /* class AlohaMac */
} /* namespace ns3 */

#endif /* SLOTTED_ALOHA_MAC_H */
