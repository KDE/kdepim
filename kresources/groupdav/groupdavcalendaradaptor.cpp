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

#include "groupdavcalendaradaptor.h"
#include "webdavhandler.h"

#include <kdebug.h>

using namespace KCal;

GroupDavCalendarAdaptor::GroupDavCalendarAdaptor() : DavCalendarAdaptor()
{
}

void GroupDavCalendarAdaptor::customAdaptDownloadUrl( KURL &url )
{
  url = WebdavHandler::toDAV( url );
}

void GroupDavCalendarAdaptor::customAdaptUploadUrl( KURL &url )
{
  url = WebdavHandler::toDAV( url );
  // FIXME: Find a good place where we can obtain the path for a new item
//  url.setPath( url.path() + "/new.ics" );
}

