
#ifndef OpieHelper_MetaTodo
#define OpieHelper_MetaTodo

#include <qptrlist.h>

#include <libkcal/todo.h>

#include <todosyncee.h>

#include "metatemplate.h"

namespace OpieHelper {
    class MetaTodo : public MetaTemplate<KSync::TodoSyncee, KSync::TodoSyncEntry> {
    public:
        MetaTodo();
        ~MetaTodo();
        /**
         * doMeta searches for differences between a new and
         * old set of Todo Events
         */
        virtual bool test( KSync::TodoSyncEntry*, KSync::TodoSyncEntry* );
    };
};

#endif
