/***************************************************************************
   Copyright (c) 2004 Kevin Ottens <ervin ipsquad net>
   Rearranged by Marco Gulino <marco@kmobiletools.org> for the mobile protocol

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef _mobile_H_
#define _mobile_H_

#include <qstring.h>
#include <q3cstring.h>

#include <kurl.h>
#include <kio/global.h>
#include <kio/slavebase.h>
#include <dcopclient.h>

#include "mobileimpl.h"

class Q3CString;

class kio_mobileProtocol : public KIO::SlaveBase
{
public:
    kio_mobileProtocol(const Q3CString &pool_socket, const Q3CString &app_socket);
    virtual ~kio_mobileProtocol();
//     virtual void mimetype(const KUrl& url);
//     virtual void get(const KUrl& url);
    virtual void listDir(const KUrl &url);
    virtual void stat(const KUrl &url);

    private:
        DCOPClient m_dcop;
        void listRoot();
        MobileImpl m_impl;
};

#endif
