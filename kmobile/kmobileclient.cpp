/*  This file is part of the KDE KMobile library
    Copyright (C) 2003-2005 Helge Deller <deller@kde.org>

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

#include <qstringlist.h>

#include <ktrader.h>
#include <klocale.h>
#include <kdebug.h>

#include "kmobileclient.h"

#define KMOBILECLIENT_DEBUG_AREA 0
#define PRINT_DEBUG kdDebug(KMOBILECLIENT_DEBUG_AREA) << "KMobileClient: "

KMobileClient::KMobileClient()
    : DCOPClient()
{
  // initialize Application and Object of remote DCOP-aware KMobile application
  m_kmobileApp = "kmobile";
  m_kmobileObj = "kmobileIface";

  bool ok = attach();
  PRINT_DEBUG << QString("attached to DCOP server %1\n").arg(ok?"sucessful.":"failed.");

//  m_clientAppId = registerAs("kmobileclient");
//  PRINT_DEBUG << QString("registered as DCOP client %1\n").arg(m_clientAppId);

  isKMobileAvailable();
}

KMobileClient::~KMobileClient()
{
  detach();
  PRINT_DEBUG << QString("detached from server\n");
}


bool KMobileClient::isKMobileAvailable()
{
  bool available = isApplicationRegistered(m_kmobileApp);
  PRINT_DEBUG << QString("KMobile DCOP server: %1\n").arg(available?"available.":"not available");
  if (!available) {
	startKMobileApplication();
	// check again...
	available = isApplicationRegistered(m_kmobileApp);
  }
  return available;
}

bool KMobileClient::startKMobileApplication()
{
  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);
  QStringList params;
  params << "--minimized";
  arg << QString("kmobile") << params;
  QCString replyType;
  QByteArray replyData;
  bool ok = call("klauncher", "klauncher", "kdeinit_exec_wait(QString,QStringList)", data, replyType, replyData);
  PRINT_DEBUG << QString("DCOP-CALL to klauncher: %1\n").arg(ok?"ok.":"failed.");
  return ok;
}


/**
 * DCOP - IMPLEMENTATION
 */

#define KMOBILECLIENT_USE_EVENTLOOP true
#define KMOBILECLIENT_TIMEOUT (4*1000) // 4 seconds


#define PREPARE( FUNC, PARAMS ) \
  QByteArray data; \
  QDataStream arg(data, IO_WriteOnly); \
  arg << PARAMS; \
  QCString replyType; \
  QByteArray replyData; \
  bool ok = call(m_kmobileApp, m_kmobileObj, FUNC, data, replyType, replyData, KMOBILECLIENT_USE_EVENTLOOP, KMOBILECLIENT_TIMEOUT); \
  PRINT_DEBUG << QString("DCOP-CALL to %1: %2\n").arg(FUNC).arg(ok?"ok.":"FAILED.")

#define RETURN_TYPE( FUNC, PARAMS, RETURN_TYPE ) \
  PREPARE( FUNC, PARAMS ); \
  QDataStream reply(replyData, IO_ReadOnly); \
  RETURN_TYPE ret; \
  if (ok) \
	reply >> ret; \
  return ret;

#define RETURN_TYPE_DEFAULT( FUNC, PARAMS, RETURN_TYPE, RETURN_DEFAULT ) \
  PREPARE( FUNC, PARAMS ); \
  QDataStream reply(replyData, IO_ReadOnly); \
  RETURN_TYPE ret = RETURN_DEFAULT; \
  if (ok) \
	reply >> ret; \
  return ret;

#define RETURN_QSTRING( FUNC, PARAMS ) \
  RETURN_TYPE( FUNC, PARAMS, QString )

#define RETURN_BOOL( FUNC, PARAMS ) \
  RETURN_TYPE_DEFAULT( FUNC, PARAMS, bool, false )

#define RETURN_INT( FUNC, PARAMS ) \
  RETURN_TYPE_DEFAULT( FUNC, PARAMS, int , 0 )



QStringList KMobileClient::deviceNames()
{
  if (!isKMobileAvailable())
	return QStringList();
  RETURN_TYPE( "deviceNames()", QString::fromLatin1(""), QStringList );
}

void KMobileClient::removeDevice( QString deviceName )
{
  PREPARE( "removeDevice(QString)", deviceName );
  Q_UNUSED(ok);
}

void KMobileClient::configDevice( QString deviceName )
{
  PREPARE( "configDevice(QString)", deviceName );
  Q_UNUSED(ok);
}


bool KMobileClient::connectDevice( QString deviceName )
{
  RETURN_BOOL( "connectDevice(QString)", deviceName );
}

bool KMobileClient::disconnectDevice( QString deviceName )
{
  RETURN_BOOL( "disconnectDevice(QString)", deviceName );
}

bool KMobileClient::connected( QString deviceName )
{
  RETURN_BOOL( "connected(QString)", deviceName );
}


QString KMobileClient::deviceClassName( QString deviceName )
{
  RETURN_QSTRING( "deviceClassName(QString)", deviceName );
}

QString KMobileClient::deviceName( QString deviceName )
{
  RETURN_QSTRING( "deviceName(QString)", deviceName );
}

QString KMobileClient::revision( QString deviceName )
{
  RETURN_QSTRING( "revision(QString)", deviceName );
}

int KMobileClient::classType( QString deviceName )
{
  RETURN_INT( "classType(QString)", deviceName );
}

int KMobileClient::capabilities( QString deviceName )
{
  RETURN_INT( "capabilities(QString)", deviceName );
}

QString KMobileClient::nameForCap( QString deviceName, int cap )
{
  RETURN_QSTRING( "nameForCap(QString,int)", deviceName << cap );
}

QString KMobileClient::iconFileName( QString deviceName )
{
  RETURN_QSTRING( "iconFileName(QString)", deviceName );
}

int KMobileClient::numAddresses( QString deviceName )
{
  RETURN_INT( "numAddresses(QString)", deviceName );
}

QString KMobileClient::readAddress( QString deviceName, int index )
{
  RETURN_QSTRING( "readAddress(QString,int)", deviceName << index );
}

bool KMobileClient::storeAddress( QString deviceName, int index, QString vcard, bool append )
{
  RETURN_BOOL( "storeAddress(QString,int,QString,bool)", deviceName << index << vcard << append );
}

int KMobileClient::numCalendarEntries( QString deviceName )
{
  RETURN_INT( "numCalendarEntries(QString)", deviceName );
}

int KMobileClient::numNotes( QString deviceName )
{
  RETURN_INT( "numNotes(QString)", deviceName );
}

QString KMobileClient::readNote( QString deviceName, int index )
{
  RETURN_QSTRING( "readNote(QString,int)", deviceName << index );
}

bool KMobileClient::storeNote( QString deviceName, int index, QString note )
{
  RETURN_BOOL( "storeNote(QString,int,QString)", deviceName << index << note );
}



#undef PRINT_DEBUG

#include "kmobileclient.moc"
