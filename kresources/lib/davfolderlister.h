/*
    This file is part of libkcal.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
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
#ifndef KPIM_DAVFOLDERLISTER_H
#define KPIM_DAVFOLDERLISTER_H

#include <folderlister.h>
#include <qdom.h>

namespace KIO {
class DavJob;
}

namespace KPIM {

class DavFolderLister : public FolderLister
{
    Q_OBJECT
  public:
    DavFolderLister( Type );

  protected:
    /** Applied custom adjustments to the URL, e.g. changes http:// -> webdav://, etc. */
    virtual KURL customAdjustUrl( const KURL &u );
    /** Creates the job to retrieve information about the folder at the given
        url. It is expected that the job retrieves at least the following props:
          DAV:displayname, DAV:resourcetype, DAV:hassubs
    */
    virtual KIO::Job *createJob( const KURL &url );
    /** Interprets the results returned by the liste job (created by
        createJob(url) ). Typically, this adds an Entry to the mFolders list if
        the job describes a folder of the appropriate type. If the folder has
        subfolders, just call doRetrieveFolder(url) recursively. */
    virtual void interpretFolderResult( KIO::Job *job );
    /** Returns the type of folder retrieved in the dom node. Typically, you'll
        compare the DAV:resourcetype property with some values. */
    virtual FolderType getFolderType( const QDomNode &folderNode ) = 0;
    /** Extract from the dav response whether the folder has subitems that need
        to be examined */
    virtual bool getFolderHasSubs( const QDomNode &folderNode ) = 0;
};

}

#endif
