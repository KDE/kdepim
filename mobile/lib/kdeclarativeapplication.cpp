/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "kdeclarativeapplication.h"

#include <kglobalsettings.h>
#include <QFont>

KDeclarativeApplication::KDeclarativeApplication()
{
#ifndef Q_WS_MAEMO_5
  // make it look more like on the actual device when testing on the desktop
  QFont f = font();
  f.setPointSize( 16 );
  setFont( f );
#endif

  // start with the oxygen palette (which is not necessarily the default on all platforms)
  QPalette pal = KGlobalSettings::createApplicationPalette( KGlobal::config() );

  // background comes from QML
  pal.setColor( QPalette::Window, QColor( 0, 0, 0, 0 ) );

  // FIXME: actually makes things worse with the Maemo5 style which completely ignores our palette apparently
//  setPalette( pal );
}

#include "kdeclarativeapplication.moc"
