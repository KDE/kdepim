#ifndef RMM_CONTENT_TYPE_H
#define RMM_CONTENT_TYPE_H

#include <qcstring.h>

#include <RMM_HeaderBody.h>
#include <RMM_ParameterList.h>

namespace RMM {

/**
 * An RContentType has a mime type, a mime subtype, and a parameter list.
 */
class RContentType : public RHeaderBody {

#include "RMM_ContentType_generated.h"

    public:

        void setType(const QCString &);
        void setSubType(const QCString &);
        void setParameterList(RParameterList &);
        
        QCString type();
        QCString subType();
        RParameterList & parameterList();

    private:

        QCString        type_;
        QCString        subType_;
        RParameterList  parameterList_;
};

}

#endif //RMM_CONTENTTYPE_H
// vim:ts=4:sw=4:tw=78
