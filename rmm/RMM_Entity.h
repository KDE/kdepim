#ifndef RMM_ENTITY_H
#define RMM_ENTITY_H

#include <qcstring.h>
#include <RMM_MessageComponent.h>

namespace RMM {

/**
 * @short An REntity is the base class of an RBodyPart and an RMessage.
 * An REntity is the base class of an RBodyPart and an RMessage. Note that the
 * RFC822 specification is recursive. That means that an RBodyPart can also be
 * an RMessage, which then in turn contains an RBodyPart !
 */
class REntity : public RMessageComponent {

    public:

        REntity() : RMessageComponent() {}
        REntity(const REntity & e)  : RMessageComponent(e) {}
        REntity(const QCString & s) : RMessageComponent(s) {}
        virtual ~REntity() {}

        virtual const char * className() const { return "REntity"; }
};

}

#endif
// vim:ts=4:sw=4:tw=78
