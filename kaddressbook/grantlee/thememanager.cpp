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

#include "thememanager.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdirwatch.h>

#include <QtCore/QDebug>
#include <QtCore/QDirIterator>

using namespace Grantlee;

class ThemeManager::Private
{
  public:
    Private( const QString &themesPath, ThemeManager *qq )
      : q( qq )
    {
      setThemesPath( themesPath );

      mDirWatch = new KDirWatch( q );
      q->connect( mDirWatch, SIGNAL(dirty(QString)),
                  SLOT(directoryChanged()) );
    }

    ~Private()
    {
      delete mDirWatch;
    }

    void setThemesPath( const QString &themesPath )
    {
      if ( !mThemesPath.isEmpty() ) {
        mDirWatch->stopScan();
        mDirWatch->removeDir( mThemesPath );
      }

      // clear all previous theme information
      QMutableHashIterator<QString, Theme> it( mThemes );
      while ( it.hasNext() ) {
        it.next();

        const Theme theme = it.value();
        it.remove();

        //emit q->themeRemoved( theme );
      }

      if ( themesPath.isEmpty() )
        return;

      mThemesPath = themesPath;

      /**
       * Let's assume a theme directory structure like the following:
       *
       * .../my/path/to/themes/
       * .../my/path/to/themes/clear/
       * .../my/path/to/themes/tiny/
       * .../my/path/to/themes/fancy/
       *
       * then mThemesPath points to .../my/path/to/themes/ and we have the themes
       * 'clear', 'tiny' and 'fancy'
       */
      QDirIterator dirIt( mThemesPath, QStringList(), QDir::AllDirs | QDir::NoDotAndDotDot );
      while ( dirIt.hasNext() ) {
        dirIt.next();

        const QString identifier = dirIt.fileName();
        const Theme theme = parseTheme( dirIt.fileName(), dirIt.filePath() );

        mThemes.insert( identifier, theme );
        //emit q->themeAdded( theme );
      }

      mDirWatch->addDir( mThemesPath );
      mDirWatch->startScan();
    }

    Theme parseTheme( const QString &identifier, const QString &themePath )
    {
      const QString themeInfoFile = themePath + QDir::separator() + QString::fromLatin1( "theme-%1.desktop" ).arg( identifier );
      KConfig config( themeInfoFile );
      KConfigGroup group( &config, QLatin1String( "Desktop Entry" ) );

      Theme theme;
      theme.setIdentifier( identifier );
      theme.setName( group.readEntry( "Name", QString() ) );
      theme.setDescription( group.readEntry( "Description", QString() ) );
      theme.setBasePath( themePath );
      theme.setDefaultTemplatePath( themePath +
                                    QDir::separator() +
                                    group.readEntry( "FileName", QString() ) );

      return theme;
    }

    void directoryChanged()
    {
      setThemesPath( mThemesPath );
    }

    ThemeManager *q;
    QString mThemesPath;
    QHash<QString, Theme> mThemes;
    KDirWatch *mDirWatch;
};

ThemeManager::ThemeManager( const QString &themesPath, QObject *parent )
  : QObject( parent ), d( new Private( themesPath, this ) )
{
}

ThemeManager::~ThemeManager()
{
  delete d;
}

void ThemeManager::setThemesPath( const QString &path )
{
  d->setThemesPath( path );
}

QString ThemeManager::themesPath() const
{
  return d->mThemesPath;
}

Theme::List ThemeManager::themes() const
{
  return d->mThemes.values();
}

#include "thememanager.moc"
