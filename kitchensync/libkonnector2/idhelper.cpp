/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <freyther@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kconfig.h>
#include <kdebug.h>

#include "idhelper.h"

using namespace KSync;

// TypeORAppName||%%||KonnectorId||%%||KDEID
KonnectorUIDHelper::KonnectorUIDHelper( const QString &dir )
{
//    kdDebug(5201) << "new KonnectorUIDHelper " << dir <<endl;
    m_config = new KConfig( dir + "/konnector-ids.conf");
    m_config->setGroup("uids");
    QString string = m_config->readEntry( "ids" );
    QStringList list = QStringList::split( "%%||%%",  string );

    for ( QStringList::Iterator it = list.begin(); it != list.end() ; ++it ) {
        QStringList list2 = QStringList::split("||%%||",(*it), true ); // allow empty entries
        addId( list2[0],  list2[1], list2[2] );
    }

}
KonnectorUIDHelper::~KonnectorUIDHelper()
{
    save();
    delete m_config;
}
QString KonnectorUIDHelper::konnectorId( const QString &appName,  const QString &kdeId, const QString &defaultId )
{
//    kdDebug(5201) << "IdHelper: KonnectorIdAppName: "
//                  << appName << " KDE Id: "
//                  << kdeId << " defaultId "
//                  << defaultId << endl;

    QMap<QString,  Kontainer::ValueList >::Iterator it;
    it = m_ids.find( appName );
    if ( it != m_ids.end() ) {
        Kontainer::ValueList kontainer = it.data();
        Kontainer::ValueList::Iterator it;
        for ( it = kontainer.begin(); it != kontainer.end(); ++it ) {
            if ( kdeId.stripWhiteSpace() == (*it).second.stripWhiteSpace() ) {
//                kdDebug(5201) << "it.first = " << (*it).first() << endl;
                return (*it).first;
            }
        }
    }
    return defaultId;
}
QString KonnectorUIDHelper::kdeId( const QString &appName,  const QString &konnectorId, const QString &defaultId )
{
//    kdDebug(5201) << "kdeId: AppName: "
//                  << appName  << " konnectorId "
//                  << konnectorId << endl;

    QMap<QString,  Kontainer::ValueList >::Iterator it;
    it = m_ids.find( appName );
    if ( it != m_ids.end() ) {
        Kontainer::ValueList kontainer = it.data();
        Kontainer::ValueList::Iterator it;
        for ( it = kontainer.begin(); it != kontainer.end(); ++it ) {
            if ( konnectorId.stripWhiteSpace() == (*it).first.stripWhiteSpace() ) {
//                kdDebug(5201) << "it.second " << (*it).second() << endl;
                return (*it).second;
            }
        }
    }
    return defaultId;
}
void KonnectorUIDHelper::addId( const QString& appName,
                                const QString& konnectorId,
                                const QString& kdeId )
{
//    kdDebug(5201) << "addId " << appName
//                  << "  konId "  << konnectorId
//                  << " kdeId " << kdeId << endl;

    QMap<QString,  Kontainer::ValueList >::Iterator it;
    it = m_ids.find( appName );

    if ( it == m_ids.end() ) {
//        kdDebug(5201) << "First insert" << endl;
        Kontainer::ValueList kontainer;
        kontainer.append( Kontainer( konnectorId,  kdeId ) );
        m_ids.replace( appName,  kontainer );
    }else{
//        kdDebug(5201) << "Already inserted" << endl;
        Kontainer::ValueList &kontainer = it.data();
        Kontainer kont( konnectorId,  kdeId );
        kontainer.remove( kont );
        kontainer.append( kont );
    }
}
void KonnectorUIDHelper::removeId( const QString &appName,  const QString &id )
{
    QMap<QString,  Kontainer::ValueList >::Iterator it;
    it = m_ids.find( appName );
    if ( it != m_ids.end() ) {
        Kontainer::ValueList &kontainer = it.data();
        Kontainer::ValueList::Iterator it;
        for ( it = kontainer.begin(); it != kontainer.end(); ++it ) {
            if ( (*it).first == id || (*it).second == id ) {
                it  = kontainer.remove( it );
                return;
            }
        }
    }
}
void KonnectorUIDHelper::replaceIds( const QString &app,
                                     Kontainer::ValueList ids )
{
    m_ids.replace( app,  ids );
}
void KonnectorUIDHelper::clear()
{
    m_ids.clear();
    save();
}
void KonnectorUIDHelper::save()
{
    QString string;
    QMap<QString,  Kontainer::ValueList >::Iterator mapIt;
    Kontainer::ValueList::Iterator kontainerIt;
    for ( mapIt = m_ids.begin(); mapIt != m_ids.end(); ++mapIt ) {
        for ( kontainerIt = mapIt.data().begin();
              kontainerIt != mapIt.data().end();
              ++kontainerIt ) {

            /*  AppName||%%||KonnectorId||%%||KDEID%%||%%AppName||%%||KonnectorId||%%||KDEID */
            //kdDebug() << mapIt.key() << " "
            //          << (*kontainerIt).first()
            //          << " " << (*kontainerIt).second() << endl;

            string.append(mapIt.key()+ "||%%||"
                          + (*kontainerIt).first +
                          "||%%||" + (*kontainerIt).second+ "%%||%%");
        }
    }
    m_config->writeEntry( "ids",  string );
    m_config->sync();
}
