
#ifndef KSYNC_EVENT_SYNCEE_H
#define KSYNC_EVENT_SYNCEE_H

#include "incidencetemplate.h"
#include "synctemplate.h"

#include <event.h>

namespace KSync {
    class EventSyncEntry : public IncidenceTemplate<KCal::Event> {
    public:
        EventSyncEntry(KCal::Event* ev = 0l) :
            IncidenceTemplate<KCal::Event>( ev ) {
        };
        SyncEntry* clone();
        bool mergeWith( SyncEntry* );

    };
    class EventSyncee : public SyncTemplate<EventSyncEntry> {
    public:
        enum Supports  {
        // InicdenceBase
            Organizer = 0,
            ReadOnly,
            DtStart,
            Duration,
            Float,
            // Inicdence
            Attendee,
            CreatedDate,
            Revision,
            Description,
            Summary,
            Category,
            Relations,
            ExDates,
            Attachments,
            Secrecy,
            Resources,
            Priority,
            Alarms,
            Recurrence,
            Location,
            // Event
            DtEnd
        };
        EventSyncee();
        Syncee* clone();
        QString type() const;
        QString newId() const;
    };
};

#endif
