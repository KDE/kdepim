#ifndef RMM_ADDRESS_H
#define RMM_ADDRESS_H

#include <qcstring.h>
#include <qvaluelist.h>

#include <RMM_Mailbox.h>
#include <RMM_HeaderBody.h>

namespace RMM {

/**
 * An RAddress is schizophrenic. It's either an Group or a Mailbox. Don't
 * worry about it.
 */
class RAddress : public RHeaderBody {

#include "RMM_Address_generated.h"
    
    public:

        enum Type { Mailbox, Group };
        Type type();

        QValueList<RMailbox> mailboxList();

        void setName(const QCString &);
        void setPhrase(const QCString &);
        void setRoute(const QCString &);
        void setLocalPart(const QCString &);
        void setDomain(const QCString &);

        QCString name();
        QCString phrase();
        QCString route();
        QCString localPart();
        QCString domain();

    private:

        QValueList<RMailbox> mailboxList_;
        QCString name_;
        QCString phrase_;
};

}

#endif //RADDRESS_H
// vim:ts=4:sw=4:tw=78
