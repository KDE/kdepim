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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include <tqstringlist.h>

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
  PRINT_DEBUG << TQString("attached to DCOP server %1\n").arg(ok?"sucessful.":"failed.");

//  m_clientAppId = registerAs("kmobileclient");
//  PRINT_DEBUG << TQString("registered as DCOP client %1\n").arg(m_clientAppId);

  isKMobileAvailable();
}

KMobileClient::~KMobileClient()
{
  detach();
  PRINT_DEBUG << TQString("detached from server\n");
}


bool KMobileClient::isKMobileAvailable()
{
  bool available = isApplicationRegistered(m_kmobileApp);
  PRINT_DEBUG << TQString("KMobile DCOP server: %1\n").arg(available?"available.":"not available");
  if (!available) {
	startKMobileApplication();
	// check again...
	available = isApplicationRegistered(m_kmobileApp);
  }
  return available;
}

bool KMobileClient::startKMobileApplication()
{
  TQByteArray data;
  TQDataStream arg(data, IO_WriteOnly);
  TQStringList params;
  params << "--minimized";
  arg << TQString("kmobile") << params;
  TQCString replyType;
  TQByteArray replyData;
  bool ok = call("klauncher", "klauncher", "kdeinit_exec_wait(TQString,TQStringList)", data, replyType, replyData);
  PRINT_DEBUG << TQString("DCOP-CALL to klauncher: %1\n").arg(ok?"ok.":"failed.");
  return ok;
}


/**
 * DCOP - IMPLEMENTATION
 */

#define KMOBILECLIENT_USE_EVENTLOOP true
#define KMOBILECLIENT_TIMEOUT (4*1000) // 4 seconds


#define PREPARE( FUNC, PARAMS ) \
  TQByteArray data; \
  TQDataStream arg(data, IO_WriteOnly); \
  arg << PARAMS; \
  TQCString replyType; \
  TQByteArray replyData; \
  bool ok = call(m_kmobileApp, m_kmobileObj, FUNC, data, replyType, replyData, KMOBILECLIENT_USE_EVENTLOOP, KMOBILECLIENT_TIMEOUT); \
  PRINT_DEBUG << TQString("DCOP-CALL to %1: %2\n").arg(FUNC).arg(ok?"ok.":"FAILED.")

#define RETURN_TYPE( FUNC, PARAMS, RETURN_TYPE ) \
  PREPARE( FUNC, PARAMS ); \
  TQDataStream reply(replyData, IO_ReadOnly); \
  RETURN_TYPE ret; \
  if (ok) \
	reply >> ret; \
  return ret;

#define RETURN_TYPE_DEFAULT( FUNC, PARAMS, RETURN_TYPE, RETURN_DEFAULT ) \
  PREPARE( FUNC, PARAMS ); \
  TQDataStream reply(replyData, IO_ReadOnly); \
  RETURN_TYPE ret = RETURN_DEFAULT; \
  if (ok) \
	reply >> ret; \
  return ret;

#define RETURN_QSTRING( FUNC, PARAMS ) \
  RETURN_TYPE( FUNC, PARAMS, TQString )

#define RETURN_BOOL( FUNC, PARAMS ) \
  RETURN_TYPE_DEFAULT( FUNC, PARAMS, bool, false )

#define RETURN_INT( FUNC, PARAMS ) \
  RETURN_TYPE_DEFAULT( FUNC, PARAMS, int , 0 )



TQStringList KMobileClient::deviceNames()
{
  if (!isKMobileAvailable())
	return TQStringList();
  RETURN_TYPE( "deviceNames()", TQString::fromLatin1(""), TQStringList );
}

void KMobileClient::removeDevice( TQString deviceName )
{
  PREPARE( "removeDevice(TQString)", deviceName );
  Q_UNUSED(ok);
}

void KMobileClient::configDevice( TQString deviceName )
{
  PREPARE( "configDevice(TQString)", deviceName );
  Q_UNUSED(ok);
}


bool KMobileClient::connectDevice( TQString deviceName )
{
  RETURN_BOOL( "connectDevice(TQString)", deviceName );
}

bool KMobileClient::disconnectDevice( TQString deviceName )
{
  RETURN_BOOL( "disconnectDevice(TQString)", deviceName );
}

bool KMobileClient::connected( TQString deviceName )
{
  RETURN_BOOL( "connected(TQString)", deviceName );
}


TQString KMobileClient::deviceClassName( TQString deviceName )
{
  RETURN_QSTRING( "deviceClassName(TQString)", deviceName );
}

TQString KMobileClient::deviceName( TQString deviceName )
{
  RETURN_QSTRING( "deviceName(TQString)", deviceName );
}

TQString KMobileClient::revision( TQString deviceName )
{
  RETURN_QSTRING( "revision(TQString)", deviceName );
}

int KMobileClient::classType( TQString deviceName )
{
  RETURN_INT( "classType(TQString)", deviceName );
}

int KMobileClient::capabilities( TQString deviceName )
{
  RETURN_INT( "capabilities(TQString)", deviceName );
}

TQString KMobileClient::nameForCap( TQString deviceName, int cap )
{
  RETURN_QSTRING( "nameForCap(TQString,int)", deviceName << cap );
}

TQString KMobileClient::iconFileName( TQString deviceName )
{
  RETURN_QSTRING( "iconFileName(TQString)", deviceName );
}

int KMobileClient::numAddresses( TQString deviceName )
{
  RETURN_INT( "numAddresses(TQString)", deviceName );
}

TQString KMobileClient::readAddress( TQString deviceName, int index )
{
  RETURN_QSTRING( "readAddress(TQString,int)", deviceName << index );
}

bool KMobileClient::storeAddress( TQString deviceName, int index, TQString vcard, bool append )
{
  RETURN_BOOL( "storeAddress(TQString,int,TQString,bool)", deviceName << index << vcard << append );
}

int KMobileClient::numCalendarEntries( TQString deviceName )
{
  RETURN_INT( "numCalendarEntries(TQString)", deviceName );
}

int KMobileClient::numNotes( TQString deviceName )
{
  RETURN_INT( "numNotes(TQString)", deviceName );
}

TQString KMobileClient::readNote( TQString deviceName, int index )
{
  RETURN_QSTRING( "readNote(TQString,int)", deviceName << index );
}

bool KMobileClient::storeNote( TQString deviceName, int index, TQString note )
{
  RETURN_BOOL( "storeNote(TQString,int,TQString)", deviceName << index << note );
}



#undef PRINT_DEBUG

#include "kmobileclient.moc"
