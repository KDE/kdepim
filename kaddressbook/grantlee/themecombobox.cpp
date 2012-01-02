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

#include "themecombobox.h"

#include "thememodel.h"

using namespace Grantlee;

class ThemeComboBox::Private
{
  public:
    Private( const QString &themesPath )
    {
      mModel = new ThemeModel( themesPath );
    }

    ~Private()
    {
      delete mModel;
    }

    ThemeModel *mModel;
};

ThemeComboBox::ThemeComboBox( const QString &themesPath, QWidget *parent )
  : KComboBox( parent ), d( new Private( themesPath ) )
{
  setModel( d->mModel );
}

ThemeComboBox::~ThemeComboBox()
{
  delete d;
}

void ThemeComboBox::setThemesPath( const QString &path )
{
  d->mModel->setThemesPath( path );
}

QString ThemeComboBox::themesPath() const
{
  return d->mModel->themesPath();
}

void ThemeComboBox::setCurrentTheme( const QString &identifier )
{
  for ( int row = 0; row < d->mModel->rowCount(); ++row ) {
    const QModelIndex index = d->mModel->index( row, 0 );
    if ( index.data( ThemeModel::IdentifierRole ).toString() == identifier ) {
      setCurrentIndex( row );
      break;
    }
  }
}

QString ThemeComboBox::currentTheme() const
{
  return d->mModel->index( currentIndex(), 0 ).data( ThemeModel::IdentifierRole ).toString();
}

QString ThemeComboBox::currentBasePath() const
{
  return d->mModel->index( currentIndex(), 0 ).data( ThemeModel::BasePathRole ).toString();
}

#include "themecombobox.moc"
