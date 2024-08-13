#ifndef WIRELESS_TRANSMISSION_UNIT_H
#define WIRELESS_TRANSMISSION_UNIT_H

#include "ns3/wireless-transmission-vector.h"

namespace ns3 {

class TransmissionUnit: public SimpleRefCount<TransmissionUnit>
{
    public:

        TransmissionUnit(Ptr<const TransmissionVector> rxVector);
        virtual ~TransmissionUnit();

        Ptr<const TransmissionVector> GetTransmissionVector(void) const;
        bool IsCorrupted(void) const;
        void Corrupt(void);

    private:

        Ptr<const TransmissionVector> m_rxVector;
        bool m_corrupted;
        
};

} /* namespace ns3 */

#endif /* WIRELESS_TRANSMISSION_UNIT_H */