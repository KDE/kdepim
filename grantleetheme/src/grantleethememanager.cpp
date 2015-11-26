/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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
#include "grantleetheme_p.h"

#include <KDirWatch>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KActionCollection>
#include <KToggleAction>
#include <KLocalizedString>
#include <KNS3/DownloadDialog>
#include <KCoreAddons/kaboutdata.h>
#include <KActionMenu>
#include <QDebug>
#include <QAction>
#include <QIcon>

#include <QDir>
#include <QDirIterator>
#include <QActionGroup>
#include <QStandardPaths>

using namespace GrantleeTheme;

class Q_DECL_HIDDEN ThemeManager::Private
{
public:
    Private(const QString &type,
            const QString &desktopFileName,
            KActionCollection *ac,
            const QString &relativePath, ThemeManager *qq)
        : applicationType(type),
          defaultDesktopFileName(desktopFileName),
          actionGroup(0),
          menu(0),
          actionCollection(ac),
          q(qq)
    {
        watch = new KDirWatch(q);
        initThemesDirectories(relativePath);
        downloadThemesAction = new QAction(i18n("Download New Themes..."), q);
        downloadThemesAction->setIcon(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")));
        if (actionCollection) {
            actionCollection->addAction(QStringLiteral("download_header_themes"), downloadThemesAction);
        }
        separatorAction = new QAction(q);
        separatorAction->setSeparator(true);

        q->connect(downloadThemesAction, SIGNAL(triggered(bool)), q, SLOT(slotDownloadHeaderThemes()));
        q->connect(watch, SIGNAL(dirty(QString)), SLOT(directoryChanged()));
        updateThemesPath(true);

        // Migrate the old configuration format that only support mail and addressbook
        // theming to the new generic format
        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        if (config->hasGroup(QStringLiteral("GrantleeTheme"))) {
            const KConfigGroup group = config->group(QStringLiteral("GrantleeTheme"));
            const QString mailTheme = group.readEntry(QStringLiteral("grantleeMailThemeName"));
            const QString addressbookTheme = group.readEntry(QStringLiteral("grantleeAddressBookThemeName"));

            config->group(QStringLiteral("mail")).writeEntry(QStringLiteral("themeName"), mailTheme);
            config->group(QStringLiteral("addressbook")).writeEntry(QStringLiteral("themeName"), addressbookTheme);

            config->deleteGroup(QStringLiteral("GrantleeTheme"));
        }
    }

    ~Private()
    {
        Q_FOREACH (KToggleAction *action, themesActionList) {
            if (actionGroup) {
                actionGroup->removeAction(action);
            }
            if (actionCollection) {
                actionCollection->removeAction(action);
            }
            delete action;
        }
        themesActionList.clear();
        themes.clear();
        if (downloadThemesDialog) {
            delete downloadThemesDialog.data();
        }
    }

    void slotDownloadHeaderThemes()
    {
        if (!downloadThemesDialog) {
            downloadThemesDialog = new KNS3::DownloadDialog(downloadConfigFileName);
        }
        downloadThemesDialog.data()->show();
    }

    void directoryChanged()
    {
        updateThemesPath();
        updateActionList();
        Q_EMIT q->updateThemes();
    }

    void updateThemesPath(bool init = false)
    {
        if (!init) {
            if (!themesDirectories.isEmpty()) {
                Q_FOREACH (const QString &directory, themesDirectories) {
                    watch->removeDir(directory);
                }
            } else {
                return;
            }
        }

        // clear all previous theme information
        themes.clear();

        Q_FOREACH (const QString &directory, themesDirectories) {
            QDirIterator dirIt(directory, QStringList(), QDir::AllDirs | QDir::NoDotAndDotDot);
            QStringList alreadyLoadedThemeName;
            while (dirIt.hasNext()) {
                dirIt.next();
                const QString dirName = dirIt.fileName();
                GrantleeTheme::Theme theme = q->loadTheme(dirIt.filePath(), dirName, defaultDesktopFileName);
                if (theme.isValid()) {
                    QString themeName = theme.name();
                    if (alreadyLoadedThemeName.contains(themeName)) {
                        int i = 2;
                        const QString originalName(theme.name());
                        while (alreadyLoadedThemeName.contains(themeName)) {
                            themeName = originalName + QStringLiteral(" (%1)").arg(i);
                            ++i;
                        }
                        theme.d->name = themeName;
                    }
                    alreadyLoadedThemeName << themeName;
                    themes.insert(dirName, theme);
                    //qDebug()<<" theme.name()"<<theme.name();
                }
            }
            watch->addDir(directory);
        }

        Q_EMIT q->themesChanged();
        watch->startScan();
    }

    void updateActionList()
    {
        if (!actionGroup || !menu) {
            return;
        }
        QString themeActivated;
        Q_FOREACH (KToggleAction *action, themesActionList) {
            if (action->isChecked()) {
                themeActivated = action->data().toString();
            }
            actionGroup->removeAction(action);
            if (actionCollection) {
                actionCollection->removeAction(action);
            }
            menu->removeAction(action);
            delete action;
        }
        menu->removeAction(separatorAction);
        menu->removeAction(downloadThemesAction);
        themesActionList.clear();

        bool themeActivatedFound = false;
        QMapIterator<QString, GrantleeTheme::Theme> i(themes);
        while (i.hasNext()) {
            i.next();
            GrantleeTheme::Theme theme = i.value();
            KToggleAction *act = new KToggleAction(theme.name(), q);
            act->setToolTip(theme.description());
            act->setData(theme.dirName());
            if (theme.dirName() == themeActivated) {
                act->setChecked(true);
                themeActivatedFound = true;
            }
            themesActionList.append(act);
            actionGroup->addAction(act);
            menu->addAction(act);
            q->connect(act, SIGNAL(triggered(bool)), q, SLOT(slotThemeSelected()));
        }
        if (!themeActivatedFound) {
            if (!themesActionList.isEmpty() && !themeActivated.isEmpty()) {
                //Activate first item if we removed theme.
                KToggleAction *act = themesActionList.at(0);
                act->setChecked(true);
                selectTheme(act);
            }
        }
        menu->addAction(separatorAction);
        menu->addAction(downloadThemesAction);
    }

    void selectTheme(KToggleAction *act)
    {
        if (act) {
            KSharedConfig::Ptr config = KSharedConfig::openConfig();
            KConfigGroup group = config->group(applicationType);
            group.writeEntry(QStringLiteral("themeName"), act->data().toString());
            config->sync();
        }
    }

    void slotThemeSelected()
    {
        if (q->sender()) {
            KToggleAction *act = dynamic_cast<KToggleAction *>(q->sender());
            selectTheme(act);
            Q_EMIT q->grantleeThemeSelected();
        }
    }

    KToggleAction *actionForTheme()
    {
        const KSharedConfig::Ptr config = KSharedConfig::openConfig();
        const KConfigGroup group = config->group(applicationType);
        const QString themeName = group.readEntry(QStringLiteral("themeName"), QStringLiteral("default"));

        if (themeName.isEmpty()) {
            return 0;
        }
        Q_FOREACH (KToggleAction *act, themesActionList) {
            if (act->data().toString() == themeName) {
                return static_cast<KToggleAction *>(act);
            }
        }
        return 0;
    }

    void initThemesDirectories(const QString &themesRelativePath)
    {
        if (!themesRelativePath.isEmpty()) {

            themesDirectories = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, themesRelativePath, QStandardPaths::LocateDirectory);
            if (themesDirectories.count() < 2) {
                //Make sure to add local directory
                const QString localDirectory = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + themesRelativePath;
                if (!themesDirectories.contains(localDirectory)) {
                    themesDirectories.append(localDirectory);
                }
            }
        }
    }
    QString applicationType;
    QString defaultDesktopFileName;
    QString downloadConfigFileName;
    QStringList themesDirectories;
    QMap<QString, GrantleeTheme::Theme> themes;
    QList<KToggleAction *> themesActionList;
    KDirWatch *watch;
    QActionGroup *actionGroup;
    KActionMenu *menu;
    KActionCollection *actionCollection;
    QAction *separatorAction;

    QAction *downloadThemesAction;
    QWeakPointer<KNS3::DownloadDialog> downloadThemesDialog;
    ThemeManager *q;
};

ThemeManager::ThemeManager(const QString &applicationType,
                           const QString &defaultDesktopFileName,
                           KActionCollection *actionCollection,
                           const QString &path,
                           QObject *parent)
    : QObject(parent)
    , d(new Private(applicationType, defaultDesktopFileName, actionCollection, path, this))
{
}

ThemeManager::~ThemeManager()
{
    delete d;
}

QMap<QString, GrantleeTheme::Theme> ThemeManager::themes() const
{
    return d->themes;
}

void ThemeManager::setActionGroup(QActionGroup *actionGroup)
{
    if (d->actionGroup != actionGroup) {
        d->actionGroup = actionGroup;
        d->updateActionList();
    }
}

KToggleAction *ThemeManager::actionForTheme()
{
    return d->actionForTheme();
}

void ThemeManager::setThemeMenu(KActionMenu *menu)
{
    if (d->menu != menu) {
        d->menu = menu;
        d->updateActionList();
    }
}

QStringList ThemeManager::displayExtraVariables(const QString &themename) const
{
    QMapIterator<QString, GrantleeTheme::Theme> i(d->themes);
    while (i.hasNext()) {
        i.next();
        if (i.value().dirName() == themename) {
            return i.value().displayExtraVariables();
        }
    }
    return QStringList();
}

GrantleeTheme::Theme ThemeManager::theme(const QString &themeName)
{
    return d->themes.value(themeName);
}

void ThemeManager::setDownloadNewStuffConfigFile(const QString &configFileName)
{
    d->downloadConfigFileName = configFileName;
}

QString ThemeManager::pathFromThemes(const QString &themesRelativePath,
                                     const QString &themeName,
                                     const QString &defaultDesktopFileName)
{
    QStringList themesDirectories;
    if (!themesRelativePath.isEmpty()) {
        themesDirectories = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, themesRelativePath, QStandardPaths::LocateDirectory);
        if (themesDirectories.count() < 2) {
            //Make sure to add local directory
            const QString localDirectory = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + themesRelativePath;
            if (!themesDirectories.contains(localDirectory)) {
                themesDirectories.append(localDirectory);
            }
        }
        Q_FOREACH (const QString &directory, themesDirectories) {
            QDirIterator dirIt(directory, QStringList(), QDir::AllDirs | QDir::NoDotAndDotDot);
            while (dirIt.hasNext()) {
                dirIt.next();
                const QString dirName = dirIt.fileName();
                GrantleeTheme::Theme theme = loadTheme(dirIt.filePath(), dirName, defaultDesktopFileName);
                if (theme.isValid()) {
                    if (dirName == themeName) {
                        return theme.absolutePath();
                    }
                }
            }
        }
    }
    return QString();
}

GrantleeTheme::Theme ThemeManager::loadTheme(const QString &themePath,
        const QString &dirName,
        const QString &defaultDesktopFileName)
{
    const GrantleeTheme::Theme theme(themePath, dirName, defaultDesktopFileName);
    return theme;
}

QString ThemeManager::configuredThemeName() const
{
    return configuredThemeName(d->applicationType);
}

QString ThemeManager::configuredThemeName(const QString &themeType)
{
    const KSharedConfig::Ptr config = KSharedConfig::openConfig();
    const KConfigGroup grp = config->group(themeType);
    return grp.readEntry(QStringLiteral("themeName"));
}

#include "moc_grantleethememanager.cpp"
