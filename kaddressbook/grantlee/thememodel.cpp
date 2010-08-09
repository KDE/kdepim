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

#include "thememodel.h"

#include "thememanager.h"

using namespace Grantlee;

class ThemeModel::Private
{
  public:
    Private()
      : mManager( new ThemeManager )
    {
    }

    ~Private()
    {
      delete mManager;
    }

    ThemeManager *mManager;
    Theme::List mThemes;
};

ThemeModel::ThemeModel( const QString &themesPath, QObject *parent )
  : QAbstractListModel( parent ), d( new Private )
{
  d->mManager->setThemesPath( themesPath );
  d->mThemes = d->mManager->themes();
}

ThemeModel::~ThemeModel()
{
}

void ThemeModel::setThemesPath( const QString &path )
{
  modelAboutToBeReset();
  d->mManager->setThemesPath( path );
  d->mThemes = d->mManager->themes();
  modelReset();
}

QString ThemeModel::themesPath() const
{
  return d->mManager->themesPath();
}

int ThemeModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return d->mThemes.count();
}

QVariant ThemeModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.column() < 0 ||
       index.row() >= d->mThemes.count() || index.column() > 1 )
    return QVariant();

  const Theme theme = d->mThemes.at( index.row() );
  switch ( role ) {
    case Qt::DisplayRole:
      return theme.name();
    case IdentifierRole:
      return theme.identifier();
    case BasePathRole:
      return theme.basePath();
  }

  return QVariant();
}
