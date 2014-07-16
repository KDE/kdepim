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

#include "guistatemanager.h"

#include <QtCore/QDebug>

static void printStack( const QStack<int> &stack )
{
  QString output = QLatin1String( "UI-State-Stack: " );
  for ( int i = 0; i < stack.count(); ++i )
    output += QLatin1Char( ':' ) + QString::number( stack.at( i ) );

  qDebug( "%s", qPrintable( output ) );
}

class GuiStateManager::Private
{
  public:
    Private()
    {
      mGuiStates.push( GuiStateManager::HomeScreenState );
    }

    QStack<int> mGuiStates;
};

GuiStateManager::GuiStateManager( QObject *parent )
  : QObject( parent ), d( new Private )
{
}

GuiStateManager::~GuiStateManager()
{
  delete d;
}

void GuiStateManager::switchState( int state )
{
  Q_ASSERT( !d->mGuiStates.isEmpty() );

  const int previousState = d->mGuiStates.pop();
  d->mGuiStates.push( state );

  printStack( d->mGuiStates );
  emitChangedSignal();

  emit guiStateChanged( previousState, state );
}

void GuiStateManager::pushState( int state )
{
  const int previousState = (d->mGuiStates.isEmpty() ? -1 : d->mGuiStates.top());

  d->mGuiStates.push( state );

  printStack( d->mGuiStates );
  emitChangedSignal();

  emit guiStateChanged( previousState, state );
}

void GuiStateManager::pushUniqueState( int state )
{
  const int previousState = (d->mGuiStates.isEmpty() ? -1 : d->mGuiStates.top());

  if ( d->mGuiStates.isEmpty() ) {
    d->mGuiStates.push( state );
  } else {
    if ( d->mGuiStates.top() != state )
      d->mGuiStates.push( state );
  }

  printStack( d->mGuiStates );
  emitChangedSignal();

  emit guiStateChanged( previousState, state );
}

void GuiStateManager::popState()
{
  const int previousState = d->mGuiStates.pop();
  Q_ASSERT( !d->mGuiStates.isEmpty() );

  printStack( d->mGuiStates );
  emitChangedSignal();

  emit guiStateChanged( previousState, d->mGuiStates.top() );
}

int GuiStateManager::currentState() const
{
  Q_ASSERT( !d->mGuiStates.isEmpty() );

  return d->mGuiStates.top();
}

bool GuiStateManager::inHomeScreenState() const
{
  return (currentState() == HomeScreenState);
}

bool GuiStateManager::inAccountScreenState() const
{
  return (currentState() == AccountScreenState);
}

bool GuiStateManager::inSingleFolderScreenState() const
{
  return (currentState() == SingleFolderScreenState);
}

bool GuiStateManager::inMultipleFolderScreenState() const
{
  return (currentState() == MultipleFolderScreenState);
}

bool GuiStateManager::inBulkActionScreenState() const
{
  return (currentState() == BulkActionScreenState);
}

bool GuiStateManager::inMultipleFolderSelectionScreenState() const
{
  return (currentState() == MultipleFolderSelectionScreenState);
}

bool GuiStateManager::inViewSingleItemState() const
{
  return (currentState() == ViewSingleItemState);
}

bool GuiStateManager::inSearchScreenState() const
{
  return (currentState() == SearchScreenState);
}

bool GuiStateManager::inSearchResultScreenState() const
{
  return (currentState() == SearchResultScreenState);
}

bool GuiStateManager::inConfigScreenState() const
{
  return (currentState() == ConfigScreenState);
}

void GuiStateManager::emitChangedSignal()
{
  emit guiStateChanged();
}

