#ifndef RMM_HEADER_H
#define RMM_HEADER_H

#include <qlist.h>

#include <RMM_Enum.h>
#include <RMM_HeaderBody.h>
#include <RMM_Defines.h>

namespace RMM {

/**
 * An RHeader encapsulates an header name and an RHeaderBody.
 */
class RHeader : public RMessageComponent
{

#include "RMM_Header_generated.h"
        
    public:
        
        QCString headerName();
        RMM::HeaderType headerType();
        RHeaderBody * headerBody();

        void setName(const QCString & name);
        void setType(RMM::HeaderType t);
        void setBody(RHeaderBody * b);

    private:
 
        RHeaderBody * _newHeaderBody(RMM::HeaderType);
        RHeaderBody * _newHeaderBody(RMM::HeaderType, RHeaderBody *);
        
        QCString        headerName_;
        RMM::HeaderType headerType_;
        RHeaderBody *   headerBody_;
};

typedef QListIterator<RHeader> RHeaderListIterator;
typedef QList<RHeader> RHeaderList;

}

#endif

// vim:ts=4:sw=4:tw=78
