/*
 *   This file is part of ScalixAdmin.
 *
 *   Copyright (C) 2007 Trolltech ASA. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kio/scheduler.h>
#include <kstringhandler.h>

#include "settings.h"

Settings* Settings::mSelf = 0;

Settings::Settings()
{
  mSlave = KIO::Scheduler::getConnectedSlave( accountUrl(),
                                              accountData() );
}

Settings::~Settings()
{
}

Settings* Settings::self()
{
  if ( !mSelf )
    mSelf = new Settings;

  return mSelf;
}

KIO::MetaData Settings::accountData() const
{
  KConfig config( "scalixadminrc" );
  KConfigGroup group( &config, "Account" );

  KIO::MetaData data;
  data.insert( "auth", group.readEntry( "auth", QString() ) );
  data.insert( "tls", group.readEntry( "use-tls", false ) ? "on" : "off" );

  return data;
}

KUrl Settings::accountUrl() const
{
  KConfig config( "scalixadminrc" );
  KConfigGroup group( &config, "Account" );

  KUrl url;
  url.setProtocol( group.readEntry( "use-ssl", false ) ? "imaps" : "imap" );
  url.setUser( group.readEntry( "user", QString() ) );
  url.setPass( KStringHandler::obscure( group.readEntry( "pass", QString() ) ) );
  url.setHost( group.readEntry( "host", QString() ) );
  url.setPort( group.readEntry( "port", 143 ) );

  return url;
}

QString Settings::accountPassword() const
{
  KConfig config( "scalixadminrc" );
  KConfigGroup group( &config, "Account" );

  return KStringHandler::obscure( group.readEntry( "pass", QString() ) );
}

KIO::Slave* Settings::globalSlave() const
{
  return mSlave;
}

QString Settings::rulesWizardUrl() const
{
  KConfig config( "scalixadminrc" );
  KConfigGroup group( &config, "Misc" );

  QString url = group.readEntry( "rulesWizardUrl", QString() );
  if ( url.isEmpty() ) {
    KConfigGroup group( &config, "Account" );
    url = QString( "http://%1/Scalix/rw/?username=%2" ).arg( group.readEntry( "host", QString() ) )
                                                       .arg( group.readEntry( "user", QString() ) );
  }

  return url;
}

QString Settings::ldapHost() const
{
  KConfig config( "scalixadminrc" );
  KConfigGroup group( &config, "LDAP" );
  return group.readEntry( "host", QString() );
}

QString Settings::ldapPort() const
{
  KConfig config( "scalixadminrc" );
  KConfigGroup group( &config, "LDAP" );
  return group.readEntry( "port", "389" );
}

QString Settings::ldapBase() const
{
  KConfig config( "scalixadminrc" );
  KConfigGroup group( &config, "LDAP" );
  return group.readEntry( "base", QString() );
}

QString Settings::ldapBindDn() const
{
  KConfig config( "scalixadminrc" );
  KConfigGroup group( &config, "LDAP" );
  return group.readEntry( "bindDn", QString() );
}

QString Settings::ldapPassword() const
{
  KConfig config( "scalixadminrc" );
  KConfigGroup group( &config, "LDAP" );
  return group.readEntry( "password", QString() );
}
