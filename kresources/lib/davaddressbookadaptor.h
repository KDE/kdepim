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
#ifndef KABC_DAVADDRESSBOOKADAPTOR_H
#define KCAL_DAVADDRESSBOOKADAPTOR_H

#include "addressbookadaptor.h"
#include "folderlister.h"
#include <qdom.h>

namespace KABC {

class DavAddressBookAdaptor : public AddressBookAdaptor
{
  public:
    DavAddressBookAdaptor() {}
    
    /** Interprets the results returned by the liste job (created by
        createListFoldersJob(url) ). Typically, this adds an Entry to the mFolders list if
        the job describes a folder of the appropriate type, by emitting 
        folderInformationRetrieved( href, displayName, type ). If the folder has
        subfolders, just emit retrieveSubfolder( href ) for each of them. */
    virtual void interpretListFoldersJob( KIO::Job *job, KPIM::FolderLister *folderLister );
    /** Returns the type of folder retrieved in the dom node. Typically, you'll
        compare the DAV:resourcetype property with some values. */
    virtual KPIM::FolderLister::FolderType getFolderType( const QDomNode &folderNode ) = 0;
    /** Extract from the dav response whether the folder has subitems that need
        to be examined */
    virtual bool getFolderHasSubs( const QDomNode &folderNode ) = 0;
};

}

#endif
