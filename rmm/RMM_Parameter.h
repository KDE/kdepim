#ifndef RMM_PARAMETER_H
#define RMM_PARAMETER_H

#include <qcstring.h>

#include <RMM_MessageComponent.h>

namespace RMM {

/**
 * An RParameter consists of an attribute, value pair.
 * It is used in RParameterList, for example when looking at an RCte field.
 */
class RParameter : public RMessageComponent {

#include "RMM_Parameter_generated.h"
        
    public:

        QCString attribute();
        QCString value();

        void setAttribute    (const QCString & attribute);
        void setValue        (const QCString & value);
        
    private:

        QCString attribute_;
        QCString value_;
};

}

#endif
// vim:ts=4:sw=4:tw=78
