#include <kconfig.h>

#include "profilefilemanager.h"


using namespace KSync;

/*
 * nothing to do here
 */
ProfileFileManager::ProfileFileManager() {

}

/*
 * nothing to do here
 */
ProfileFileManager::~ProfileFileManager() {


}
/*
 * saves Profils
 */
void ProfileFileManager::save( const QValueList<Profile>& list) {
    QValueList<Profile>::ConstIterator it;
    KConfig conf("kitchensync_profiles");
    clear( &conf ); // clear the config first
    QStringList strlist;
    for (it = list.begin(); it != list.end(); ++it ) {
        strlist << (*it).uid();
        saveOne( &conf, (*it) );
    }
    conf.setGroup("General");
    conf.writeEntry("Keys", strlist );
}

/*
 * loads one from file
 */
QValueList<Profile> ProfileFileManager::load()  {
    QValueList<Profile> list;
    KConfig conf("kitchensync_profiles");

    conf.setGroup("General");
    QStringList keys = conf.readListEntry("Keys");
    QStringList::Iterator it;

    for (it = keys.begin(); it != keys.end(); ++it ) {
        conf.setGroup( (*it) );
        list.append( readOne( &conf) );
    };

    return list;
}
void ProfileFileManager::clear( KConfig* conf ) {
    QStringList list = conf->groupList();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
        conf->deleteGroup( (*it) );
    }

}
void ProfileFileManager::saveOne( KConfig* conf,  const Profile& prof ) {
    conf->setGroup( prof.uid() );
    conf->writeEntry("Name", prof.name() );
    conf->writeEntry("Pixmap", prof.pixmap() );
    conf->writeEntry("ConfirmDelete", prof.confirmDelete() );
    conf->writeEntry("ConfirmSync", prof.confirmSync() );

    QMap<QString,QString> paths = prof.paths();
    QMap<QString,QString>::Iterator pathIt;
    QStringList pathlist;
    for ( pathIt = paths.begin(); pathIt != paths.end(); ++pathIt ) {
        pathlist << pathIt.key();
        conf->writeEntry("Path"+pathIt.key(), pathIt.data() );
    }
    conf->writeEntry("LocationPath", pathlist );

    ManPartService::ValueList list = prof.manParts();
    conf->writeEntry("ManPartServices",  list.count() );
    for ( uint i = 0; i < list.count(); i++ ) {
        conf->setGroup(prof.uid() + "ManPart" + QString::number( i ) );
        saveManPart( conf, list[i] );
    }
}
void ProfileFileManager::saveManPart( KConfig* conf, const ManPartService& man ) {
    conf->writeEntry("Name",  man.name() );
    conf->writeEntry("Comment", man.comment() );
    conf->writeEntry("Libname",  man.libname() );
    conf->writeEntry("icon",  man.icon() );
}
Profile ProfileFileManager::readOne(KConfig *conf) {
    Profile prof;
    prof.setUid( conf->group() );
    prof.setName( conf->readEntry("Name") );
    prof.setPixmap( conf->readEntry("Pixmap") );
    prof.setConfirmSync( conf->readBoolEntry("ConfirmSync",  true) );
    prof.setConfirmDelete( conf->readBoolEntry("ConfirmDelete", true) );

    QStringList locationPath = conf->readListEntry("LocationPath");
    QStringList::Iterator it;
    QMap<QString,QString> map;
    for ( it = locationPath.begin(); it != locationPath.end(); ++it ) {
        QString val = conf->readEntry("Path"+(*it) );
        map.insert( (*it),  val );
    };
    prof.setPaths( map );

    // ManPartServices now id + ManPart + number
    int count = conf->readNumEntry("ManPartServices");
    ManPartService::ValueList partList;
    for ( int i = 0; i < count; i++ ) {
        conf->setGroup( prof.uid() +"ManPart" + QString::number( i ) );
        ManPartService service;
        service.setName( conf->readEntry("Name") );
        service.setComment(conf->readEntry("Comment") );
        service.setLibname(conf->readEntry("Libname") );
        service.setIcon(conf->readEntry("icon") );
        partList.append( service );
    }
    prof.setManParts( partList );
    return prof;
}
