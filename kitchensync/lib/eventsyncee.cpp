
#include "eventsyncee.h"


using namespace KSync;

EventSyncee::EventSyncee()
    : SyncTemplate<EventSyncEntry>() {

};
QString EventSyncee::type() const{
    return QString::fromLatin1( "EventSyncee" );
}
