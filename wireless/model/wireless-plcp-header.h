#ifndef WIRELESS_PLCP_H
#define WIRELESS_PLCP_H

#include <stdint.h>
#include <string>
#include "ns3/header.h"
#include "ns3/buffer.h"

namespace ns3 {

class PlcpHeader : public Header {

public: 

    PlcpHeader();
    ~PlcpHeader();

    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream &os) const;
    static uint32_t GetSize(void);
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);

private: 

    /* preamble */
    uint8_t m_syncBits[16];
    uint8_t m_sfd[2];

    /* header */
    uint8_t m_signal;
    uint8_t m_service;
    uint8_t m_length[2];
    uint8_t m_crc[2];

}; /* class PlcpHeader */

} /* namespace ns3 */

#endif /* WIRELESS_PLCP_H */