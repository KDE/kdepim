/*
    This file is part of libkcal.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <typeinfo>
#include <stdlib.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qptrlist.h>

#include <kdebug.h>
#include <kurl.h>
#include <kio/job.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "vcaldrag.h"
#include "vcalformat.h"
#include "icalformat.h"
#include "exceptions.h"
#include "incidence.h"
#include "event.h"
#include "todo.h"
#include "journal.h"
#include "filestorage.h"

#include <kresources/configwidget.h>

#include "resourceremoteconfig.h"

#include "resourceremote.h"

using namespace KCal;

class RemoteFactory : public KRES::PluginFactory
{
  public:
    KRES::Resource *resource( const KConfig *config )
    {
      return new ResourceRemote( config );
    }

    KRES::ConfigWidget *configWidget( QWidget *parent )
    {
      return new ResourceRemoteConfig( parent, "ResourceRemoteConfig" );
    }
};

extern "C"
{
  void *init_kcal_remote()
  {
    return ( new RemoteFactory() );
  }
}


ResourceRemote::ResourceRemote( const KConfig* config )
  : ResourceCalendar( config )
{
  if ( config ) {
    readConfig( config );
  }

  init();
}

ResourceRemote::ResourceRemote( const KURL &downloadUrl, const KURL &uploadUrl )
  : ResourceCalendar( 0 )
{
  mDownloadUrl = downloadUrl;
  
  if ( uploadUrl.isEmpty() ) {
    mUploadUrl = mDownloadUrl;
  } else {
    mUploadUrl = uploadUrl;
  }

  init();
}

ResourceRemote::~ResourceRemote()
{
  close();

  if ( mDownloadJob ) mDownloadJob->kill();
  if ( mUploadJob ) mUploadJob->kill();
}

void ResourceRemote::init()
{
  mDownloadJob = 0;
  mUploadJob = 0;

  setType( "remote" );

  mOpen = false;
}

void ResourceRemote::readConfig( const KConfig *config )
{
  QString url = config->readEntry( "DownloadUrl" );
  mDownloadUrl = KURL( url );

  url = config->readEntry( "UploadUrl" );
  mUploadUrl = KURL( url );
}

void ResourceRemote::writeConfig( KConfig *config )
{
  kdDebug() << "ResourceRemote::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );

  config->writeEntry( "DownloadUrl", mDownloadUrl.url() );
  config->writeEntry( "UploadUrl", mUploadUrl.url() );
}

void ResourceRemote::setDownloadUrl( const KURL &url )
{
  mDownloadUrl = url;
}

KURL ResourceRemote::downloadUrl() const
{
  return mDownloadUrl;
}

void ResourceRemote::setUploadUrl( const KURL &url )
{
  mUploadUrl = url;
}

KURL ResourceRemote::uploadUrl() const
{
  return mUploadUrl;
}

QString ResourceRemote::cacheFile()
{
  QString file = locateLocal( "cache", "kcal/kresources/" + identifier() );
  kdDebug() << "ResourceRemote::cacheFile(): " << file;
  return file;
}

bool ResourceRemote::doOpen()
{
  kdDebug(5800) << "ResourceRemote::doOpen()" << endl;

  mOpen = true;

  return true;
}

bool ResourceRemote::load()
{
  kdDebug() << "ResourceRemote::load()" << endl;

  if ( !mOpen ) return true;

  if ( mDownloadJob ) {
    kdWarning() << "ResourceRemote::load(): download still in progress."
                << endl;
    return false;
  }
  if ( mUploadJob ) {
    kdWarning() << "ResourceRemote::load(): upload still in progress."
                << endl;
    return false;
  }

  mCalendar.close();

  mCalendar.load( cacheFile() );

  mDownloadJob = KIO::file_copy( mDownloadUrl, KURL( cacheFile() ), -1, true );
  connect( mDownloadJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotLoadJobResult( KIO::Job * ) ) );

  return true;
}

void ResourceRemote::slotLoadJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0 );
  } else {
    kdDebug() << "ResourceRemote::slotLoadJobResult() success" << endl;

    mCalendar.close();
    mCalendar.load( cacheFile() );

    emit resourceChanged( this );
  }

  mDownloadJob = 0;

  emit resourceLoaded( this );
}

bool ResourceRemote::save()
{
  kdDebug() << "ResourceRemote::save()" << endl;

  if ( !mOpen ) return true;

  if ( readOnly() ) {
    emit resourceSaved( this );
    return true;
  }

  if ( mDownloadJob ) {
    kdWarning() << "ResourceRemote::save(): download still in progress."
                << endl;
    return false;
  }
  if ( mUploadJob ) {
    kdWarning() << "ResourceRemote::save(): upload still in progress."
                << endl;
    return false;
  }

  mCalendar.save( cacheFile() );

  mUploadJob = KIO::file_copy( KURL( cacheFile() ), mUploadUrl, -1, true );
  connect( mUploadJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotSaveJobResult( KIO::Job * ) ) );

  return true;
}

bool ResourceRemote::isSaving()
{
  return mUploadJob;
}

void ResourceRemote::slotSaveJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0 );
  } else {
    kdDebug() << "ResourceRemote::slotSaveJobResult() success" << endl;
  }
  
  mUploadJob = 0;

  emit resourceSaved( this );
}

void ResourceRemote::doClose()
{
  if ( !mOpen ) return;

  mCalendar.close();
  mOpen = false;
}


bool ResourceRemote::addEvent(Event *event)
{
  return mCalendar.addEvent( event );
}

void ResourceRemote::deleteEvent(Event *event)
{
  kdDebug(5800) << "ResourceRemote::deleteEvent" << endl;

  mCalendar.deleteEvent( event );
}


Event *ResourceRemote::event( const QString &uid )
{
  return mCalendar.event( uid );
}

QPtrList<Event> ResourceRemote::rawEventsForDate(const QDate &qd, bool sorted)
{
  return mCalendar.rawEventsForDate( qd, sorted );
}


QPtrList<Event> ResourceRemote::rawEvents( const QDate &start, const QDate &end,
                                          bool inclusive )
{
  return mCalendar.rawEvents( start, end, inclusive );
}

QPtrList<Event> ResourceRemote::rawEventsForDate(const QDateTime &qdt)
{
  return mCalendar.rawEventsForDate( qdt.date() );
}

QPtrList<Event> ResourceRemote::rawEvents()
{
  return mCalendar.rawEvents();
}

bool ResourceRemote::addTodo(Todo *todo)
{
  return mCalendar.addTodo( todo );
}

void ResourceRemote::deleteTodo(Todo *todo)
{
  mCalendar.deleteTodo( todo );
}


QPtrList<Todo> ResourceRemote::rawTodos()
{
  return mCalendar.rawTodos();
}

Todo *ResourceRemote::todo( const QString &uid )
{
  return mCalendar.todo( uid );
}

QPtrList<Todo> ResourceRemote::todos( const QDate &date )
{
  return mCalendar.todos( date );
}


bool ResourceRemote::addJournal(Journal *journal)
{
  kdDebug(5800) << "Adding Journal on " << journal->dtStart().toString() << endl;

  return mCalendar.addJournal( journal );
}

Journal *ResourceRemote::journal(const QDate &date)
{
//  kdDebug(5800) << "ResourceRemote::journal() " << date.toString() << endl;

  return mCalendar.journal( date );
}

Journal *ResourceRemote::journal(const QString &uid)
{
  return mCalendar.journal( uid );
}

QPtrList<Journal> ResourceRemote::journals()
{
  return mCalendar.journals();
}


Alarm::List ResourceRemote::alarmsTo( const QDateTime &to )
{
  return mCalendar.alarmsTo( to );
}

Alarm::List ResourceRemote::alarms( const QDateTime &from, const QDateTime &to )
{
//  kdDebug(5800) << "ResourceRemote::alarms(" << from.toString() << " - " << to.toString() << ")\n";

  return mCalendar.alarms( from, to );
}

void ResourceRemote::update(IncidenceBase *)
{
}

void ResourceRemote::dump() const
{
  ResourceCalendar::dump();
  kdDebug(5800) << "  DownloadUrl: " << mDownloadUrl.url() << endl;
  kdDebug(5800) << "  UploadUrl: " << mUploadUrl.url() << endl;
}

#include "resourceremote.moc"
