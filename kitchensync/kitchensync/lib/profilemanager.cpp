

#include "profilefilemanager.h"
#include "profilemanager.h"

using namespace KSync;

ProfileManager::ProfileManager() {

};
ProfileManager::ProfileManager( const Profile::ValueList& list )
    : m_list( list ) {
}
ProfileManager::~ProfileManager() {

}

Profile ProfileManager::currentProfile() const {
    return m_cur;
}
void ProfileManager::setCurrentProfile( const Profile& prof) {
    m_cur = prof;
}
Profile::ValueList ProfileManager::profiles() const {
    return m_list;
};
void ProfileManager::setProfiles( const Profile::ValueList& list ) {
    m_list = list;
    m_cur = Profile(); // invalidate
}
Profile ProfileManager::byName(const QString& name ) {
    Profile prof;

    Profile::ValueList::Iterator it;
    for ( it = m_list.begin(); it != m_list.end(); ++it ) {
        if ( (*it).name() == name ) {
            prof = (*it);
            break;
        }
    };
    return prof;
}
Profile::ValueList ProfileManager::byName2( const QString& name ) {
    Profile::ValueList list;
    Profile::ValueList::Iterator it;
    for ( it = m_list.begin(); it != m_list.end(); ++it ) {
        if ( (*it).name() == name )
            list.append( (*it) );
    };
    return list;
}
/*
 * Load from KConfig
 * there is a Global group for the current Profile ( id )
 * and one Group for each Profile
 *
 */
void ProfileManager::load() {
    ProfileFileManager man;
    m_list = man.load();
};

void ProfileManager::save() {
    ProfileFileManager man;
    man.save( m_list );
}
void ProfileManager::addProfile( const Profile& prof ) {
    m_list.append( prof );
}
void ProfileManager::replaceProfile( const Profile& prod ) {
    m_list.remove( prod );
    m_list.append( prod );
}
void ProfileManager::removeProfile( const Profile& prof ) {
    m_list.remove( prof );
}
