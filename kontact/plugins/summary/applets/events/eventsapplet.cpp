/*
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
 * 
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "eventsapplet.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include <akonadi/collection.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>

K_EXPORT_PLASMA_APPLET(events, EventsApplet)

EventsApplet::EventsApplet( QObject* parent, QVariantList args )
{
}

void EventsApplet::init()
{
    m_collectionJobsInProgress = 0;

    configChanged();
}

void EventsApplet::configChanged()
{
    KConfigGroup cg = config();

    // This can be either "Todo", "Events" or "Agenda" (merged of both)
    m_incidenceType = cg.readEntry( "incidenceType", "Agenda" );
    m_numDays = cg.readEntry( "numDays", 7 );

    m_collectionIds = cg.readEntry( "collections", QStringList() );

    // Now find and load the collections we need
    Akonadi::Collection::List collections;

    if ( m_collectionIds.length() > 0 ) {
        m_noCollections = false;
    } else {
        m_noCollections = true;
        getCollectionFromKorg();
    }

    foreach( QString Id, m_collectionIds ) {
        Akonadi::Collection collection = Akonadi::Collection( Id.toInt() );
        collections << collection;
    }

    Akonadi::CollectionFetchJob* job = new Akonadi::CollectionFetchJob( collections, this );
    connect( job, SIGNAL( collectionsReceived(  const Akonadi::Collection::List& ) ),
             this, SLOT(  collectionsReceived(  const Akonadi::Collection::List& ) ) );
    connect( job, SIGNAL( result(  KJob* ) ), this, SLOT(  collectionFetchResult(  KJob* ) ) );

    m_collectionJobsInProgress++;
    setBusy( m_collectionJobsInProgress );
}

void EventsApplet::getCollectionFromKorg()
{
    QString korgrc = KStandardDirs::locate( "config", "korganizerrc" );

    if ( korgrc.length() > 0 ) {
        KSharedConfigPtr config = KSharedConfig::openConfig( korgrc );
        KConfigGroup cg = config->group( "GlobalCollectionSelection" );

        QString collections = cg.readEntry( "Role_CheckState" );

        // TODO: Any better way to parse this??
        QStringList splitCollections = collections.split( "," );
        for ( int i = 0; i < splitCollections.length(); i += 2 ) {
            // So splitCollections[i] will have the collection and splitCollections[i+1] will
            // have its state. 0 is disabled, 2(?) is enabled. 1 causes the LHC to destroy the
            // universe?

            if ( splitCollections[ i+1 ] == "2" ) {
                QString nonParsed = splitCollections[ i ];
                QString collectionId = nonParsed.right( nonParsed.length()-1 );
                kDebug() << collectionId << "is enabled";
                m_collectionIds << collectionId;
            }
        }
    }
}

void EventsApplet::collectionsReceived(  const Akonadi::Collection::List& collections )
{
    kDebug()<< "collections received";
    m_collections << collections;
}

void EventsApplet::collectionFetchResult( KJob* job )
{
    m_collectionJobsInProgress--;
    setBusy( m_collectionJobsInProgress );

    kDebug() << job;

    if ( m_collectionJobsInProgress == 0 ) {
        //events();
        kDebug() << m_collections;
    }
}
