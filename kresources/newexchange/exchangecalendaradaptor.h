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
#ifndef KPIM_EXCHANGECALENDARADAPTOR_H
#define KPIM_EXCHANGECALENDARADAPTOR_H

#include "exchangeglobals.h"

#include <groupwareresourcejob.h>
#include <davcalendaradaptor.h>
#include <kurl.h>
#include <qdom.h>

namespace KCal {

class Incidence;

class ExchangeCalendarUploadItem : public KPIM::GroupwareUploadItem
{
  public:
    ExchangeCalendarUploadItem( CalendarAdaptor *adaptor, KCal::Incidence *incidence, UploadType type );
    virtual ~ExchangeCalendarUploadItem() {}
    virtual KIO::TransferJob *createUploadNewJob(
            KPIM::GroupwareDataAdaptor *adaptor, const KURL &baseurl );
    virtual KIO::TransferJob *createUploadJob(
            KPIM::GroupwareDataAdaptor *adaptor, const KURL &url );

  protected:
    ExchangeCalendarUploadItem( UploadType type ) : KPIM::GroupwareUploadItem( type ) {}
    QDomDocument mDavData;
};

class ExchangeCalendarAdaptor : public DavCalendarAdaptor
{
  public:
    ExchangeCalendarAdaptor();

    void customAdaptDownloadUrl( KURL &url );
    void customAdaptUploadUrl( KURL &url );
    QString mimeType() const { return "message/rfc822"; }
    QCString identifier() const { return "KCalResourceExchange"; }
    QString defaultNewItemName( KPIM::GroupwareUploadItem *item );
    long flags() const { return GWResBatchDelete; }



    // Creating Jobs
    KIO::Job *createListFoldersJob( const KURL &url )
        { return ExchangeGlobals::createListFoldersJob( url ); }
    KIO::TransferJob *createListItemsJob( const KURL &url )
        { return ExchangeGlobals::createListItemsJob( url ); }
    KIO::TransferJob *createDownloadJob( const KURL &url, KPIM::FolderLister::ContentType ctype )
        { return ExchangeGlobals::createDownloadJob( this, url, ctype ); }
    KIO::Job *createRemoveJob( const KURL &uploadurl, const KPIM::GroupwareUploadItem::List &deletedItems )
        { return ExchangeGlobals::createRemoveJob( uploadurl, deletedItems ); }


    // Interpreting Jobs
    bool interpretListItemsJob( KIO::Job *job, const QString &jobData )
        { return ExchangeGlobals::interpretListItemsJob( this, job, jobData ); }
    bool interpretDownloadItemsJob( KIO::Job *job, const QString &jobData )
        { return ExchangeGlobals::interpretCalendarDownloadItemsJob( this, job, jobData );  }
    bool interpretUploadJob( KIO::Job *job, const QString &/*jobData*/ );
    bool interpretUploadNewJob( KIO::Job *job, const QString &/*jobData*/ );



    KPIM::GroupwareUploadItem *newUploadItem( KCal::Incidence*it,
           KPIM::GroupwareUploadItem::UploadType type );



    bool getFolderHasSubs( const QDomNode &folderNode )
        { return ExchangeGlobals::getFolderHasSubs( folderNode ); }
    KPIM::FolderLister::ContentType getContentType( const QDomNode &folderNode )
        { return ExchangeGlobals::getContentType( folderNode ); }
};

}

#endif
