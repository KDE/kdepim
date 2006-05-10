 /*
    This file is part of kdepim.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef API_BLOGGER_H
#define API_BLOGGER_H

#include "API_Blog.h"

#include <QString>
#include <QVariant>


namespace KBlog {

class APIBlogger : public APIBlog
{
  public:
    APIBlogger( const KUrl &server, QObject *parent = 0L, const char *name = 0L ) : APIBlog( server, parent, name ) {}
    QString getFunctionName( blogFunctions type );
    QString interfaceName() const { return "Blogger API 1.0"; }


    KIO::Job *createUserInfoJob();
    KIO::Job *createListFoldersJob();
    KIO::TransferJob *createListItemsJob( const KUrl &url );
    KIO::TransferJob *createDownloadJob( const KUrl &url );
    KIO::TransferJob *createUploadJob( const KUrl &url, KBlog::BlogPosting *posting );
    KIO::TransferJob *createUploadNewJob( KBlog::BlogPosting *posting );
    KIO::Job *createRemoveJob( const KUrl &url, const QString &postid );

    bool interpretUserInfoJob( KIO::Job *job );
    void interpretListFoldersJob( KIO::Job *job );
    bool interpretListItemsJob( KIO::Job *job );
    bool interpretDownloadItemsJob( KIO::Job *job );
  protected:
    bool readPostingFromMap( BlogPosting *post, const QMap<QString, QVariant> &postInfo );
};

}
#endif
