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
#include <KActionCollection>
#include <KToggleAction>
#include <KLocale>
#include <KNS3/DownloadDialog>
#include <KActionMenu>

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
          actionGroup(0),
          menu(0),
          actionCollection(ac),
          q(qq)
    {
        watch = new KDirWatch( q );

        downloadThemesAction = new KAction(i18n("Download new themes..."), q);
        if (actionCollection)
            actionCollection->addAction( "download_header_themes", downloadThemesAction );
        separatorAction = new QAction(q);
        separatorAction->setSeparator(true);

        q->connect(downloadThemesAction, SIGNAL(triggered(bool)), q, SLOT(slotDownloadHeaderThemes()) );
        q->connect( watch, SIGNAL(dirty(QString)), SLOT(directoryChanged()) );
    }

    ~Private()
    {
        Q_FOREACH ( KToggleAction *action, themesActionList ) {
            if (actionGroup)
                actionGroup->removeAction(action);
            if (actionCollection)
                actionCollection->removeAction( action );
            delete action;
        }
        themesActionList.clear();
        themes.clear();
        if (downloadThemesDialog)
            delete downloadThemesDialog.data();
    }

    void slotDownloadHeaderThemes()
    {
        if (!downloadThemesDialog) {
            downloadThemesDialog = new KNS3::DownloadDialog(QLatin1String("header_themes.knsrc"));
        }
        downloadThemesDialog.data()->show();
    }

    void directoryChanged()
    {
        updateActionList();
        Q_EMIT q->updateThemes();
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
            const QString dirName = dirIt.fileName();
            const GrantleeTheme theme = loadTheme( dirIt.filePath(), dirName );
            themes.insert( dirName, theme );
            qDebug()<<" theme.name()"<<theme.name();
        }

        Q_EMIT q->themesChanged();
        watch->addDir( themesPath );
        watch->startScan();
    }

    GrantleeTheme loadTheme(const QString &themePath, const QString &dirName )
    {
        const QString themeInfoFile = themePath + QDir::separator() + QString::fromLatin1( "header.desktop" );
        KConfig config( themeInfoFile );
        KConfigGroup group( &config, QLatin1String( "Desktop Entry" ) );

        GrantleeTheme theme;
        theme.setDirName(dirName);
        theme.setName( group.readEntry( "Name", QString() ) );
        theme.setDescription( group.readEntry( "Description", QString() ) );
        theme.setFilename( group.readEntry( "FileName" , QString() ) );
        theme.setDisplayExtraHeaders( group.readEntry( "DisplayExtraHeaders", QStringList() ) );
        return theme;
    }

    void updateActionList()
    {
        if (!actionGroup || !menu)
            return;
        setThemesPath( themesPath );
        QString themeActivated;
        Q_FOREACH ( KToggleAction *action, themesActionList ) {
            if (action->isChecked())
                themeActivated = action->data().toString();
            actionGroup->removeAction(action);
            if (actionCollection)
                actionCollection->removeAction( action );
            menu->removeAction(action);
            delete action;
        }
        menu->removeAction(separatorAction);
        menu->removeAction(downloadThemesAction);
        themesActionList.clear();


        QMapIterator<QString, GrantleeTheme> i(themes);
        while (i.hasNext()) {
            i.next();
            GrantleeTheme theme = i.value();
            KToggleAction *act = new KToggleAction(theme.name(),q);
            act->setToolTip(theme.description());
            act->setData(theme.dirName());
            if (theme.dirName() == themeActivated)
                act->setChecked(true);
            themesActionList.append(act);
            actionGroup->addAction(act);
            menu->addAction(act);
            q->connect(act, SIGNAL(triggered(bool)), q, SLOT(slotThemeSelected()));
        }
        menu->addAction(separatorAction);
        menu->addAction(downloadThemesAction);
    }

    void slotThemeSelected()
    {
        if (q->sender() ) {
            KToggleAction *act = dynamic_cast<KToggleAction *>(q->sender());
            if (act) {
                GlobalSettings::self()->setGrantleeThemeName( act->data().toString() );
                GlobalSettings::self()->writeConfig();
            }
            Q_EMIT q->grantleeThemeSelected();
        }
    }

    KToggleAction *actionForHeaderStyle()
    {
        const QString themeName = GlobalSettings::self()->grantleeThemeName();
        if (themeName.isEmpty())
            return 0;
        Q_FOREACH(KToggleAction *act, themesActionList) {
            if (act->data().toString() == themeName) {
                return static_cast<KToggleAction*>(act);
            }
        }
        return 0;
    }


    QString themesPath;
    QMap<QString, GrantleeTheme> themes;
    QList<KToggleAction*> themesActionList;
    KDirWatch *watch;
    QActionGroup *actionGroup;
    KActionMenu *menu;
    KActionCollection *actionCollection;
    QAction *separatorAction;
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

void GrantleeThemeManager::setActionGroup( QActionGroup *actionGroup )
{
    if (d->actionGroup != actionGroup) {
        d->actionGroup = actionGroup;
        d->updateActionList();
    }
}

KToggleAction *GrantleeThemeManager::actionForHeaderStyle()
{
    return d->actionForHeaderStyle();
}

void GrantleeThemeManager::setHeaderMenu(KActionMenu *menu)
{
    if (d->menu != menu) {
        d->menu = menu;
        d->updateActionList();
    }
}

QStringList GrantleeThemeManager::displayExtraHeader(const QString &themename) const
{
    QMapIterator<QString, GrantleeTheme> i(d->themes);
    while (i.hasNext()) {
        i.next();
        if (i.value().dirName() == themename) {
            return i.value().displayExtraHeaders();
        }
    }
    return QStringList();
}

GrantleeTheme GrantleeThemeManager::theme(const QString &themeName)
{
    if (d->themes.contains(themeName)) {
        return d->themes.value(themeName);
    }
    return GrantleeTheme();
}


#include "grantleethememanager.moc"
