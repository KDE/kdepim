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

#ifndef GRANTLEE_THEMECOMBOBOX_H
#define GRANTLEE_THEMECOMBOBOX_H

#include <KComboBox>

namespace Grantlee {

/**
 *
 */
class ThemeComboBox : public KComboBox
{
  Q_OBJECT

  public:
    /**
     * Creates a new theme combo box.
     *
     * @param themesPath The path to the top-level directory of the theme directories.
     * @param parent The parent object.
     */
    explicit ThemeComboBox( const QString &themesPath = QString(), QWidget *parent = 0 );

    /**
     * Destroys the theme combo box.
     */
    ~ThemeComboBox();

    /**
     * Sets the @p path of the top-level directory of the theme directories.
     */
    void setThemesPath( const QString &path );

    /**
     * Returns the path of the top-level directory of the theme directories.
     */
    QString themesPath() const;

    /**
     * Sets the current theme @p identifier.
     */
    void setCurrentTheme( const QString &identifier );

    /**
     * Returns the current theme identifier.
     */
    QString currentTheme() const;

    /**
     * Returns the base path of the current theme.
     */
    QString currentBasePath() const;

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
