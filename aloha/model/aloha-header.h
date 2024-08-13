#ifndef SLOTTED_ALOHA_ACK_H
#define SLOTTED_ALOHA_ACK_H

#include <stdint.h>
#include <string>
#include "ns3/buffer.h"
#include "ns3/header.h"
#include "ns3/mac48-address.h"

namespace ns3 {

class AlohaHeader : public Header {

public:

    AlohaHeader(void);
    AlohaHeader(Address src, Address dst);
    ~AlohaHeader();

    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream &os) const;
    static uint32_t GetSize(void);
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);

    Mac48Address GetSrc(void) const;
    Mac48Address GetDst(void) const;

private:

    Mac48Address m_src;
    Mac48Address m_dst;

}; /* class AlohaHeader */

} /* namespace ns3 */
#endif /* SLOTTED_ALOHA_ACK_H */