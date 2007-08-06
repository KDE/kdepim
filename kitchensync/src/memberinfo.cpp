/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "memberinfo.h"

#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>

MemberInfo::MemberInfo( const QSync::Member &member )
  : mMember( member )
{
}

QPixmap MemberInfo::smallIcon() const
{
  return KGlobal::iconLoader()->loadIcon( iconName(), KIcon::Small );
}

QPixmap MemberInfo::desktopIcon() const
{
  return KGlobal::iconLoader()->loadIcon( iconName(), KIcon::Desktop );
}

QString MemberInfo::iconName() const
{
  return pluginIconName( mMember.pluginName() );
}

QString MemberInfo::name() const
{
  static QMap<QString, QString> nameMap;
  if ( nameMap.isEmpty() ) {
    nameMap.insert( "file-sync", i18n( "File" ) );
    nameMap.insert( "palm-sync", i18n( "Palm" ) );
    nameMap.insert( "kdepim-sync", i18n( "KDE PIM" ) );
    nameMap.insert( "kio-sync", i18n( "Remote File" ) );
    nameMap.insert( "irmc-sync", i18n( "Mobile Phone" ) );
    nameMap.insert( "evo2-sync", i18n( "Evolution" ) );
    nameMap.insert( "opie-sync", i18n( "Handheld" ) );
    nameMap.insert( "ldap-sync", i18n( "LDAP" ) );
    nameMap.insert( "syncml-obex-client", i18n( "Mobile Phone" ) );
    nameMap.insert( "syncml-http-server", i18n( "Mobile Phone" ) );
    nameMap.insert( "moto-sync", i18n( "Mobile Phone" ) );
    nameMap.insert( "gnokii-sync", i18n( "Mobile Phone" ) );
    nameMap.insert( "google-calendar", i18n( "Google Calendar" ) );
    nameMap.insert( "gpe-sync", i18n( "Handheld" ) );
    nameMap.insert( "sunbird-sync", i18n( "Sunbird Calendar" ) );
    nameMap.insert( "jescs-sync", i18n( "Java Enterprise System Calendar" ) );
    nameMap.insert( "synce-plugin", i18n( "WinCE Devices" ) );
  }

  if ( mMember.name().isEmpty() )
    return nameMap[ mMember.pluginName() ] + " (" + QString::number( mMember.id() ) + ") ";
  else
    return mMember.name();
}

QString MemberInfo::pluginIconName( const QString &pluginName )
{
  if ( pluginName == "file-sync" ) return "folder";
  if ( pluginName == "palm-sync" ) return "pda_black";
  if ( pluginName == "kdepim-sync" ) return "kontact";
  if ( pluginName == "kio-sync" ) return "network";
  if ( pluginName == "irmc-sync" ) return "mobile_phone";
  if ( pluginName == "evo2-sync" ) return "evolution";
  if ( pluginName == "opie-sync" ) return "pda_blue";
  if ( pluginName == "synce-plugin" ) return "pda_blue";
  if ( pluginName == "ldap-sync" ) return "contents2";
  if ( pluginName == "syncml-obex-client" ) return "mobile_phone";
  if ( pluginName == "syncml-http-server" ) return "pda_blue";
  if ( pluginName == "moto-sync" ) return "mobile_phone";
  if ( pluginName == "gnokii-sync" ) return "mobile_phone";
  if ( pluginName == "google-calendar" ) return "www";
  if ( pluginName == "gpe-sync" ) return "pda_blue";
  if ( pluginName == "sunbird-sync" ) return "www";
  if ( pluginName == "jescs-sync" ) return "www";


  return QString::null;
}
