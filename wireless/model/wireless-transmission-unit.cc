#include "wireless-transmission-unit.h"

namespace ns3 {

TransmissionUnit::TransmissionUnit(Ptr<const TransmissionVector> rxVector)
{
    m_rxVector = rxVector;
    m_corrupted = rxVector->ShouldBeCorrupted();
    
}

TransmissionUnit::~TransmissionUnit(){
}

Ptr<const TransmissionVector>
TransmissionUnit::GetTransmissionVector(void) const
{
    return m_rxVector;
}

bool
TransmissionUnit::IsCorrupted(void) const
{
    return m_corrupted;
}

void
TransmissionUnit::Corrupt(void)
{
    m_corrupted = true;
}

} /* namespace ns3 */