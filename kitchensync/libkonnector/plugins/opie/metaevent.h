#ifndef OpieHelper_MetaEvent
#define OpieHelper_MetaEvent

#include <qptrlist.h>
#include <libkcal/event.h>

#include <eventsyncee.h>

#include "metatemplate.h"

namespace OpieHelper {

    class MetaEvent
        : public MetaTemplate<KSync::EventSyncee, KSync::EventSyncEntry>
    {
    public:
        MetaEvent();
        ~MetaEvent();
        virtual bool test( KSync::EventSyncEntry*, KSync::EventSyncEntry* );

    };


};

#endif
