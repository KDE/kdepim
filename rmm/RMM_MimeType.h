#ifndef RMM_MIMETYPE_H
#define RMM_MIMETYPE_H

#include <qcstring.h>

#include <RMM_HeaderBody.h>
#include <RMM_Parameter.h>
#include <RMM_ParameterList.h>
#include <RMM_Enum.h>

namespace RMM {

class RMimeType : public RHeaderBody {

#include "RMM_MimeType_generated.h"

    public:

        QCString boundary();
        QCString name();

        RMM::MimeType type();
        RMM::MimeSubType subType();

        void setType(RMM::MimeType);
        void setType(const QCString &);
        void setSubType(RMM::MimeSubType);
        void setSubType(const QCString &);

        void setBoundary(const QCString & boundary);
        void setName(const QCString & name);

    private:

        QCString            boundary_;
        QCString            name_;
        
        RMM::MimeType       type_;
        RMM::MimeSubType    subType_;
        
        RParameterList      parameterList_;
};

}

#endif
// vim:ts=4:sw=4:tw=78
