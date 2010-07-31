/*  This file is part of the KDE mobile library.
    Copyright (C) 2004 Helge Deller <deller@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef __KIO_MOBILE_H__
#define __KIO_MOBILE_H__

#include <tqstring.h>
#include <tqvaluelist.h>

#include <kio/slavebase.h>
#include <kmobiledevice.h>
#include <kmobileclient.h>

class KMobileProtocol : public KIO::SlaveBase
{
public:
  KMobileProtocol( const TQCString &pool, const TQCString &app );
  ~KMobileProtocol();

  void get( const KURL& url );
  void stat( const KURL& url );
  void mimetype( const KURL& url );
  void listDir( const KURL& url );

protected:
  int  getDeviceAndRessource(const TQString &_path,
        TQString &devName, TQString &resource, TQString &devPath,
        KMobileDevice::Capabilities &devCaps);

  TQString folderMimeType(int cap);
  TQString entryMimeType(int cap);

  void listRoot(const KURL& url);
  void listTopDeviceDir(const TQString &devName);
  void listEntries(const TQString &devName,
	const TQString &resource, const TQString &devPath,
	const KMobileDevice::Capabilities devCaps);

  void listAddressBook(const TQString &devName, const TQString &resource);
  int  getVCard( const TQString &devName, TQCString &result, TQString &mime, const TQString &path );

  void listCalendar(const TQString &devName, const TQString &resource);
  int  getCalendar( const TQString &devName, TQCString &result, TQString &mime, const TQString &path );

  void listNotes(const TQString &devName, const TQString &resource);
  int  getNote( const TQString &devName, TQCString &result, TQString &mime, const TQString &path );

  void listFileStorage(const TQString &devName, const TQString &resource, const TQString &devPath);
  int  getFileStorage( const TQString &devName, TQCString &result, TQString &mime, const TQString &path );

private:
  KMobileClient m_dev;
};

#endif
