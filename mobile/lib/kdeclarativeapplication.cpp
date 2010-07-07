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

  QPalette p;
  p.setColor( QPalette::Window,          QColor( 0,     0,   0 ) );
  p.setColor( QPalette::WindowText,      QColor( 255, 255, 255 ) );
  p.setColor( QPalette::Base,            QColor( 255, 255, 255 ) );
  p.setColor( QPalette::AlternateBase,   QColor( 239, 239, 239 ) );
  p.setColor( QPalette::Text,            QColor(   0,   0,   0 ) );
  p.setColor( QPalette::Button,          QColor(   0,   0,   0 ) );
  p.setColor( QPalette::ButtonText,      QColor( 255, 255, 255 ) );
  p.setColor( QPalette::BrightText,      QColor( 255, 255, 255 ) );
  p.setColor( QPalette::Light,           QColor(   0,   0,   0 ) );
  p.setColor( QPalette::Midlight,        QColor( 203, 199, 196 ) );
  p.setColor( QPalette::Dark,            QColor(   0,   0,   0 ) );
  p.setColor( QPalette::Mid,             QColor( 184, 181, 178 ) );
  p.setColor( QPalette::Shadow,          QColor(   0,   0,   0 ) );
  p.setColor( QPalette::Highlight,       QColor(  55, 180, 252 ) );
  p.setColor( QPalette::HighlightedText, QColor(   0,  16,  26 ) );
  p.setColor( QPalette::Link,            QColor(   0,   0, 255 ) );
  p.setColor( QPalette::LinkVisited,     QColor( 255,   0, 255 ) );

  p.setColor( QPalette::Disabled, QPalette::WindowText,      QColor( 127, 127, 127 ) );
  p.setColor( QPalette::Disabled, QPalette::Text,            QColor( 127, 127, 127 ) );
  p.setColor( QPalette::Disabled, QPalette::ButtonText,      QColor( 127, 127, 127 ) );
  p.setColor( QPalette::Disabled, QPalette::Highlight,       QColor( 252, 252, 252 ) );
  p.setColor( QPalette::Disabled, QPalette::HighlightedText, QColor(  26,  26,  26 ) );

  setPalette( p );

  setStyle( "plastique" ); // to avoid oxygen artefacts
#endif

  // start with the oxygen palette (which is not necessarily the default on all platforms)
  QPalette pal = KGlobalSettings::createApplicationPalette( KGlobal::config() );

  // background comes from QML
  pal.setColor( QPalette::Window, QColor( 0, 0, 0, 0 ) );

  // FIXME: actually makes things worse with the Maemo5 style which completely ignores our palette apparently
//  setPalette( pal );
}

#include "kdeclarativeapplication.moc"
