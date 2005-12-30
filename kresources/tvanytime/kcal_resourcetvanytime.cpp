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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kcal_resourcetvanytime.h"

#include "kcal_tvanytimeprefsbase.h"
#include "kcal_resourcetvanytimeconfig.h"

#include <libkcal/icalformat.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/confirmsavedialog.h>

#include <libkdepim/kpimprefs.h>

#include <qapplication.h>
#include <qdom.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qtimer.h>

#include <kabc/locknull.h>
#include <karchive.h>
#include <kdebug.h>
#include <klocale.h>
#include <kresources/configwidget.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <ktempfile.h>
#include <ktar.h>
#include <kurl.h>

using namespace KCal;

ResourceTVAnytime::ResourceTVAnytime()
  : ResourceCached( 0 ), mLock( true )
{
  init();

  mPrefs->addGroupPrefix( identifier() );
}

ResourceTVAnytime::ResourceTVAnytime( const KConfig *config )
  : ResourceCached( config ), mLock( true )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  if ( config ) readConfig( config );
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

void ResourceTVAnytime::readConfig( const KConfig *config )
{
  kdDebug() << "KCal::ResourceTVAnytime::readConfig()" << endl;

  mPrefs->readConfig();

  ResourceCached::readConfig( config );

  QStringList defaultActive;
  defaultActive << "BBCOne" << "BBCTwo" << "BBCThree" << "BBCROne" << "BBCRTwo" << "BBCRThree" << "BBCRFour";
  mActiveServices = config->readListEntry( "ActiveServices", defaultActive );
  
}

void ResourceTVAnytime::writeConfig( KConfig *config )
{
  kdDebug() << "KCal::ResourceTVAnytime::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );

  mPrefs->writeConfig();

  ResourceCached::writeConfig( config );
  QStringList activeServices;
  ServiceMap::ConstIterator it;
  for ( it = mServiceMap.begin(); it != mServiceMap.end(); ++it )
    if ( it.data().active() )
      activeServices.append( it.key() );
  config->writeEntry( "ActiveServices", activeServices );
}

bool ResourceTVAnytime::doOpen()
{
  return true;
}

void ResourceTVAnytime::doClose()
{
  ResourceCached::doClose();
}

bool ResourceTVAnytime::doLoad()
{
  kdDebug() << "ResourceTVAnytime::load()" << endl;

  if ( mIsShowingError ) {
    kdDebug() << "Still showing error" << endl;
    return true;
  }

  if ( mDownloadJob ) {
    kdWarning() << "Download still in progress" << endl;
    return false;
  }

  mUidMap.clear();
  mCalendar.close();

  disableChangeNotification();
  // FIXME - reenable cache
  loadCache(); // TODO: default loadCache() won't set up the subresources...
  enableChangeNotification();

  emit resourceChanged( this );

  clearChanges();

  mDestination = new KTempFile;
  KURL url( prefs()->url() );

  KURL destination = KURL( mDestination->name() );

  kdDebug(5850) << "  SOURCE: " << url.url() << endl;
  kdDebug(5850) << "  DESTINATION: " << destination.url() << endl;

  // TODO: find out if the file to download is fresh.  if not, just work with the cache.
  mDownloadJob = KIO::file_copy( url, destination, -1, true );
  connect( mDownloadJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotJobResult( KIO::Job * ) ) );

  mProgress = KPIM::ProgressManager::instance()->createProgressItem(
    KPIM::ProgressManager::getUniqueID(), i18n("Downloading program schedule") );

  connect( mProgress,
           SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           SLOT( cancelLoad() ) );

  return true;
}

void ResourceTVAnytime::slotJobResult( KIO::Job *job )
{
  kdDebug() << "ResourceTVAnytime::slotJobResult(): " << endl;

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
    saveCache();
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
  mScheduleArchive = new KTar( mDestination->name(), uncompress );
  mScheduleArchive->open( IO_ReadOnly );

  QDomDocument serviceInfo = archiveFileXml( "ServiceInformation.xml" );
  if ( !serviceInfo.isNull() )
    readServiceInformation( serviceInfo );


  ServiceMap::Iterator it;
  // for each date in the tarball/or user preference:
#if 1
  for ( it = mServiceMap.begin(); it != mServiceMap.end(); ++it )
  {
    if ( (it.data().active() ) )
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
  kdDebug() << k_funcinfo << endl;
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
            kdDebug() << " couldn't find ServiceInformation: " << e3.tagName() << endl;
          n3 = n3.nextSibling();
        }
      }
      else 
        kdDebug() << " couldn't find ServiceInformationTable: " << e2.tagName() << endl;
    }
    else 
      kdDebug() << " couldn't find ProgramDescription: " << e.tagName() << endl;
    n = n.nextSibling();
  }
  return true;
}

bool ResourceTVAnytime::readService( const QString & serviceId )
{
  kdDebug() << k_funcinfo << endl;
  // open programme information table
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

  kdDebug() << "reading schedule for " << serviceId << " on " << dates << endl;

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
                //kdDebug() << "Found programme: " << pi.id() << "," << pi.title() << "," << pi.synopsis() << endl;
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
      kdDebug() << "Service file: " << programInfoFileName << " not found in archive" << endl;
    // open programme location table, iterate and create incidences
  
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
                    //kdDebug() << "programme incidence: " << se.crid()  << "," << se.startTime() << "," << se.duration() << "," << pi.title() << "," << pi.synopsis() << endl;
                    KCal::Event *event = new KCal::Event();
                    event->setFloats( false );
                    event->setSummary( pi.title() );
                    event->setDescription( pi.synopsis() );
                    event->setDtStart( KPimPrefs::localTimeToUtc( se.startTime(), "Europe/London" ) );
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
                kdDebug() << " file contains schedule for another service!" << endl;
            }
          }
        }
        n = n.nextSibling();
      }
    }
    else
      kdDebug() << "Program location file: " << programLocationFileName << " not found in archive" << endl;
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
      doLoad();
      mResourceChangedTimer.changeInterval( 150 );
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
