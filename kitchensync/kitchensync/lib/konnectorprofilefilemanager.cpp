#include <kdebug.h>
#include <kconfig.h>

#include "konnectorprofilefilemanager.h"

using namespace KSync;

KonnectorProfileFileManager::KonnectorProfileFileManager()
{
}

KonnectorProfileFileManager::~KonnectorProfileFileManager()
{
}

KonnectorProfile::ValueList KonnectorProfileFileManager::load()
{
    kdDebug() << "KonnectorProfFileManager::load" << endl;

    QStringList ids;
    QStringList::Iterator it;

    KonnectorProfile::ValueList list;

    KConfig conf("kitchensync_konnectors");

    conf.setGroup("General");
    ids = conf.readListEntry("Ids");
    for ( it = ids.begin(); it != ids.end(); ++it ) {
        kdDebug() << "id " << (*it) << endl;
        conf.setGroup( (*it) );
        KonnectorProfile prof;
        prof.loadFromConfig( &conf );

        /* see if it is valid Transputer had an almost empty config
         * only keys and no values which hit an assert later in the mainwindow
         * ...
         */
        if ( prof.isValid() ) list.append(prof );
    }

    kdDebug() << "KonnectorProfFileManager::load() done" << endl;

    return list;
}

void KonnectorProfileFileManager::save( const KonnectorProfile::ValueList& list)
{
//    kdDebug() << "Saving Profiles " << endl;
    KonnectorProfile::ValueList::ConstIterator it;
    KConfig conf("kitchensync_konnectors");
    QStringList ids;

    for ( it = list.begin();  it != list.end(); ++it ) {
        ids << (*it).uid();
//        kdDebug() << "saving id " << (*it).uid() << " name " << (*it).name() << endl;
        (*it).saveToConfig( &conf );
    }
    conf.setGroup("General");
    conf.writeEntry("Ids", ids );
    conf.sync();
}
