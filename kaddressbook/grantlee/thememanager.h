/*
  This file is part of KAddressBook.

  Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either version
  2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GRANTLEE_THEMEMANAGER_H
#define GRANTLEE_THEMEMANAGER_H

#include "theme.h"

#include <QtCore/QObject>

namespace Grantlee {

/**
 *
 */
class ThemeManager : public QObject
{
  Q_OBJECT

  public:
    /**
     * Creates a new theme manager.
     *
     * @param themesPath The path to the top-level directory of the theme directories.
     * @param parent The parent object.
     */
    explicit ThemeManager( const QString &themesPath = QString(), QObject *parent = 0 );

    /**
     * Destroys the theme manager.
     */
    ~ThemeManager();

    /**
     * Sets the @p path of the top-level directory of the theme directories.
     */
    void setThemesPath( const QString &path );

    /**
     * Returns the path of the top-level directory of the theme directories.
     */
    QString themesPath() const;

    /**
     * Returns the list of all available themes.
     */
    Theme::List themes() const;

  Q_SIGNALS:
    /**
     * This signal is emitted whenever a new theme has been added to the themes
     * top-level directory.
     *
     * @param theme The new theme that has been added.
     */
    //void themeAdded( const Theme &theme );

    /**
     * This signal is emitted whenever a theme has been removed from the themes
     * top-level directory.
     *
     * @param theme The theme that has been removed.
     */
    //void themeRemoved( const Theme &theme );

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void directoryChanged() )
    //@endcond
};

}

#endif
