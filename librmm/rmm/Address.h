#ifndef RMM_ADDRESS_H
#define RMM_ADDRESS_H

#include <qcstring.h>
#include <qvaluelist.h>

#include <rmm/Mailbox.h>
#include <rmm/HeaderBody.h>

namespace RMM {

/**
 * An Address is schizophrenic. It's either an Group or a Mailbox. Don't
 * worry about it.
 */
class Address : public HeaderBody {

#include "rmm/Address_generated.h"
    
    public:

        enum Type { AddressTypeMailbox, AddressTypeGroup };
        Type type();

        QValueList<Mailbox> mailboxList();

        void setPhrase(const QCString &);
        void setRoute(const QCString &);
        void setLocalPart(const QCString &);
        void setDomain(const QCString &);

        QCString phrase();
        QCString route();
        QCString localPart();
        QCString domain();

    private:

        QValueList<Mailbox> mailboxList_;
        QCString phrase_;
};

}

#endif //RADDRESS_H
// vim:ts=4:sw=4:tw=78
