#include <kdebug.h>
#include <kconfig.h>

#include "konnectorprofilefilemanager.h"

using namespace KSync;

KonnectorProfileFileManager::KonnectorProfileFileManager() {}
KonnectorProfileFileManager::~KonnectorProfileFileManager() {}

KonnectorProfile::ValueList KonnectorProfileFileManager::load() {
    KonnectorProfile::ValueList list;
    kdDebug() << "KonnectorProfFileManager::load" << endl;
    KConfig conf("kitchensync_konnectors");
    QStringList ids;
    QStringList::Iterator it;

    conf.setGroup("General");
    ids = conf.readListEntry("Ids");
    for ( it = ids.begin(); it != ids.end(); ++it ) {
        kdDebug() << "id " << (*it) << endl;
        conf.setGroup( (*it) );
        KonnectorProfile prof;
        prof.loadFromConfig(&conf );
        list.append(prof );
    }

    return list;
}
void KonnectorProfileFileManager::save( const KonnectorProfile::ValueList& list) {
    kdDebug() << "Saving Profiles " << endl;
    KonnectorProfile::ValueList::ConstIterator it;
    KConfig conf("kitchensync_konnectors");
    QStringList ids;

    for ( it = list.begin();  it != list.end(); ++it ) {
        ids << (*it).uid();
        kdDebug() << "saving id " << (*it).uid() << " name " << (*it).name() << endl;
        (*it).saveToConfig( &conf );
    }
    conf.setGroup("General");
    conf.writeEntry("Ids", ids );
    conf.sync();
}
