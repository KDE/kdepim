/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "groupdavaddressbookadaptor.h"
#include "groupdavglobals.h"
#include "davgroupwareglobals.h"
#include "webdavhandler.h"

#include <kabc/addressee.h>
#include <kabc/vcardconverter.h>
#include <kabcresourcecached.h>

#include <kio/job.h>
#include <kdebug.h>

using namespace KABC;

GroupDavAddressBookAdaptor::GroupDavAddressBookAdaptor() : DavAddressBookAdaptor()
{
}

void GroupDavAddressBookAdaptor::customAdaptDownloadUrl( KURL &url )
{
  url = WebdavHandler::toDAV( url );
}

void GroupDavAddressBookAdaptor::customAdaptUploadUrl( KURL &url )
{
kdDebug()<<"GroupDavAddressBookAdaptor::adaptUploadUrl( "<<url.url()<<")"<<endl;
  url = WebdavHandler::toDAV( url );
//   url.setPath( url.path() + "/new.vcf" );
// url.addPath( "new.vcf" );
kdDebug()<<"after GroupDavAddressBookAdaptor::adaptUploadUrl( "<<url.url()<<")"<<endl;
}
