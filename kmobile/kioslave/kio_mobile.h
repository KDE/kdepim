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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __KIO_MOBILE_H__
#define __KIO_MOBILE_H__

#include <qstring.h>
#include <qvaluelist.h>

#include <kio/slavebase.h>
#include <kmobiledevice.h>
#include <kmobileclient.h>

class KMobileProtocol : public KIO::SlaveBase
{
public:
  KMobileProtocol( const QCString &pool, const QCString &app );
  ~KMobileProtocol();

  void get( const KURL& url );
  void stat( const KURL& url );
  void mimetype( const KURL& url );
  void listDir( const KURL& url );

protected:
  int  getDeviceAndRessource(const QString &_path,
        QString &devName, QString &resource, QString &devPath,
        KMobileDevice::Capabilities &devCaps);

  QString folderMimeType(int cap);
  QString entryMimeType(int cap);

  void listRoot(const KURL& url);
  void listTopDeviceDir(const QString &devName);
  void listEntries(const QString &devName,
	const QString &resource, const QString &devPath,
	const KMobileDevice::Capabilities devCaps);

  void listAddressBook(const QString &devName, const QString &resource);
  int  getVCard( const QString &devName, QCString &result, QString &mime, const QString &path );

  void listCalendar(const QString &devName, const QString &resource);
  int  getCalendar( const QString &devName, QCString &result, QString &mime, const QString &path );

  void listNotes(const QString &devName, const QString &resource);
  int  getNote( const QString &devName, QCString &result, QString &mime, const QString &path );

  void listFileStorage(const QString &devName, const QString &resource, const QString &devPath);
  int  getFileStorage( const QString &devName, QCString &result, QString &mime, const QString &path );

private:
  KMobileClient m_dev;
};

#endif
