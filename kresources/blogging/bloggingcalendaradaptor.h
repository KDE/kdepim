 /*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KCAL_BLOGGINGCALENDARADAPTOR_H
#define KCAL_BLOGGINGCALENDARADAPTOR_H

#include "calendaradaptor.h"
#include "groupwareuploadjob.h"
#include "API_Blog.h"
#include <kurl.h>

namespace KIO {
class Job;
}

namespace KCal {

class BloggingCalendarAdaptor : public CalendarAdaptor
{
Q_OBJECT
  public:
    BloggingCalendarAdaptor();

/**/    QCString identifier() const { return "KCalResourceBlogging"; }
/**/    long flags() const { return GWResNeedsLogon; }
/**/
/**/    void setBaseUrl( const KURL &url );
/**/    void setUser( const QString &user );
/**/    void setPassword( const QString &password );
/**/    // We don't want to set the username / pw for the URL!
/**/    void setUserPassword( KURL &url );
/**/
/**/    KBlog::APIBlog *api() const;
/**/    void setAPI( KBlog::APIBlog *api );
/**/
/**/    KIO::Job *createLoginJob( const KURL &url, const QString &user,
/**/                              const QString &password  );
/**/    KIO::Job *createListFoldersJob( const KURL &url );
/**/    KIO::TransferJob *createListItemsJob( const KURL &url );
/**/    KIO::TransferJob *createDownloadJob( const KURL &url,
/**/                                        KPIM::GroupwareJob::ContentType ctype );
    KIO::Job *createRemoveJob( const KURL &url,
                                  KPIM::GroupwareUploadItem::List deleteItems );

/**/    bool interpretLoginJob( KIO::Job *job );
/**/    void interpretListFoldersJob( KIO::Job *job, KPIM::FolderLister * );
/**/    bool interpretListItemsJob( KIO::Job *job, const QString &jobData );
/**/    bool interpretDownloadItemsJob( KIO::Job *job, const QString &jobData );

/**/  public slots:
/**/    void slotFolderInfoRetrieved( const QString &id, const QString &name );
/**/    void slotUserInfoRetrieved( const QString &nick, const QString &user,
/**/                            const QString &email );
/**/
/**/  protected:
/**/    KBlog::APIBlog *mAPI;
/**/    bool mAuthenticated;
/**/    static QString mAppID;
};

}

#endif
