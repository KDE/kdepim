/*
    This file is part of libkcal.

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
#ifndef KCAL_EXCHANGEFOLDERLISTER_H
#define KCAL_EXCHANGEFOLDERLISTER_H

#include <davfolderlister.h>

namespace KPIM {

class ExchangeFolderLister : public DavFolderLister
{
    Q_OBJECT
  public:
    ExchangeFolderLister( Type type );

  protected:
    virtual KIO::Job *createJob( const KURL &url );
    virtual FolderType getFolderType( const QDomNode &folderNode );
    virtual bool getFolderHasSubs( const QDomNode &folderNode );

};

}

#endif
