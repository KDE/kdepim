/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <qstring.h>
#include <kurl.h>
#include <kapplication.h>
#include <kconfig.h>

#include "exchangeaccount.h"
using namespace KPIM;

ExchangeAccount::ExchangeAccount( QString& host, QString& account, QString& password )
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

void ExchangeAccount::save( QString const& group )
{
  kapp->config()->setGroup( group );
  kapp->config()->writeEntry( "host", mHost );
  kapp->config()->writeEntry( "user", mAccount );
  kapp->config()->writeEntry( "password", mPassword );
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

  QString password = kapp->config()->readEntry( "password" );
  if ( ! password.isNull() ) {
    mPassword = password;
  }
}

KURL ExchangeAccount::baseURL()
{
  KURL url = KURL( "webdav://" + mHost + "/exchange/" + mAccount );
  url.setUser( mAccount );
  url.setPass( mPassword );
  return url;
  
}

KURL ExchangeAccount::calendarURL()
{
  KURL url = baseURL();
  url.addPath( "Calendar" );
  return url;
}

