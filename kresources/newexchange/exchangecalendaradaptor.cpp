/*
    This file is part of kdepim.

    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "exchangecalendaradaptor.h"
#include "exchangeglobals.h"
#include "exchangeconvertercalendar.h"
#include <davgroupwareglobals.h>
#include <webdavhandler.h>
#include <kdebug.h>
#include <kio/davjob.h>
#include <libkcal/resourcecached.h>
#include "davcalendaradaptor.h"

using namespace KCal;

ExchangeCalendarUploadItem::ExchangeCalendarUploadItem( CalendarAdaptor *adaptor, KCal::Incidence *incidence, KPIM::GroupwareUploadItem::UploadType type )
    : GroupwareUploadItem( type )
{
  if ( incidence && adaptor ) {
    setUrl( incidence->customProperty( adaptor->identifier(), "storagelocation" ) );
    setUid( incidence->uid() );

    ExchangeConverterCalendar format;
    format.setTimeZone( adaptor->resource()->timeZoneId() );
    mDavData = format.createWebDAV( incidence );
  }
}

KIO::TransferJob *ExchangeCalendarUploadItem::createUploadJob( KPIM::GroupwareDataAdaptor *adaptor, const KURL &baseUrl )
{
kdDebug()<<"ExchangeCalendarUploadItem::createUploadJob, adaptor="<<adaptor<<", URL="<<baseUrl.url()<<endl;
  Q_ASSERT( adaptor );
kdDebug()<<"After Q_ASSERT"<<endl;
  if ( !adaptor ) return 0;
kdDebug()<<"After (!adaptor) check"<<endl;
  KURL upUrl( url() );
  adaptor->adaptUploadUrl( upUrl );
  kdDebug() << "Uploading to: " << upUrl.prettyURL() << endl;
  KIO::DavJob *job = KIO::davPropPatch( upUrl, mDavData, false );
  return job;
}

KIO::TransferJob *ExchangeCalendarUploadItem::createUploadNewJob( KPIM::GroupwareDataAdaptor *adaptor, const KURL &baseurl )
{
kdDebug()<<"ExchangeCalendarUploadItem::createUploadNewJob"<<endl;
  KURL url( baseurl );
  // TODO: Check if this URL doesn't exist yet!
  url.addPath( uid() + ".EML" );
  setUrl( url );
//url.addPath("newItem.EML");
kdDebug()<<"Upload path: "<<url.url()<<endl;
  return createUploadJob( adaptor, url );
}



ExchangeCalendarAdaptor::ExchangeCalendarAdaptor() : DavCalendarAdaptor()
{
}

void ExchangeCalendarAdaptor::customAdaptDownloadUrl( KURL &url )
{
  url = WebdavHandler::toDAV( url );
}

void ExchangeCalendarAdaptor::customAdaptUploadUrl( KURL &url )
{
  url = WebdavHandler::toDAV( url );
//   url.setPath( url.path() + "/NewItem.EML" );
}


/* Removing items: old version of the exchange resource:  If the event is part
   of a sequence of recurring event, we need to delete the master!
void ExchangeDelete::findUidSingleMaster( QString const& uid )
{
  QString query =
        "SELECT \"DAV:href\", \"urn:schemas:calendar:uid\"\r\n"
        "FROM Scope('shallow traversal of \"\"')\r\n"
        "WHERE \"urn:schemas:calendar:uid\" = '" + uid + "'\r\n"
	" AND (\"urn:schemas:calendar:instancetype\" = 0\r\n"
	"      OR \"urn:schemas:calendar:instancetype\" = 1)\r\n";

  KIO::DavJob* job = KIO::davSearch( mAccount->calendarURL(), "DAV:", "sql", query, false );
  job->setWindow( mWindow );
  connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(slotFindUidResult(KIO::Job *)));
}

*/


QString ExchangeCalendarAdaptor::defaultNewItemName( KPIM::GroupwareUploadItem *item ) {
  if ( item ) return item->uid()+".EML";
  else return QString::null;
}

KPIM::GroupwareUploadItem *ExchangeCalendarAdaptor::newUploadItem( KCal::Incidence*it,
           KPIM::GroupwareUploadItem::UploadType type )
{
  return new ExchangeCalendarUploadItem( this, it, type );
}

bool ExchangeCalendarAdaptor::interpretUploadJob( KIO::Job *job, const QString &/*jobData*/ )
{
  kdDebug(7000) << "ExchangeCalendarAdaptor::interpretUploadJob " << endl;
  KIO::DavJob *davjob = dynamic_cast<KIO::DavJob*>(job);
  bool error = job->error();
  const QString err = job->errorString();

  if ( davjob ) {
    if ( error ) {
      emit itemUploadError( davjob->url().url(), err );
      return false;
    } else {
      QDomDocument doc( davjob->response() );
      // TODO: extract the href and the items that got a 404. If there's
      // something important among them, issue the "usual" not-all-settings-uploaded
      // warning to the user.
      KURL url( davjob->url() );
      // We don't know the local id here (and we don't want to extract it from
      // the idMapper, that's the task of the receiver
      emit itemUploaded( QString::null, url.url() );
    }
    return true;
  } else {
    return false;
  }
}

bool ExchangeCalendarAdaptor::interpretUploadNewJob( KIO::Job *job, const QString &/*jobData*/ )
{
// TODO: How does the incidence mapper know the old/new ids???
  kdDebug(7000) << "ExchangeCalendarAdaptor::interpretUploadNewJob " << endl;
  KIO::DavJob *davjob = dynamic_cast<KIO::DavJob*>(job);
  bool error = job->error();
  const QString err = job->errorString();

  if ( davjob ) {
    if ( error ) {
      emit itemUploadNewError( davjob->url().url(), err );
      return false;
    } else {
      QDomDocument doc( davjob->response() );
      // TODO: extract the href and the items that got a 404. If there's
      // something important among them, issue the "usual" not-all-settings-uploaded
      // warning to the user.
      KURL url( davjob->url() );
      // We don't know the local id here (and we don't want to extract it from
      // the idMapper, that's the task of the receiver
      emit itemUploadedNew( QString::null, url.url() );
    }
    return true;
  } else {
    return false;
  }
}

