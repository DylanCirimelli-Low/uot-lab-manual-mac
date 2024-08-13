#include "ns3/wireless-plcp-header.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
// #include "ns3/unused.h"

namespace ns3 {

PlcpHeader::PlcpHeader()
{
    /* sync is fixed pattern of alternating 0s and 1s */
    //for(uint8_t i = 0; i < 16; i++)
    //    m_syncBits[i] = 0b01010101;

    /* sfd is fixed sequence */
    //m_sfd[0] = 0b11110011;
    //m_sfd[1] = 0b01010000;

    /* we only care about the sizes of fields */
    // NS_UNUSED(m_syncBits);
    // NS_UNUSED(m_sfd);
    // NS_UNUSED(m_signal);
    // NS_UNUSED(m_service);
    // NS_UNUSED(m_length);
    // NS_UNUSED(m_crc);
}

PlcpHeader::~PlcpHeader(){
}

TypeId
PlcpHeader::GetTypeId(void)
{
   static TypeId tid = TypeId ("ns3::PlcpHeader")
     .SetParent<Header> ()
     .SetGroupName ("Wireless")
     .AddConstructor<PlcpHeader> ()
   ;
   return tid;
}

TypeId 
PlcpHeader::GetInstanceTypeId (void) const
{
    return GetTypeId ();
}

void 
PlcpHeader::Print (std::ostream &os) const{
}

uint32_t 
PlcpHeader::GetSize(void)
{
    return 24;
}

uint32_t 
PlcpHeader::GetSerializedSize (void) const
{
    return 24;
}

uint32_t
PlcpHeader::Deserialize(Buffer::Iterator start)
{
    // for(auto byte : m_syncBits){
    //     byte = start.ReadU8();
    // }
    
    // for(auto byte : m_sfd){
    //     byte = start.ReadU8();
    // }
        
    // m_signal = start.ReadU8();
    // m_service = start.ReadU8();

    // for(auto byte : m_length){
    //     byte = start.ReadU8();
    // }
    
    // for(auto byte : m_crc){
    //     byte = start.ReadU8();
    // }

	return GetSerializedSize();
}

void
PlcpHeader::Serialize (Buffer::Iterator start) const
{
    // for(auto byte : m_syncBits){
    //     start.WriteU8(byte);
    // }
    
    // for(auto byte : m_sfd){
    //     start.WriteU8(byte);
    // }

    // start.WriteU8(m_signal);
    // start.WriteU8(m_service);

    // for(auto byte : m_length){
    //     start.WriteU8(byte);
    // }
    
    // for(auto byte : m_crc){
    //     start.WriteU8(byte);
    // }
}

} /* namespace ns3 */
