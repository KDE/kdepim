#ifndef RMM_MESSAGEID_H
#define RMM_MESSAGEID_H

#include <qcstring.h>
#include <RMM_HeaderBody.h>

namespace RMM {

/**
 * An RMessageID encapsulates the body of a Message-Id header field as defined
 * by RFC822. This means it has two strings, a local-part and a domain.
 */
class RMessageID : public RHeaderBody {

#include "RMM_MessageID_generated.h"

    public:
        
        QCString    localPart();
        QCString    domain();
        void        setLocalPart(const QCString & localPart);
        void        setDomain(const QCString & domain);
        
    private:

        static int seq_;
        QCString localPart_;
        QCString domain_;
};

}

#endif //RMESSAGEID_H
// vim:ts=4:sw=4:tw=78
