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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef API_BLOGGER_H
#define API_BLOGGER_H

#include "API_Blog.h"

#include <qstring.h>
#include <qvariant.h>
#include <qvaluelist.h>

namespace KBlog {

class APIBlogger : public APIBlog
{
  public:
    APIBlogger( const KURL &server, QObject *parent = 0L, const char *name = 0L ) : APIBlog( server, parent, name ) {}
    QString getFunctionName( blogFunctions type );
    QString interfaceName() const { return "Blogger API 1.0"; }


    KIO::Job *createUserInfoJob();
    KIO::Job *createListFoldersJob();
    KIO::TransferJob *createListItemsJob( const KURL &url );
    KIO::TransferJob *createDownloadJob( const KURL &url );
//     KIO::Job *createRemoveJob ( const KURL &url, KPIM::GroupwareUploadItem::List deletedItems );

    bool interpretUserInfoJob( KIO::Job *job );
    void interpretListFoldersJob( KIO::Job *job );
    bool interpretListItemsJob( KIO::Job *job );
    bool interpretDownloadItemsJob( KIO::Job *job );
  protected:
    bool readPostingFromMap( BlogPosting *post, const QMap<QString, QVariant> &postInfo );
};

}
#endif
