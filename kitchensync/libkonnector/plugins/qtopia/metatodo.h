#ifndef OPIE_META_TODO_H
#define OPIE_META_TODO_H

#include <todosyncee.h>

#include "md5metatemplate.h"

namespace OpieHelper {
    class MetaTodo : public MD5Template<KSync::TodoSyncee, KSync::TodoSyncEntry> {
    public:
        MetaTodo();
        ~MetaTodo();

        QString string( KSync::TodoSyncEntry* );

    };
};

#endif
