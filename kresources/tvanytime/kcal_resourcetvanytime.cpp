/*
    This file is part of kdepim.

    Copyright (c) 2005 Will Stephenson <wstephenson@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kcal_resourcetvanytime.h"

#include "kcal_tvanytimeprefsbase.h"
#include "kcal_resourcetvanytimeconfig.h"

#include <kcal/icalformat.h>
#include <kcal/calendarlocal.h>
#include <kcal/confirmsavedialog.h>

#include <libkdepim/kpimprefs.h>

#include <QApplication>
#include <qdom.h>
#include <QDateTime>
#include <QRegExp>
#include <QStringList>
#include <QTimer>

#include <kabc/locknull.h>
#include <karchive.h>
#include <kdebug.h>
#include <klocale.h>
#include <kresources/configwidget.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <ksystemtimezone.h>
#include <ktemporaryfile.h>
#include <ktar.h>
#include <kurl.h>

using namespace KCal;

ResourceTVAnytime::ResourceTVAnytime()
  : ResourceCached(), mLock( true )
{
  init();

  mPrefs->addGroupPrefix( identifier() );
}

ResourceTVAnytime::ResourceTVAnytime( const KConfigGroup &group )
  : ResourceCached( group ), mLock( true )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  readConfig( group );
}

ResourceTVAnytime::~ResourceTVAnytime()
{
  disableChangeNotification();

  delete mPrefs;
  mPrefs = 0;
}

void ResourceTVAnytime::init()
{
  mDownloadJob = 0;
  mProgress = 0;
  mDestination = 0;

  mIsShowingError = false;

  mPrefs = new TVAnytimePrefsBase();

  setType( "tvanytime" );

  enableChangeNotification();

  //setReadOnly( true );

  connect( &mResourceChangedTimer, SIGNAL( timeout() ),
           this, SLOT( slotEmitResourceChanged() ) );
}

TVAnytimePrefsBase *ResourceTVAnytime::prefs()
{
  return mPrefs;
}

void ResourceTVAnytime::readConfig( const KConfigGroup &group )
{
  kDebug() <<"KCal::ResourceTVAnytime::readConfig()";

  mPrefs->readConfig();

  ResourceCached::readConfig( group );

  QStringList defaultActive;
  defaultActive << "BBCOne" << "BBCTwo" << "BBCThree" << "BBCROne" << "BBCRTwo" << "BBCRThree" << "BBCRFour";
  mActiveServices = group.readEntry( "ActiveServices", defaultActive , QStringList() );
  
}

void ResourceTVAnytime::writeConfig( KConfigGroup &group )
{
  kDebug() <<"KCal::ResourceTVAnytime::writeConfig()";

  ResourceCalendar::writeConfig( group );

  mPrefs->writeConfig();

  ResourceCached::writeConfig( group );
  QStringList activeServices;
  ServiceMap::ConstIterator it;
  for ( it = mServiceMap.begin(); it != mServiceMap.end(); ++it )
    if ( it.value().active() )
      activeServices.append( it.key() );
  group.writeEntry( "ActiveServices", activeServices );
}

bool ResourceTVAnytime::doOpen()
{
  return true;
}

void ResourceTVAnytime::doClose()
{
  ResourceCached::doClose();
}

bool ResourceTVAnytime::doLoad( bool )
{
  kDebug() <<"ResourceTVAnytime::load()";

  if ( mIsShowingError ) {
    kDebug() <<"Still showing error";
    return true;
  }

  if ( mDownloadJob ) {
    kWarning() <<"Download still in progress";
    return false;
  }

  mUidMap.clear();
  calendar()->close();

  disableChangeNotification();
  // FIXME - reenable cache
  loadFromCache(); // TODO: default loadFromCache() won't set up the subresources...
  enableChangeNotification();

  emit resourceChanged( this );

  clearChanges();

  mDestination = new KTemporaryFile;
  mDestination->setAutoRemove(false);
  mDestination->open();
  KUrl url( prefs()->url() );

  KUrl destination = KUrl( mDestination->fileName() );

  kDebug(5850) <<"  SOURCE:" << url.url();
  kDebug(5850) <<"  DESTINATION:" << destination.url();

  // TODO: find out if the file to download is fresh.  if not, just work with the cache.
  mDownloadJob = KIO::file_copy( url, destination, -1, KIO::Overwrite );
  connect( mDownloadJob, SIGNAL( result( KJob * ) ),
           SLOT( slotJobResult( KJob * ) ) );

  mProgress = KPIM::ProgressManager::instance()->createProgressItem(
    KPIM::ProgressManager::getUniqueID(), i18n("Downloading program schedule") );

  connect( mProgress,
           SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           SLOT( cancelLoad() ) );

  return true;
}

void ResourceTVAnytime::slotJobResult( KJob *job )
{
  kDebug() <<"ResourceTVAnytime::slotJobResult():";

  if ( job->error() ) {
    mIsShowingError = true;
    loadError( job->errorString() );
    mIsShowingError = false;
    delete mDestination;
    mDestination = 0;
  } else {
    disableChangeNotification();

    clearCache();

    // FIXME: This does not take into account the time zone!

    readSchedule();

    // FIXME - reenable cache
    saveToCache();
    enableChangeNotification();

    clearChanges();

    emit resourceChanged( this );
    emit resourceLoaded( this );
  }

  mDownloadJob = 0;
  if ( mProgress ) mProgress->setComplete();
  mProgress = 0;
  delete mDestination; 
  mDestination = 0;
}

QDomDocument ResourceTVAnytime::archiveFileXml( const QString & name )
{
  QDomDocument result;
  const KArchiveDirectory *dir = mScheduleArchive->directory();
  const KArchiveEntry * entry = dir->entry( name );
  if ( entry && entry->isFile() )
  {
    const KArchiveFile * file = static_cast<const KArchiveFile *>( entry );
    result.setContent( file->data() );
  }
  return result;
}

bool ResourceTVAnytime::readSchedule()
{
  QString uncompress = "application/x-gzip";
  mScheduleArchive = new KTar( mDestination->fileName(), uncompress );
  mScheduleArchive->open( QIODevice::ReadOnly );

  QDomDocument serviceInfo = archiveFileXml( "ServiceInformation.xml" );
  if ( !serviceInfo.isNull() )
    readServiceInformation( serviceInfo );


  ServiceMap::Iterator it;
  // for each date in the tarball/or user preference:
#if 1
  for ( it = mServiceMap.begin(); it != mServiceMap.end(); ++it )
  {
    if ( (it.value().active() ) )
      readService( it.key() );
  }
#else
  QString serviceId = "BBCOne";
  Service s = mServiceMap[ serviceId ];
  if ( s.active() )
    readService( serviceId );
#endif

  mScheduleArchive->close();
  return true;
}

bool ResourceTVAnytime::readServiceInformation( const QDomDocument & serviceInfo )
{
  kDebug() ;
  QDomElement docElem = serviceInfo.documentElement();

  QDomNode n = docElem.firstChild();
  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if( !e.isNull() && e.tagName() == "ProgramDescription" ) {
      QDomNode n2 =  e.firstChild();
      QDomElement e2 = n2.toElement();
      if( !e2.isNull() && e2.tagName() == "ServiceInformationTable" ) {
        QDomNode n3 =  e2.firstChild();
        while ( !n3.isNull() ) {
          Service s;
          QDomElement e3 = n3.toElement();
          if (s.loadXML( e3 ) ) {
            s.setActive( mActiveServices.contains( s.id() ) );
            bool newService = !mServiceMap.contains( s.id() );   
            if ( newService )
            {
              mServiceMap.insert( s.id(), s );
              emit signalSubresourceAdded( this, "Calendar", s.id(), s.name() );
            }
          }
          else 
            kDebug() <<" couldn't find ServiceInformation:" << e3.tagName();
          n3 = n3.nextSibling();
        }
      }
      else 
        kDebug() <<" couldn't find ServiceInformationTable:" << e2.tagName();
    }
    else 
      kDebug() <<" couldn't find ProgramDescription:" << e.tagName();
    n = n.nextSibling();
  }
  return true;
}

bool ResourceTVAnytime::readService( const QString & serviceId )
{
  kDebug() ;
  // open program information table
  Service service = mServiceMap[ serviceId ];

  QStringList entries = mScheduleArchive->directory()->entries();
  QRegExp re( "^(\\d{8})" + serviceId );
  QStringList dates;
  QString todaysDate = QDate::currentDate().toString( "yyyyMMdd" );
  for( QStringList::Iterator it = entries.begin(); it != entries.end(); ++it )
  {
    if ( re.search( *it ) != -1 ) // this entry belongs to the requested service
    {
      QString entry = re.cap( 1 );
      // handle this date according to user preferences
      QDate entryDate( entry.left( 4 ).toInt(), entry.mid( 4, 2 ).toInt(), entry.right( 2 ).toInt() );
      if ( entryDate < QDate::currentDate() || ( entryDate > QDate::currentDate().addDays( prefs()->days() - 1 ) ) )
        continue;

      if ( !dates.contains( re.cap( 1 ) ) )
        dates.append( re.cap( 1 ) );
    }
  }

  kDebug() <<"reading schedule for" << serviceId <<" on" << dates;

  KTimeZone london = KSystemTimeZones::zone( "Europe/London" );

  for( QStringList::Iterator it = dates.begin(); it != dates.end(); ++it )
  {
    ProgramInformationMap progInfoMap = service.programmeInformation();
    QString programInfoFileName = QString( *it + serviceId + "_pi.xml" );
    QDomDocument programInfo = archiveFileXml( programInfoFileName );
    if ( !programInfo.isNull() ) {
      QDomElement docElem = programInfo.documentElement();
      QDomNode n = docElem.firstChild();
      while( !n.isNull() ) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if( !e.isNull() && e.tagName() == "ProgramDescription" ) {
          QDomNode n2 =  e.firstChild();
          QDomElement e2 = n2.toElement();
          if( !e2.isNull() && e2.tagName() == "ProgramInformationTable" ) {
            QDomNode n3 =  e2.firstChild();
            while ( !n3.isNull() ) {
              QDomElement e3 = n3.toElement();
              ProgramInformation pi; 
              if ( pi.loadXML( e3 ) ) {
                //kDebug() <<"Found program:" << pi.id() <<"," << pi.title() <<"," << pi.synopsis();
                progInfoMap.insert( pi.id(), pi );
              }
              n3 = n3.nextSibling();
            }
          }
        }
        n = n.nextSibling();
      }
      service.setProgramInformation( progInfoMap );
    }
    else
      kDebug() <<"Service file:" << programInfoFileName <<" not found in archive";
    // open program location table, iterate and create incidences
  
    QString programLocationFileName = QString( *it + serviceId + "_pl.xml" );
    QDomDocument programLocation = archiveFileXml( programLocationFileName );
    if ( !programLocation.isNull() ) {
      QDomElement docElem = programLocation.documentElement();
      QDomNode n = docElem.firstChild();
      while( !n.isNull() ) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if( !e.isNull() && e.tagName() == "ProgramDescription" ) {
          QDomNode n2 =  e.firstChild();
          QDomElement e2 = n2.toElement();
          if( !e2.isNull() && e2.tagName() == "ProgramLocationTable" ) {
            QDomNode n3 = e2.firstChild();
            QDomElement e3 = n3.toElement();
            if( !e3.isNull() && e3.tagName() == "Schedule" ) {
              QString foundServiceId = e3.attribute( "serviceIDRef" );
              if ( serviceId == foundServiceId ) {
                QDomNode n4 = e3.firstChild();
                while ( !n4.isNull() ) {
                  QDomElement e4 = n4.toElement();
                  ScheduleEvent se;
                  if ( se.loadXML( e4 ) ) {
                    ProgramInformation pi = progInfoMap[ se.crid() ];
                    //kDebug() <<"program incidence:" << se.crid()  <<"," << se.startTime() <<"," << se.duration() <<"," << pi.title() <<"," << pi.synopsis();
                    KCal::Event *event = new KCal::Event();
                    event->setFloats( false );
                    event->setSummary( pi.title() );
                    event->setDescription( pi.synopsis() );
                    QDateTime dt = se.startTime();
                    dt.setTimeSpec( Qt::LocalTime );
                    event->setDtStart( KDateTime( dt, london ).toUtc().dateTime() );
                    event->setDuration( se.duration() );
                    event->setLocation( service.name() );
                    event->setCategories( pi.genres() );
                    event->setUid( se.programUrl() );
                    event->setCustomProperty( "TVANYWHERE", "SERVICEID", serviceId );
                    // store the reverse mapping from event to service, for subresources
                    mUidMap.insert( event->uid(), serviceId );
                    event->setReadOnly( true );
  
                    addIncidence( event );
                  }
                  n4 = n4.nextSibling();
                }
              }
              else
                kDebug() <<" file contains schedule for another service!";
            }
          }
        }
        n = n.nextSibling();
      }
    }
    else
      kDebug() <<"Program location file:" << programLocationFileName <<" not found in archive";
  }
  return true;
}

QStringList ResourceTVAnytime::subresources() const
{
  //const_cast<ResourceTVAnytime*>( this )->doLoad();
  return mServiceMap.keys();
}

const QString
ResourceTVAnytime::labelForSubresource( const QString& subresource ) const
{
  Service s = mServiceMap[ subresource ];
  return s.name();
}

QString ResourceTVAnytime::subresourceIdentifier( Incidence *incidence )
{
  return incidence->customProperty( "TVANYWHERE", "SERVICEID" );
}

bool ResourceTVAnytime::subresourceActive( const QString & subresource ) const
{
  Service s = mServiceMap[ subresource ];
  return s.active();
}

void ResourceTVAnytime::setSubresourceActive( const QString & subresource, bool active )
{
  if ( mServiceMap.contains( subresource ) )
  {
    Service s = mServiceMap[ subresource ];
    if ( s.active() != active )
    {
      s.setActive( active );
      mServiceMap.insert( subresource, s );
      doLoad( true );
      mResourceChangedTimer.start( 150 );
    }
  }
}

KABC::Lock *ResourceTVAnytime::lock()
{
  return &mLock;
}

void ResourceTVAnytime::cancelLoad()
{
  if ( mDownloadJob ) mDownloadJob->kill();
  mDownloadJob = 0;
  if ( mProgress ) mProgress->setComplete();
  mProgress = 0;
}

void ResourceTVAnytime::slotEmitResourceChanged()
{
  emit resourceChanged( this );
  mResourceChangedTimer.stop();
}

#include "kcal_resourcetvanytime.moc"
