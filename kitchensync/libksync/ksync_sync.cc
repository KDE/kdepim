
#include <kdebug.h>

#include "plugins/kalendar/sync.h"

#include "ksync_sync.h"

using namespace KitchenSync;

SyncManager::SyncManager( QObject *obj,  const char* name )
    : QObject( obj,  name )
{

}
SyncManager::~SyncManager()
{

}
SyncReturn SyncManager::sync( int mode,
                              const KSyncEntryList& first,
                              const KSyncEntryList& out )
{
kdDebug() << "SyncManager" << endl;
    // HACK
    KSyncEntry* entry1;
    KSyncEntry* entry2;
    QPtrListIterator<KSyncEntry> it (first );
    for ( ; it.current(); ++it ) {
        entry1 = it.current();
        QPtrListIterator<KSyncEntry> it2( out );
        for ( ; it2.current(); ++it2 ) {
            entry2 = it2.current();
            if ( entry2->type() == entry1->type() ) {
                kdDebug() << "Type match " << entry1->type() << endl;
                if ( entry1->type() == QString::fromLatin1("KAlendarSyncEntry") ) {
                    SyncKalendar cal(this, "cal");
                    return cal.sync( mode,  entry1,  entry2 );
                }
            }
        }
    }
    return SyncReturn();
}
void SyncManager::syncAsync( int mode,
                             const KSyncEntryList& first,
                             const KSyncEntryList& out )
{
    kdDebug() << "SyncManager" << endl;
    // HACK
    KSyncEntry* entry1;
    KSyncEntry* entry2;
    QPtrListIterator<KSyncEntry> it (first );
    for ( ; it.current(); ++it ) {
        entry1 = it.current();
        QPtrListIterator<KSyncEntry> it2( out );
        for ( ; it2.current(); ++it2 ) {
            entry2 = it2.current();
            if ( entry2->type() == entry1->type() ) {
                kdDebug() << "Type match " << entry1->type() << endl;
                if ( entry1->type() == QString::fromLatin1("KAlendarSyncEntry") ) {
                    SyncKalendar cal(this, "cal");
                    SyncReturn ret = cal.sync( mode,  entry1,  entry2 );
//                    emit
                }
            }
        }
    }
}
void SyncManager::doneSync(const SyncReturn& ret)
{
    emit done( ret );
}
