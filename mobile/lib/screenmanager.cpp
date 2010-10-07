/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

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

#include "screenmanager.h"

static void printCurrentScreen( ScreenManager::ScreenState state )
{
  switch ( state ) {
    case ScreenManager::HomeScreen: qDebug("---> ScreenManager: HomeScreen"); break;
    case ScreenManager::AccountScreen: qDebug("---> ScreenManager: AccountScreen"); break;
    case ScreenManager::SingleFolderScreen: qDebug("---> ScreenManager: SingleFolderScreen"); break;
    case ScreenManager::MultiFolderScreen: qDebug("---> ScreenManager: MultiFolderHomeScreen"); break;
    default: qDebug("---> ScreenManager: Unknown!"); break;
  }
}

ScreenManager::ScreenManager( QObject *parent )
  : QObject( parent )
{
  mScreenStates.push( HomeScreen );
  printCurrentScreen( mScreenStates.top() );
}

ScreenManager::~ScreenManager()
{
}

void ScreenManager::switchScreen( ScreenState state )
{
  Q_ASSERT( !mScreenStates.isEmpty() );

  mScreenStates.pop();
  mScreenStates.push( state );

  printCurrentScreen( mScreenStates.top() );
  emit screenVisibilityChanged();
}

void ScreenManager::pushScreen( ScreenState state )
{
  mScreenStates.push( state );
  printCurrentScreen( mScreenStates.top() );
  emit screenVisibilityChanged();
}

void ScreenManager::popScreen()
{
  mScreenStates.pop();
  Q_ASSERT( !mScreenStates.isEmpty() );

  printCurrentScreen( mScreenStates.top() );
  emit screenVisibilityChanged();
}

bool ScreenManager::isHomeScreenVisible() const
{
  return (mScreenStates.top() == HomeScreen);
}

bool ScreenManager::isAccountScreenVisible() const
{
  return (mScreenStates.top() == AccountScreen);
}

bool ScreenManager::isSingleFolderScreenVisible() const
{
  return (mScreenStates.top() == SingleFolderScreen);
}

bool ScreenManager::isMultiFolderScreenVisible() const
{
  return (mScreenStates.top() == MultiFolderScreen);
}

#include "screenmanager.moc"
