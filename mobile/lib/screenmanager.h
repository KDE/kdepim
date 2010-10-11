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

#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QStack>

class ScreenManager : public QObject
{
  Q_OBJECT

  Q_PROPERTY( bool isHomeScreenVisible READ isHomeScreenVisible NOTIFY screenVisibilityChanged )
  Q_PROPERTY( bool isAccountScreenVisible READ isAccountScreenVisible NOTIFY screenVisibilityChanged )
  Q_PROPERTY( bool isSingleFolderScreenVisible READ isSingleFolderScreenVisible NOTIFY screenVisibilityChanged )
  Q_PROPERTY( bool isMultiFolderScreenVisible READ isMultiFolderScreenVisible NOTIFY screenVisibilityChanged )

  public:
    /**
     * Describes the state of the visible screens.
     */
    enum ScreenState {
      HomeScreen = 0,
      AccountScreen = 1,
      SingleFolderScreen = 2,
      MultiFolderScreen = 4
    };

    ScreenManager( QObject *parent = 0 );
    ~ScreenManager();

  public Q_SLOTS:
    void switchScreen( ScreenState state );
    void pushScreen( ScreenState state );
    void popScreen();

    /**
     * Returns whether the home screen is currently visible.
     */
    bool isHomeScreenVisible() const;

    /**
     * Returns whether the account screen is currently visible.
     */
    bool isAccountScreenVisible() const;

    /**
     * Returns whether the single folder screen is currently visible.
     */
    bool isSingleFolderScreenVisible() const;

    /**
     * Returns whether the multiple folder screen is currently visible.
     */
    bool isMultiFolderScreenVisible() const;

  Q_SIGNALS:
    void screenVisibilityChanged();

  private:
    QStack<ScreenState> mScreenStates;
};

#endif // SCREENMANAGER_H
