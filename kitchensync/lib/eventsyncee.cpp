
#include <calformat.h>

#include <kstaticdeleter.h>
#include "eventsyncee.h"


using namespace KSync;

SyncEntry* EventSyncEntry::clone()  {
    return new EventSyncEntry( *this );
}

EventSyncee::EventSyncee()
    : SyncTemplate<EventSyncEntry>(DtEnd+1) {

}
/* merging hell! */
namespace{
    typedef MergeBase<KCal::Event, EventSyncee> MergeEvent;
    static MergeEvent* mergeMap = 0l;
    static KStaticDeleter<MergeEvent> deleter;
    void mergeDtEnd( KCal::Event* const dest, const KCal::Event* src) {
        dest->setDtEnd( src->dtEnd() );
    }
    MergeEvent* map() {
        if (!mergeMap ) {
            deleter.setObject( mergeMap, new MergeEvent );
            mergeMap->add( EventSyncee::DtEnd, mergeDtEnd );
        }
        return mergeMap;
    }
}
bool EventSyncEntry::mergeWith( SyncEntry* entry ) {
    if ( entry->name() != name() || !syncee() || !entry->syncee() )
        return false;
    EventSyncEntry* toEv = static_cast<EventSyncEntry*>(entry);
    QBitArray da = toEv->syncee()->bitArray();
    QBitArray hier = syncee()->bitArray();

    for (uint i = 0; i< da.count() && i < hier.count(); i++ ) {
        if (da[i] && !hier[i] )
            map()->invoke(i, incidence(), toEv->incidence() );
    }

    return true;
}

QString EventSyncee::type() const{
    return QString::fromLatin1( "EventSyncee" );
}
Syncee* EventSyncee::clone() {
    EventSyncee* temp = new EventSyncee();
    temp->setSyncMode( syncMode() );
    temp->setFirstSync( firstSync() );
    EventSyncEntry* entry;
    for ( entry = mList.first(); entry != 0; entry = mList.next() ) {
        temp->addEntry( entry->clone() );
    }
    return temp;
}
QString EventSyncee::newId() const {
    return KCal::CalFormat::createUniqueId();
}
