#ifndef RMM_HEADER_BODY_H
#define RMM_HEADER_BODY_H

#include <RMM_MessageComponent.h>

namespace RMM {

/**
 * Base class for RAddress, RContentType, RAddressList etc.
 */
class RHeaderBody : public RMessageComponent {

#include "RMM_HeaderBody_generated.h"
        
};

};

#endif
// vim:ts=4:sw=4:tw=78
