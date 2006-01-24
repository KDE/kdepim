 /*
    This file is part of kdepim.

    Copyright (C) 2004-05 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KCAL_BLOGGINGCALENDARADAPTOR_H
#define KCAL_BLOGGINGCALENDARADAPTOR_H

#include "calendaradaptor.h"
#include "groupwareuploadjob.h"
#include "API_Blog.h"
#include <kurl.h>
//Added by qt3to4:
#include <QList>
#include <QByteArray>

namespace KIO {
class Job;
}

namespace KCal {

class BloggingUploadItem : public KPIM::GroupwareUploadItem
{
  public:
    BloggingUploadItem( KBlog::APIBlog *api, CalendarAdaptor *adaptor, KCal::Incidence *incidence, UploadType type );
    virtual ~BloggingUploadItem();
    virtual KIO::TransferJob *createUploadNewJob(
            KPIM::GroupwareDataAdaptor *adaptor, const KUrl &baseurl );
    virtual KIO::TransferJob *createUploadJob(
            KPIM::GroupwareDataAdaptor *adaptor, const KUrl &url );

  protected:
    BloggingUploadItem( UploadType type ) : KPIM::GroupwareUploadItem( type ) {}
    KBlog::BlogPosting *mPosting;
    KBlog::APIBlog *mAPI;
};

class BloggingCalendarAdaptor : public CalendarAdaptor
{
Q_OBJECT
  public:
    BloggingCalendarAdaptor();
    QList<KPIM::FolderLister::ContentType> supportedTypes()
    {
      QList<KPIM::FolderLister::ContentType> types;
      types << KPIM::FolderLister::Journal;
      return types;
    }

    QByteArray identifier() const { return "KCalResourceBlogging"; }
    long flags() const { return GWResNeedsLogon; }

    void setBaseURL( const KUrl &url );
    void setUser( const QString &user );
    void setPassword( const QString &password );
    // We don't want to set the username / pw for the URL!
    void setUserPassword( KUrl &url );

    KBlog::APIBlog *api() const;
    void setAPI( KBlog::APIBlog *api );

    KIO::Job *createLoginJob( const KUrl &url, const QString &user,
                              const QString &password  );
    KIO::Job *createListFoldersJob( const KUrl &url );
    KIO::TransferJob *createListItemsJob( const KUrl &url );
    KIO::TransferJob *createDownloadJob( const KUrl &url,
                                    KPIM::FolderLister::ContentType ctype );
    KIO::Job *createRemoveJob( const KUrl &url, KPIM::GroupwareUploadItem *deleteItem );

    bool interpretLoginJob( KIO::Job *job );
    void interpretListFoldersJob( KIO::Job *job, KPIM::FolderLister * );
    bool interpretListItemsJob( KIO::Job *job, const QString &jobData );
    bool interpretDownloadItemsJob( KIO::Job *job, const QString &jobData );

  public slots:
    void slotFolderInfoRetrieved( const QString &id, const QString &name );
    void slotUserInfoRetrieved( const QString &nick, const QString &user,
                            const QString &email );

  protected:
    KPIM::GroupwareUploadItem *newUploadItem( KCal::Incidence*it,
                                  KPIM::GroupwareUploadItem::UploadType type );

    KBlog::APIBlog *mAPI;
    bool mAuthenticated;
    static QString mAppID;
};

}

#endif
