
#ifndef KSYNC_EVENT_SYNCEE_H
#define KSYNC_EVENT_SYNCEE_H

#include "incidencetemplate.h"
#include "synctemplate.h"

#include <event.h>

namespace KSync {
    class EventSyncEntry : public IncidenceTemplate<KCal::Event> {
    public:
        EventSyncEntry(KCal::Event* ev) :
            IncidenceTemplate<KCal::Event>( ev ) {

        };
        SyncEntry* clone();

    };
    class EventSyncee : public SyncTemplate<EventSyncEntry> {
    public:
        EventSyncee();
        Syncee* clone();
        QString type() const;
    };
};

#endif
