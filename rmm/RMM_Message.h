#ifndef RMM_MESSAGE_H
#define RMM_MESSAGE_H

#include <RMM_Entity.h>
#include <RMM_Envelope.h>
#include <RMM_BodyPart.h>
#include <RMM_Defines.h>
#include <RMM_Enum.h>

namespace RMM {

class RMessage : public RBodyPart {

#include "RMM_Message_generated.h"

    public:
        
        QCString recipientListAsPlainString();

        void        addPart(RBodyPart & bp);
        void        removePart(RBodyPart & part);
        
        bool hasParentMessageID();

        void setStatus(RMM::MessageStatus status);
        RMM::MessageStatus status();
        
    protected:
        
        RMM::MessageStatus    status_;
};

}

#endif

// vim:ts=4:sw=4:tw=78
