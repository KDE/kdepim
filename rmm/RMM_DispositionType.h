#ifndef RMM_DISPOSITION_TYPE_H
#define RMM_DISPOSITION_TYPE_H

#include <qcstring.h>

#include <RMM_Enum.h>
#include <RMM_HeaderBody.h>
#include <RMM_Defines.h>
#include <RMM_Parameter.h>
#include <RMM_ParameterList.h>

namespace RMM {

class RDispositionType : public RHeaderBody {

#include "RMM_DispositionType_generated.h"

    public:
        
        QCString            filename();
        void                setFilename(const QCString &);
        void                addParameter(RParameter & p);
        RParameterList &    parameterList();
        RMM::DispType       type();

    private:

        RParameterList      parameterList_;
        RMM::DispType       dispType_;
        QCString            filename_;
};

}

#endif //RDISPOSITIONTYPE_H
// vim:ts=4:sw=4:tw=78
