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
#include "globalsettings.h"

#include <KDirWatch>
#include <KConfigGroup>
#include <KConfig>
#include <KXMLGUIClient>
#include <KActionCollection>
#include <KToggleAction>
#include <KLocale>
#include <KNS3/DownloadDialog>

#include <QDir>
#include <QAction>
#include <QDirIterator>
#include <QActionGroup>


using namespace MessageViewer;

class GrantleeThemeManager::Private
{
public:
    Private(KActionCollection *ac, const QString &path, GrantleeThemeManager *qq)
        : themesPath(path),
          guiClient(0),
          actionGroup(0),
          actionCollection(ac),
          q(qq)
    {
        watch = new KDirWatch( q );
        q->connect( watch, SIGNAL(dirty(QString)), SLOT(directoryChanged()) );
        downloadThemesAction = new KAction(i18n("Download new themes..."), q);
        actionCollection->addAction( "download_header_themes", downloadThemesAction );
        connect(downloadThemesAction, SIGNAL(triggered(bool)), q, SLOT(slotDownloadHeaderThemes()) );
    }

    void slotDownloadHeaderThemes()
    {
        if (!downloadThemesDialog) {
            downloadThemesDialog = new KNS3::DownloadDialog(QLatin1String("header_themes.knsrc"));
            connect(downloadThemesDialog.data(), SIGNAL(accepted()), q, SLOT(slotNewStuffFinished()));
        }
        downloadThemesDialog.data()->show();
    }

    void slotNewStuffFinished()
    {
        if (downloadThemesDialog) {
        //TODO
        }
    }

    void directoryChanged()
    {
        setThemesPath( themesPath );
        updateActionList();
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

            themes.insert( theme.name(), theme );
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

    void updateActionList()
    {
        if (!actionGroup || !guiClient)
            return;

        guiClient->unplugActionList(QLatin1String("theme_action_list"));

        Q_FOREACH ( QAction *action, themesActionList ) {
            actionGroup->removeAction(action);
            actionCollection->removeAction( action );
        }
        themesActionList.clear();

        QMapIterator<QString, GrantleeTheme> i(themes);
        while (i.hasNext()) {
            i.next();
            KToggleAction *act = new KToggleAction(i.value().name(),q);
            themesActionList.append(act);
            actionGroup->addAction(act);
            q->connect(act, SIGNAL(triggered(bool)), q, SLOT(slotThemeSelected()));
        }
        guiClient->plugActionList(QLatin1String("theme_action_list"), themesActionList);
    }

    void slotThemeSelected()
    {
        if (q->sender() ) {
            KToggleAction *act = dynamic_cast<KToggleAction *>(q->sender());
            if (act) {
                GlobalSettings::self()->setGrantleeThemeName( act->text() );
            }
            Q_EMIT q->grantleeThemeSelected();
        }
    }

    KToggleAction *actionForHeaderStyle()
    {
        const QString themeName = GlobalSettings::self()->grantleeThemeName();
        if (themeName.isEmpty())
            return 0;
        Q_FOREACH(QAction *act, themesActionList) {
            if (act->text() == themeName) {
                return static_cast<KToggleAction*>(act);
            }
        }
        return 0;
    }


    QString themesPath;
    QMap<QString, GrantleeTheme> themes;
    QList<QAction*> themesActionList;
    KDirWatch *watch;
    KXMLGUIClient *guiClient;
    QActionGroup *actionGroup;
    KActionCollection *actionCollection;
    KAction *downloadThemesAction;
    QWeakPointer<KNS3::DownloadDialog> downloadThemesDialog;
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
    d->directoryChanged();
}

void GrantleeThemeManager::setActionGroup( QActionGroup *actionGroup )
{
    d->actionGroup = actionGroup;
    d->directoryChanged();
}

KToggleAction *GrantleeThemeManager::actionForHeaderStyle()
{
    return d->actionForHeaderStyle();
}

void GrantleeThemeManager::activateTheme(const QString &themeName)
{
    //TODO
}

#include "grantleethememanager.moc"
