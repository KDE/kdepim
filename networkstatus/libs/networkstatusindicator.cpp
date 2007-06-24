/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library.  If not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this library
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <QLabel>
#include <QVBoxLayout>
#include <KIconLoader>
#include <KLocale>

#include "kconnectionmanager.h"

#include "networkstatusindicator.h"

StatusBarNetworkStatusIndicator::StatusBarNetworkStatusIndicator(
    QWidget * parent) : QWidget( parent)
{
  //setMargin( 2 );
  QVBoxLayout * layout = new QVBoxLayout( this );
  layout->setSpacing( 1 );
  QLabel * label = new QLabel( this );
  label->setPixmap( SmallIcon("connect-no") );
  label->setToolTip( i18n( "The desktop is offline" ) );
  layout->addWidget( label );
  setLayout( layout );

  connect( KConnectionManager::self(), SIGNAL( statusChanged( NetworkStatus::Status ) ),
      SLOT( networkStatusChanged( NetworkStatus::Status) ) );

}

StatusBarNetworkStatusIndicator::~StatusBarNetworkStatusIndicator()
{
}

void StatusBarNetworkStatusIndicator::init()
{
  networkStatusChanged( KConnectionManager::self()->status());
}

void StatusBarNetworkStatusIndicator::networkStatusChanged( NetworkStatus::Status status )
{
  if ( status == NetworkStatus::Online || status == NetworkStatus::NoNetworks ) {
    hide();
  } else {
    show();
  }
}

#include "networkstatusindicator.moc"
