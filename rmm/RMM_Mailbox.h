#ifndef RMM_MAILBOX_H
#define RMM_MAILBOX_H

#include <qcstring.h>
#include <RMM_MessageComponent.h>

namespace RMM {

/**
 * An RMailbox holds either a (phrase route-addr) or (localpart domain).
 * (localpart domain) is called an addr-spec by RFC822.
 * You can see which type this is by calling phrase().isEmpty(). If it's empty,
 * you have an addr-spec.
 */
class RMailbox : public RMessageComponent {

#include "RMM_Mailbox_generated.h"
        
    public:

        void setPhrase(const QCString &);
        void setRoute(const QCString &);
        void setLocalPart(const QCString &);
        void setDomain(const QCString &);

        QCString phrase();
        QCString route();
        QCString localPart();
        QCString domain();

    private:

        QCString phrase_;
        QCString route_;
        QCString localPart_;
        QCString domain_;
};

}

#endif //RMAILBOX_H
// vim:ts=4:sw=4:tw=78
