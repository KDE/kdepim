/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "grantleethememanager.h"

#include <KDirWatch>
#include <KConfigGroup>
#include <KConfig>
#include <KXMLGUIClient>
#include <KActionCollection>

#include <QDir>
#include <QDirIterator>


using namespace MessageViewer;

class GrantleeThemeManager::Private
{
public:
    Private(KActionCollection *ac, const QString &path, GrantleeThemeManager *qq)
        : themesPath(path),
          guiClient(0),
          actionCollection(ac),
          q(qq)
    {
        watch = new KDirWatch( q );
        q->connect( watch, SIGNAL(dirty(QString)), SLOT(directoryChanged()) );
    }

    void directoryChanged()
    {
        setThemesPath( themesPath );
    }

    void setThemesPath(const QString& path)
    {
        if ( !themesPath.isEmpty() ) {
            watch->stopScan();
            watch->removeDir( themesPath );
        }

        // clear all previous theme information
        themes.clear();
        if ( path.isEmpty() ) {
            return;
        }

        themesPath = path;

        QDirIterator dirIt( themesPath, QStringList(), QDir::AllDirs | QDir::NoDotAndDotDot );
        while ( dirIt.hasNext() ) {
            dirIt.next();
            const GrantleeTheme theme = loadTheme( dirIt.filePath() );

            themes.insert( dirIt.fileName(), theme );
        }

        Q_EMIT q->themesChanged();
        watch->addDir( themesPath );
        watch->startScan();
    }

    GrantleeTheme loadTheme(const QString &themePath )
    {
        const QString themeInfoFile = themePath + QDir::separator() + QString::fromLatin1( "header.desktop" );
        KConfig config( themeInfoFile );
        KConfigGroup group( &config, QLatin1String( "Desktop Entry" ) );

        GrantleeTheme theme;
        theme.setName( group.readEntry( "Name", QString() ) );
        theme.setDescription( group.readEntry( "Description", QString() ) );
        theme.setFilename( themePath );
        theme.setDisplayExtraHeaders( group.readEntry( "DisplayExtraHeaders", QStringList() ) );
        return theme;
    }

    QMap<QString, GrantleeTheme> themes;
    QString themesPath;
    KDirWatch *watch;
    KXMLGUIClient *guiClient;
    KActionCollection *actionCollection;
    GrantleeThemeManager *q;
};

GrantleeThemeManager::GrantleeThemeManager(KActionCollection *actionCollection, const QString &path, QObject *parent)
    : QObject(parent), d(new Private(actionCollection, path,this))
{
}

GrantleeThemeManager::~GrantleeThemeManager()
{
    delete d;
}

QMap<QString, GrantleeTheme> GrantleeThemeManager::themes() const
{
    return d->themes;
}

GrantleeTheme GrantleeThemeManager::findTheme( const QString &themeName) const
{
    return d->themes.find(themeName).value();
}

void GrantleeThemeManager::setXmlGuiClient( KXMLGUIClient *guiClient )
{
    d->guiClient = guiClient;
}

#include "grantleethememanager.moc"
