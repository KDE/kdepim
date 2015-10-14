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

#ifndef GRANTLEETHEMEMANAGER_H
#define GRANTLEETHEMEMANAGER_H

#include "grantleetheme.h"
#include "grantleetheme_export.h"
#include <QObject>
#include <QMap>
class QActionGroup;
class KActionCollection;
class KToggleAction;
class KActionMenu;

namespace GrantleeTheme
{
class GRANTLEETHEME_EXPORT ThemeManager : public QObject
{
    Q_OBJECT
public:
    explicit ThemeManager(const QString &themeType,
                          const QString &defaultDesktopFileName,
                          KActionCollection *actionCollection = Q_NULLPTR,
                          const QString &path = QString(),
                          QObject *parent = Q_NULLPTR);
    ~ThemeManager();

    QMap<QString, GrantleeTheme::Theme> themes() const;

    void setActionGroup(QActionGroup *actionGroup);

    KToggleAction *actionForTheme();

    void setThemeMenu(KActionMenu *menu);

    QStringList displayExtraVariables(const QString &themename) const;

    GrantleeTheme::Theme theme(const QString &themeName);

    void setDownloadNewStuffConfigFile(const QString &configFileName);

    QString configuredThemeName() const;
    static QString configuredThemeName(const QString &themeType);

    static QString pathFromThemes(const QString &path, const QString &themeName,
                                  const QString &defaultDesktopFilename);
    static GrantleeTheme::Theme loadTheme(const QString &themePath,
                                          const QString &dirName,
                                          const QString &defaultDesktopFilename);

Q_SIGNALS:
    void themesChanged();
    void grantleeThemeSelected();
    void updateThemes();

private:
    Q_PRIVATE_SLOT(d, void directoryChanged())
    Q_PRIVATE_SLOT(d, void slotDownloadHeaderThemes())
    Q_PRIVATE_SLOT(d, void slotThemeSelected())
    class Private;
    Private *const d;
};
}
#endif // GRANTLEETHEMEMANAGER_H
