#ifndef RMM_CONTENT_TYPE_H
#define RMM_CONTENT_TYPE_H

#include <qcstring.h>

#include <rmm/HeaderBody.h>
#include <rmm/ParameterList.h>

namespace RMM {

/**
 * A ContentType has a mime type, a mime subtype, and a parameter list.
 */
class ContentType : public HeaderBody {

#include "rmm/ContentType_generated.h"

    public:

        void setType(const QCString &);
        void setSubType(const QCString &);
        void setParameterList(ParameterList &);
        
        QCString type();
        QCString subType();
        ParameterList & parameterList();

    private:

        QCString        type_;
        QCString        subType_;
        ParameterList  parameterList_;
};

}

#endif //rmm/CONTENTTYPE_H
// vim:ts=4:sw=4:tw=78
