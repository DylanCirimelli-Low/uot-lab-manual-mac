#ifndef WIRELESS_NET_DEVICE_H
#define WIRELESS_NET_DEVICE_H

#include "ns3/address.h"
#include "ns3/backoff.h"
#include "ns3/callback.h"
#include "ns3/data-rate.h"
#include "ns3/mac48-address.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/queue-fwd.h"
#include "ns3/traced-callback.h"

#include <cstring>

namespace ns3
{

class WirelessPhy;
class WirelessChannel;
class ErrorModel;
class WirelessMacUpcalls;

/**
 * \defgroup csma CSMA Network Device
 *
 * This section documents the API of the ns-3 csma module. For a generic functional description,
 * please refer to the ns-3 manual.
 */

/**
 * \ingroup csma
 * \class WirelessNetDevice
 *
 * \brief A Device for a Csma Network Link.
 *
 * The Csma net device class is analogous to layer 1 and 2 of the
 * TCP stack. The NetDevice takes a raw packet of bytes and creates a
 * protocol specific packet from them.
 */
class WirelessNetDevice : public NetDevice
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    /**
     * Construct a WirelessNetDevice
     *
     *
     * This is the default constructor for a WirelessNetDevice
     *.
     */
    WirelessNetDevice();

    /**
     * Destroy a WirelessNetDevice
     *
     *
     * This is the destructor for a WirelessNetDevice
     *.
     */
    ~WirelessNetDevice() override;

    /**
     * Attach the device to a channel.
     *
     * The function Attach is used to add a WirelessNetDevice
     * to a WirelessChannel.
     *
     * \see SetDataRate ()
     * \see SetInterframeGap ()
     * \param ch a pointer to the channel to which this object is being attached.
     * \returns true if no error
     */
    bool Attach(Ptr<WirelessChannel> ch);

    /**
     * Attach a queue to the WirelessNetDevice
     *.
     *
     * The WirelessNetDevice
     * "owns" a queue.  This queue may be set by higher
     * level topology objects to implement a particular queueing method such as
     * DropTail.
     *
     * \see Queue
     * \see DropTailQueue
     * \param queue a Ptr to the queue for being assigned to the device.
     */
    void SetQueue(Ptr<Queue<Packet>> queue);

    /**
     * Get a copy of the attached Queue.
     *
     * \return a pointer to the queue.
     */
    Ptr<Queue<Packet>> GetQueue() const;

    /**
     * Receive a packet from a connected WirelessChannel.
     *
     * The WirelessNetDevice
     * receives packets from its connected channel
     * and forwards them up the protocol stack.  This is the public method
     * used by the channel to indicate that the last bit of a packet has
     * arrived at the device.
     *
     * \see WirelessChannel
     * \param p a reference to the received packet
     * that transmitted the packet in the first place
     */
    void Receive(Ptr<Packet> p);

    //
    // The following methods are inherited from NetDevice base class.
    //
    void SetIfIndex(const uint32_t index) override;
    uint32_t GetIfIndex() const override;
    Ptr<Channel> GetChannel() const override;
    bool SetMtu(const uint16_t mtu) override;
    uint16_t GetMtu() const override;
    void SetAddress(Address address) override;
    Address GetAddress() const override;
    bool IsLinkUp() const override;
    void AddLinkChangeCallback(Callback<void> callback) override;
    bool IsBroadcast() const override;
    Address GetBroadcast() const override;
    bool IsMulticast() const override;

    /**
     * \brief Make and return a MAC multicast address using the provided
     *        multicast group
     *
     * \RFC{1112} says that an Ipv4 host group address is mapped to an Ethernet
     * multicast address by placing the low-order 23-bits of the IP address into
     * the low-order 23 bits of the Ethernet multicast address
     * 01-00-5E-00-00-00 (hex).
     *
     * This method performs the multicast address creation function appropriate
     * to an EUI-48-based CSMA device.  This MAC address is encapsulated in an
     *  abstract Address to avoid dependencies on the exact address format.
     *
     * \param multicastGroup The IP address for the multicast group destination
     * of the packet.
     * \return The MAC multicast Address used to send packets to the provided
     * multicast group.
     *
     * \see Ipv4Address
     * \see Mac48Address
     * \see Address
     */
    Address GetMulticast(Ipv4Address multicastGroup) const override;

    /**
     * Is this a point to point link?
     * \returns false.
     */
    bool IsPointToPoint() const override;

    /**
     * Is this a bridge?
     * \returns false.
     */
    bool IsBridge() const override;

    /**
     * Start sending a packet down the channel.
     * \param packet packet to send
     * \param dest layer 2 destination address
     * \param protocolNumber protocol number
     * \return true if successful, false otherwise (drop, ...)
     */
    bool Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber) override;

    /**
     * Start sending a packet down the channel, with MAC spoofing
     * \param packet packet to send
     * \param source layer 2 source address
     * \param dest layer 2 destination address
     * \param protocolNumber protocol number
     * \return true if successful, false otherwise (drop, ...)
     */
    bool SendFrom(Ptr<Packet> packet,
                  const Address& source,
                  const Address& dest,
                  uint16_t protocolNumber) override;

    /**
     * Get the node to which this device is attached.
     *
     * \returns Ptr to the Node to which the device is attached.
     */
    Ptr<Node> GetNode() const override;

    /**
     * Set the node to which this device is being attached.
     *
     * \param node Ptr to the Node to which the device is being attached.
     */
    void SetNode(Ptr<Node> node) override;

    /**
     * Does this device need to use the address resolution protocol?
     *
     * \returns True if the encapsulation mode is set to a value that requires
     * ARP (IP_ARP or LLC).
     */
    bool NeedsArp() const override;

    /**
     * Set the callback to be used to notify higher layers when a packet has been
     * received.
     *
     * \param cb The callback.
     */
    void SetReceiveCallback(NetDevice::ReceiveCallback cb) override;

    /**
     * \brief Get the MAC multicast address corresponding
     * to the IPv6 address provided.
     * \param addr IPv6 address
     * \return the MAC multicast address
     * \warning Calling this method is invalid if IsMulticast returns not true.
     */
    Address GetMulticast(Ipv6Address addr) const override;

    void SetPromiscReceiveCallback(PromiscReceiveCallback cb) override;
    bool SupportsSendFrom() const override;

    /**
     * Assign a fixed random variable stream number to the random variables
     * used by this model.  Return the number of streams (possibly zero) that
     * have been assigned.
     *
     * \param stream first stream index to use
     * \return the number of stream indices assigned by this model
     */
    int64_t AssignStreams(int64_t stream);

    void SetPhy(Ptr<WirelessPhy> phy);
    Ptr<WirelessPhy> GetPhy(void) const;

  protected:
    /**
     * Perform any object release functionality required to break reference
     * cycles in reference counted objects held by the device.
     */
    void DoDispose() override;

    /**
     * Adds the necessary headers and trailers to a packet of data in order to
     * respect the packet type
     *
     * \param p Packet to which header should be added
     * \param source MAC source address from which packet should be sent
     * \param dest MAC destination address to which packet should be sent
     * \param protocolNumber In some protocols, identifies the type of
     * payload contained in this packet.
     */
    void AddHeader(Ptr<Packet> p, Mac48Address source, Mac48Address dest, uint16_t protocolNumber);

  private:
    /**
     * Operator = is declared but not implemented.  This disables the assignment
     * operator for WirelessNetDevice
     * objects.
     * \param o object to copy
     * \returns the copied object
     */
    WirelessNetDevice& operator=(const WirelessNetDevice& o);

    /**
     * Copy constructor is declared but not implemented.  This disables the
     * copy constructor for WirelessNetDevice
     * objects.
     * \param o object to copy
     */
    WirelessNetDevice(const WirelessNetDevice& o);

    /**
     * Initialization function used during object construction.
     * \param sendEnable if device will be allowed to send
     * \param receiveEnable if device will be allowed to receive
     */
    void Init(bool sendEnable, bool receiveEnable);

    /**
     * Start Sending a Packet Down the Wire.
     *
     * The TransmitStart method is the method that is used internally in
     * the WirelessNetDevice
     * to begin the process of sending a packet
     * out on the channel.  A corresponding method is called on the
     * channel to let it know that the physical device this class
     * represents has actually started sending signals, this causes the
     * channel to enter the BUSY state.  An event is scheduled for the time at
     * which the bits have been completely transmitted.
     *
     * If the channel is found to be BUSY, this method reschedules itself for
     * execution at a later time (within the backoff period).
     *
     * \see WirelessChannel::TransmitStart ()
     * \see TransmitCompleteEvent ()
     */
    void TransmitStart();

    /**
     * Stop Sending a Packet Down the Wire and Begin the Interframe Gap.
     *
     * The TransmitCompleteEvent method is used internally to finish the process
     * of sending a packet out on the channel.  During execution of this method
     * the TransmitEnd method is called on the channel to let it know that the
     * physical device this class represents has finished sending simulated
     * signals.  The channel uses this event to begin its speed of light delay
     * timer after which it notifies the Net Device(s) at the other end of the
     * link that new bits have arrived (it delivers the Packet).  During this
     * method, the net device also schedules the TransmitReadyEvent at which
     * time the transmitter becomes ready to send the next packet.
     *
     * \see WirelessChannel::TransmitEnd ()
     * \see TransmitReadyEvent ()
     */
    void TransmitCompleteEvent();

    /**
     * Cause the Transmitter to Become Ready to Send Another Packet.
     *
     * The TransmitReadyEvent method is used internally to re-enable the
     * transmit machine of the net device.  It is scheduled after a suitable
     * interframe gap after the completion of the previous transmission.
     * The queue is checked at this time, and if there is a packet waiting on
     * the queue, the transmission process is begun.
     *
     * If a packet is in the queue, it is extracted for the queue as the
     * next packet to be transmitted by the net device.
     *
     * \see TransmitStart ()
     */
    void TransmitReadyEvent();

    /**
     * Notify any interested parties that the link has come up.
     */
    void NotifyLinkUp();

    /**
     * Device ID returned by the attached functions. It is used by the
     * mp-channel to identify each net device to make sure that only
     * active net devices are writing to the channel
     */
    uint32_t m_deviceId;

    /**
     * Enumeration of the states of the transmit machine of the net device.
     */
    enum TxMachineState
    {
        READY,  /**< The transmitter is ready to begin transmission of a packet */
        BUSY,   /**< The transmitter is busy transmitting a packet */
        GAP,    /**< The transmitter is in the interframe gap time */
    };

    /**
     * The state of the Net Device transmit state machine.
     * \see TxMachineState
     */
    TxMachineState m_txMachineState;

    /**
     * Next packet that will be transmitted (if transmitter is not
     * currently transmitting) or packet that is currently being
     * transmitted.
     */
    Ptr<Packet> m_currentPkt;

    /**
     * The WirelessPhy to which this WirelessNetDevice
     * has been attached.
     * \see class WirelessPhy
     */
    Ptr<WirelessPhy> m_phy;

    /**
     * The WirelessPhy to which this WirelessNetDevice
     * has been attached.
     * \see class WirelessPhy
     */
    Ptr<WirelessChannel> m_channel;

    /**
     * The Queue which this WirelessNetDevice
     * uses as a packet source.
     * Management of this Queue has been delegated to the WirelessNetDevice
     *
     * and it has the responsibility for deletion.
     * \see class Queue
     * \see class DropTailQueue
     */
    Ptr<Queue<Packet>> m_queue;

    /**
     * The Node to which this device is attached.
     */
    Ptr<Node> m_node;

    /**
     * The MAC address which has been assigned to this device.
     */
    Mac48Address m_address;

    /**
     * The callback used to notify higher layers that a packet has been received.
     */
    NetDevice::ReceiveCallback m_rxCallback;

    /**
     * The callback used to notify higher layers that a packet has been received in promiscuous
     * mode.
     */
    NetDevice::PromiscReceiveCallback m_promiscRxCallback;

    /**
     * The interface index (really net evice index) that has been assigned to
     * this network device.
     */
    uint32_t m_ifIndex;

    /**
     * Flag indicating whether or not the link is up.  In this case,
     * whether or not the device is connected to a channel.
     */
    bool m_linkUp;

    /**
     * List of callbacks to fire if the link changes state (up or down).
     */
    TracedCallback<> m_linkChangeCallbacks;

    /**
     * Default Maximum Transmission Unit (MTU) for the WirelessNetDevice
     *
     */
    static const uint16_t DEFAULT_MTU = 1500;

    /**
     * The Maximum Transmission Unit.  This corresponds to the maximum
     * number of bytes that can be transmitted as seen from higher layers.
     * This corresponds to the 1500 byte MTU size often seen on IP over
     * Ethernet.
     */
    uint32_t m_mtu;

    Ptr<WirelessMacUpcalls> m_macUpcalls;
    
};

} // namespace ns3

#endif /* CSMA_NET_DEVICE_H */
