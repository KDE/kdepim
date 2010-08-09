/*
  This file is part of the Grantlee template system.

  Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either version
  2 of the Licence, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "theme.h"

#include <QtCore/QSharedData>

using namespace Grantlee;

class Theme::Private : public QSharedData
{
  public:
    Private()
    {
    }

    Private( const Private &other )
      : QSharedData( other )
    {
      mIdentifier = other.mIdentifier;
      mName = other.mName;
      mDescription = other.mDescription;
      mDefaultTemplatePath = other.mDefaultTemplatePath;
      mBasePath = other.mBasePath;
    }

    QString mIdentifier;
    QString mName;
    QString mDescription;
    QString mDefaultTemplatePath;
    QString mBasePath;
};

Theme::Theme()
  : d( new Private )
{
}

Theme::~Theme()
{
}

Theme::Theme( const Theme &other )
  : d( other.d )
{
}

Theme& Theme::operator=( const Theme &other )
{
  if ( this != &other )
    d = other.d;

  return *this;
}

bool Theme::operator==( const Theme &other ) const
{
  return (d->mIdentifier == other.d->mIdentifier);
}

bool Theme::isValid() const
{
  return !d->mIdentifier.isEmpty();
}

void Theme::setIdentifier( const QString &identifier )
{
  d->mIdentifier = identifier;
}

QString Theme::identifier() const
{
  return d->mIdentifier;
}

void Theme::setName( const QString &name )
{
  d->mName = name;
}

QString Theme::name() const
{
  return d->mName;
}

void Theme::setDescription( const QString &description )
{
  d->mDescription = description;
}

QString Theme::description() const
{
  return d->mDescription;
}

void Theme::setDefaultTemplatePath( const QString &path )
{
  d->mDefaultTemplatePath = path;
}

QString Theme::defaultTemplatePath() const
{
  return d->mDefaultTemplatePath;
}

void Theme::setBasePath( const QString &path )
{
  d->mBasePath = path;
}

QString Theme::basePath() const
{
  return d->mBasePath;
}
