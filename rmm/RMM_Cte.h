#ifndef RMM_CTE_H
#define RMM_CTE_H

#include <qcstring.h>

#include <RMM_Defines.h>
#include <RMM_Enum.h>
#include <RMM_Parameter.h>
#include <RMM_HeaderBody.h>

namespace RMM {

/**
 * An RCte holds a Content-Transfer-Encoding header body. It contains a
 * mechanism. This is likely to be "7bit, "quoted-printable", "base64", "8bit",
 * "binary" or an 'x-token'. An x-token is an extension token and is prefixed
 * by 'X-'.
 */
class RCte : public RHeaderBody {

#include "RMM_Cte_generated.h"

    public:
        
        RMM::CteType mechanism();
        void setMechanism(RMM::CteType);
        
    private:

        RMM::CteType mechanism_;
};

}

#endif

// vim:ts=4:sw=4:tw=78
