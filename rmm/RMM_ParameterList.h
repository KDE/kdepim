#ifndef RMM_PARAMETERLIST_H
#define RMM_PARAMETERLIST_H

#include <qvaluelist.h>

#include <RMM_Parameter.h>
#include <RMM_HeaderBody.h>

namespace RMM {

/**
 * @short Simple encapsulation of a list of RParameter, which is also an
 * RHeaderBody.
 */
class RParameterList : public RHeaderBody {

#include "RMM_ParameterList_generated.h"

    public:

        QValueList<RParameter> list();
        void setList(QValueList<RParameter> &);

    private:

        QValueList<RParameter> list_;

};

}

#endif //RPARAMETERLIST_H

// vim:ts=4:sw=4:tw=78
