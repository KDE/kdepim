/*
    This file is part of libkpimexchange
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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

#include <qstring.h>
#include <qapplication.h>
#include <qwidgetlist.h>
#include <qwidget.h>

#include <kurl.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kconfig.h>
#include <dcopclient.h>
#include <kio/authinfo.h>

#include "exchangeaccount.h"

using namespace KPIM;

ExchangeAccount::ExchangeAccount( QString host, QString account, QString password )
{
  mHost = host;
  mAccount = account;
  mPassword = password;
}

ExchangeAccount::ExchangeAccount( QString group )
{
  load( group );
}

ExchangeAccount::~ExchangeAccount()
{
}

QString endecryptStr( const QString &aStr ) 
{
  QString result;
  for (uint i = 0; i < aStr.length(); i++)
    result += (aStr[i].unicode() < 0x20) ? aStr[i] :
      QChar(0x1001F - aStr[i].unicode());
  return result;
}

void ExchangeAccount::save( QString const& group )
{
  kapp->config()->setGroup( group );
  kapp->config()->writeEntry( "host", mHost );
  kapp->config()->writeEntry( "user", mAccount );
  kapp->config()->writeEntry( "MS-ID", endecryptStr( mPassword ) );
  kapp->config()->sync();
}

void ExchangeAccount::load( QString const& group )
{
  kapp->config()->setGroup( group );

  QString host = kapp->config()->readEntry( "host" );
  if ( ! host.isNull() ) {
    mHost = host;
  }

  QString user = kapp->config()->readEntry( "user" );
  if ( ! user.isNull() ) {
    mAccount = user;
  }

  QString password = endecryptStr( kapp->config()->readEntry( "MS-ID" ) );
  if ( ! password.isNull() ) {
    mPassword = password;
  }
}

KURL ExchangeAccount::baseURL()
{
  KURL url = KURL( "webdav://" + mHost + "/exchange/" + mAccount );
  return url;
}

KURL ExchangeAccount::calendarURL()
{
  KURL url = baseURL();
  url.addPath( "Calendar" );
  return url;
}

void ExchangeAccount::authenticate()
{
  kdDebug() << "Entering ExchangeAccount::authenticate()" << endl;

  KIO::AuthInfo info;
  info.url = baseURL();
  info.username = mAccount;
  info.password = mPassword;

  long windowId;
  QWidgetList* widgets = QApplication::topLevelWidgets();
  if ( widgets->isEmpty() )
    windowId = 0;
  else
    windowId = widgets->first()->winId();
  delete widgets;

  kdDebug() << "window ID: " << windowId << endl;

  DCOPClient *dcopClient = new DCOPClient();
  dcopClient->attach();

  QByteArray params;
  QDataStream stream(params, IO_WriteOnly);
  stream << info << windowId;

  dcopClient->send( "kded", "kpasswdserver", "addAuthInfo(KIO::AuthInfo, long int)", params );

  dcopClient->detach();
  delete dcopClient;
}

