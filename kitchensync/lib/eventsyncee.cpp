
#include "eventsyncee.h"


using namespace KSync;

SyncEntry* EventSyncEntry::clone()  {
    return new EventSyncEntry( *this );
}

EventSyncee::EventSyncee()
    : SyncTemplate<EventSyncEntry>() {

};
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
