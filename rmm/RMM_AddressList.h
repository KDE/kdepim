#ifndef RMM_ADDRESS_LIST_H
#define RMM_ADDRESS_LIST_H

#include <qcstring.h>
#include <qvaluelist.h>
#include <RMM_Address.h>
#include <RMM_Defines.h>

namespace RMM {

/**
 * @short Simple encapsulation of a list of RAddress, which is also an
 * RHeaderBody.
 */
class RAddressList : public RHeaderBody {

#include "RMM_AddressList_generated.h"

    public:
        
        RAddress at(unsigned int);
        unsigned int count();

    private:
        
        QValueList<RAddress> list_;
};

}

#endif //RADDRESSLIST_H
// vim:ts=4:sw=4:tw=78
