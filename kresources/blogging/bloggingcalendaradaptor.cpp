/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>

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

#include "bloggingcalendaradaptor.h"
#include "bloggingglobals.h"
#include <libemailfunctions/idmapper.h>
#include <folderlister.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>
#include <libkcal/resourcecached.h>

#include <kdebug.h>

using namespace KCal;

// That terribly long app key was generated at
// http://www.blogger.com/developers/api/1_docs/register.html
// for the "KDE-Pim libkcal blogging resource".
// TODO:
/*QString BloggingCalendarAdaptor::mAppID =
    QString("20ffffffd7ffffffc5ffffffbdffffff87ffffffb72d39fffffffe5c4bfffff"
            "fcfffffff80ffffffd4665cfffffff375ffffff88ffffff871a0cffffff8029");
*/


BloggingCalendarAdaptor::BloggingCalendarAdaptor() : mAPI( 0 ), mAuthenticated( false )
{
}


KBlog::APIBlog *BloggingCalendarAdaptor::api() const
{
  return mAPI;
}

void BloggingCalendarAdaptor::setAPI( KBlog::APIBlog *api )
{
  delete mAPI;
  mAPI = api;
  mAuthenticated = false;
  connect( api, SIGNAL( userInfoRetrieved( const QString &, const QString &,
                                           const QString & ) ),
           SLOT( slotUserInfoRetrieved( const QString &, const QString &,
                                    const QString & ) ) );
  connect( api, SIGNAL( folderInfoRetrieved( const QString &, const QString & ) ),
           SLOT( slotFolderInfoRetrieved( const QString &, const QString & ) ) );
  connect( api, SIGNAL( itemOnServer( const QString & ) ),
           SIGNAL( itemOnServer( const QString & ) ) );
  connect( api, SIGNAL( itemDownloaded( KCal::Incidence *, const QString &,
                                        const QString &, const QString &, const QString & ) ),
           SLOT( calendarItemDownloaded( KCal::Incidence *, const QString &,
                                         const QString &, const QString &, const QString & ) ) );

}

void BloggingCalendarAdaptor::slotFolderInfoRetrieved( const QString &id, const QString &name )
{
  emit folderInfoRetrieved( id, name, KPIM::FolderLister::JournalsFolder );
}

void BloggingCalendarAdaptor::slotUserInfoRetrieved( const QString &/*nick*/,
       const QString &/*user*/, const QString &/*email*/ )
{
kdDebug() << "BloggingCalendarAdaptor::slotUserInfoRetrieved"<<endl;
  mAuthenticated = true;
}

void BloggingCalendarAdaptor::setBaseUrl( const KURL &url )
{
  if ( mAPI ) {
    mAPI->setURL( url );
  }
}

void BloggingCalendarAdaptor::setUser( const QString &user )
{
  CalendarAdaptor::setUser( user );
  if ( mAPI ) {
    mAPI->setUsername( user );
  }
}

void BloggingCalendarAdaptor::setPassword( const QString &password )
{
  CalendarAdaptor::setPassword( password );
  if ( mAPI ) {
    mAPI->setPassword( password );
  }
}

void BloggingCalendarAdaptor::setUserPassword( KURL & )
{
  kdDebug(5800) << "BloggingCalendarAdaptor::setUserPassword" << endl;
}



KIO::Job *BloggingCalendarAdaptor::createLoginJob( const KURL &url,
                                                   const QString &user,
                                                   const QString &password )
{
  if ( mAPI ) {
    mAPI->setURL( url );
    mAPI->setUsername( user );
    mAPI->setPassword( password );
    return mAPI->createUserInfoJob();
  } else return 0;
}

KIO::Job *BloggingCalendarAdaptor::createListFoldersJob( const KURL &/*url*/ )
{
  if ( mAPI ) {
    return mAPI->createListFoldersJob();
  } else return 0;
}

KIO::TransferJob *BloggingCalendarAdaptor::createListItemsJob( const KURL &url )
{
  if ( mAPI ) {
    return mAPI->createListItemsJob( url );
  } else return 0;
}

KIO::TransferJob *BloggingCalendarAdaptor::createDownloadJob( const KURL &url,
                                     KPIM::GroupwareJob::ContentType ctype )
{
  if ( mAPI && (ctype == KPIM::GroupwareJob::Journal) ) {
    return mAPI->createDownloadJob( url );
  } else return 0;
}

KIO::Job *BloggingCalendarAdaptor::createRemoveJob( const KURL &/*url*/,
                         const KPIM::GroupwareUploadItem::List &/*deleteItem*/ )
{
  /* TODO: if ( mAPI ) {
    return mAPI->createRemoveJob( deleteItem );
  } else*/ return 0;
}

bool BloggingCalendarAdaptor::interpretLoginJob( KIO::Job *job )
{
kdDebug()<<"BloggingCalendarAdaptor::interpretLoginJob"<<endl;
  if ( mAPI && job ) {
kdDebug()<<"We have an API and a job"<<endl;
    mAuthenticated = false;
    mAPI->interpretUserInfoJob( job );
kdDebug() << "authenticated=" << mAuthenticated << endl;
    return mAuthenticated;
  } else return false;
}


void BloggingCalendarAdaptor::interpretListFoldersJob( KIO::Job *job, KPIM::FolderLister * )
{
kdDebug() << "BloggingCalendarAdaptor::interpretListFoldersJob" << endl;
  if ( mAPI && job ) {
    mAPI->interpretListFoldersJob( job );
  }
}


bool BloggingCalendarAdaptor::interpretListItemsJob( KIO::Job *job,
                                                    const QString &/*jobData*/ )
{
  if ( mAPI ) {
    return mAPI->interpretListItemsJob( job );
  } else {
    return false;
  }
}


bool BloggingCalendarAdaptor::interpretDownloadItemsJob( KIO::Job *job,
                                                    const QString &/*jobData*/ )
{
  if ( mAPI ) {
    return mAPI->interpretDownloadItemsJob( job );
  } else {
    return false;
  }
}

#include "bloggingcalendaradaptor.moc"
