#include <kdebug.h>

#include "konnectorprofilefilemanager.h"
#include "konnectorprofilemanager.h"

using namespace KSync;

KonnectorProfileManager::KonnectorProfileManager() {}

KonnectorProfileManager::KonnectorProfileManager( const KonnectorProfile::ValueList& list )
{
    setList( list );
}

KonnectorProfileManager::~KonnectorProfileManager()
{
}

KonnectorProfile KonnectorProfileManager::find( const QString& id )
{
    KonnectorProfile prof;
    KonnectorProfile::ValueList::Iterator it;
    for ( it = m_list.begin(); it != m_list.end(); ++it ) {
        if ( (*it).name() == id || (*it).uid() == id ) {
            prof = (*it);
            break;
        }
    }
    return prof;
}

KonnectorProfile KonnectorProfileManager::find( const Device& dev )
{
    KonnectorProfile prof;
    KonnectorProfile::ValueList::Iterator it;
    for ( it = m_list.begin(); it != m_list.end(); ++it  ) {
        if ( (*it).device() == dev ) {
            prof = (*it);
            break;
        }

    }
    return prof;
}

KonnectorProfile::ValueList KonnectorProfileManager::list() const
{
    return m_list;
}

void KonnectorProfileManager::setList( const KonnectorProfile::ValueList& list )
{
    m_list= list;

    if (list.count() > 0 ) // take the first as current
        m_current = list[0];
    else
        m_current = KonnectorProfile();

}

KonnectorProfile KonnectorProfileManager::current() const
{
    return m_current;
}

void KonnectorProfileManager::setCurrent( const KonnectorProfile &cur )
{
    m_current = cur;
}

void KonnectorProfileManager::add( const KonnectorProfile &prof )
{
    m_list.remove( prof );
    m_list.append( prof );
}

void KonnectorProfileManager::replace( const KonnectorProfile &prof )
{
    m_list.remove( prof );
    m_list.append( prof );
}

void KonnectorProfileManager::remove ( const KonnectorProfile &prof )
{
    m_list.remove( prof );
}

void KonnectorProfileManager::clear()
{
    m_list.clear();
    m_current = KonnectorProfile();
}

void KonnectorProfileManager::load()
{
    kdDebug(5210) << "KonnectorProfileManager::load() " << endl;
    KonnectorProfileFileManager man;
    setList( man.load() );
}

void KonnectorProfileManager::save()
{
    KonnectorProfileFileManager man;
    man.save( m_list );
}

uint KonnectorProfileManager::count() const
{
    return m_list.count();
}

KonnectorProfile KonnectorProfileManager::profile(uint index ) const
{
    if ( index >= m_list.count() ) {
        KonnectorProfile prof;
        return prof;
    }

    return m_list[index];
}
