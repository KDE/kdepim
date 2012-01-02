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

#ifndef GRANTLEE_THEME_H
#define GRANTLEE_THEME_H

#include <QtCore/QSharedDataPointer>
#include <QtCore/QStringList>

namespace Grantlee
{

/**
 *
 */
class Theme
{
  public:
    /**
     * Describes a list of theme objects.
     */
    typedef QList<Theme> List;

    /**
     * Creates new theme.
     */
    Theme();

    /**
     * Destroys the theme.
     */
    ~Theme();

    /**
     * Creates a new theme from the @p other theme.
     */
    Theme( const Theme &other );

    /**
     * Assigns the theme from the @p other theme.
     */
    Theme& operator=( const Theme &other );

    /**
     * Returns whether this theme equals the @p other theme.
     */
    bool operator==( const Theme &other ) const;

    /**
     * Returns whether this theme is valid.
     */
    bool isValid() const;

    /**
     * Sets the @p identifier of the theme.
     */
    void setIdentifier( const QString &identifier );

    /**
     * Returns the identifier of the theme.
     */
    QString identifier() const;

    /**
     * Sets the i18n'ed @p name of the theme.
     */
    void setName( const QString &name );

    /**
     * Returns the i18n'ed name of the theme.
     */
    QString name() const;

    /**
     * Set i18n'ed description of the theme.
     */
    void setDescription( const QString &description );

    /**
     * Returns the i18n'ed description of the theme.
     */
    QString description() const;

    /**
     * Sets the @p path of the default template file.
     */
    void setDefaultTemplatePath( const QString &path );

    /**
     * Returns the path of the default template file.
     */
    QString defaultTemplatePath() const;

    /**
     * Sets the base @p path of the template.
     */
    void setBasePath( const QString &path );

    /**
     * Returns the base path of the template.
     */
    QString basePath() const;

  private:
    //@cond PRIVATE
    class Private;
    QSharedDataPointer<Private> d;
    //@endcond
};

}

#endif
