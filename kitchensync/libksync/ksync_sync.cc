
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
                              const QPtrList<KSyncEntry>& first,
                              const QPtrList<KSyncEntry>& second )
{

}
void SyncManager::syncAsync( int mode,
                             const QPtrList<KSyncEntry>& first,
                             const QPtrList<KSyncEntry>& out )
{

}
void SyncManager::doneSync(const SyncReturn& ret)
{
    emit done( ret );
}
