#include "ns3/aloha-header.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

AlohaHeader::AlohaHeader(void){
    m_src = Mac48Address();
    m_dst = Mac48Address();
}

AlohaHeader::AlohaHeader(Address src, Address dst){
    m_src = Mac48Address::ConvertFrom(src);
    m_dst = Mac48Address::ConvertFrom(dst);
}

AlohaHeader::~AlohaHeader(){
}

TypeId
AlohaHeader::GetTypeId(void)
{
    static TypeId tid = TypeId ("ns3::AlohaHeader")
        .SetParent<Header> ()
        .SetGroupName ("Aloha")
        .AddConstructor<AlohaHeader> ()
    ;
    return tid;
}

TypeId
AlohaHeader::GetInstanceTypeId (void) const
{
    return GetTypeId();
}

void
AlohaHeader::Print (std::ostream &os) const
{
    os << "[src = " << m_src << ", dst = " << m_dst << "]";
}

uint32_t
AlohaHeader::GetSize(void)
{
    return 12;
}

uint32_t
AlohaHeader::GetSerializedSize (void) const
{
    return 12;
}

void
AlohaHeader::Serialize (Buffer::Iterator start) const
{
    uint8_t srcBuffer[6];
    m_src.CopyTo(srcBuffer);

    for(uint8_t byte : srcBuffer){
        start.WriteU8(byte);
    }

    uint8_t dstBuffer[6];
    m_dst.CopyTo(dstBuffer);

    for(uint8_t byte : dstBuffer){
        start.WriteU8(byte);
    }
}

uint32_t
AlohaHeader::Deserialize(Buffer::Iterator start)
{
    uint8_t src[6];
    for(int i = 0; i < 6; i++)
        src[i] = start.ReadU8();

    m_src.CopyFrom(src);

    uint8_t dst[6];
    for(int i = 0; i < 6; i++)
        dst[i] = start.ReadU8();

    m_dst.CopyFrom(dst);

    return GetSerializedSize();
}

Mac48Address
AlohaHeader::GetSrc(void) const
{
    return m_src;
}

Mac48Address
AlohaHeader::GetDst(void) const
{
    return m_dst;
}

} /* namespace ns3 */