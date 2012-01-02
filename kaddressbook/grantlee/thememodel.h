/*
  This file is part of the Grantlee template system.

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

#ifndef GRANTLEE_THEMEMODEL_H
#define GRANTLEE_THEMEMODEL_H

#include <QtCore/QAbstractListModel>

namespace Grantlee
{

/**
 *
 */
class ThemeModel : public QAbstractListModel
{
  Q_OBJECT

  public:
    /**
     * Describes the additional roles the theme model provides.
     */
    enum Role
    {
      IdentifierRole = Qt::UserRole + 1,
      BasePathRole
    };

    /**
     * Creates a new theme model.
     *
     * @param themesPath The path to the top-level directory of the theme directories.
     * @param parent The parent object.
     */
    ThemeModel( const QString &themesPath = QString(), QObject *parent = 0 );

    /**
     * Destroys the theme model.
     */
    ~ThemeModel();

    /**
     * Sets the @p path of the top-level directory of the theme directories.
     */
    void setThemesPath( const QString &path );

    /**
     * Returns the path of the top-level directory of the theme directories.
     */
    QString themesPath() const;

    /**
     * @reimpl
     */
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    /**
     * @reimpl
     */
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;
    //@endcond
};

}

#endif
