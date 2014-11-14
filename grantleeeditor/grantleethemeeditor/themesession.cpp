/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "themesession.h"

#include <KConfig>
#include <KConfigGroup>
#include <QDebug>
#include <KMessageBox>
#include <KLocalizedString>

#include <QDir>

using namespace GrantleeThemeEditor;

ThemeSession::ThemeSession(const QString &projectDirectory, const QString &themeTypeName)
    : mProjectDirectory(projectDirectory),
      mThemeTypeName(themeTypeName),
      mVersion(1)
{
}

ThemeSession::~ThemeSession()
{
}

QString ThemeSession::projectDirectory() const
{
    return mProjectDirectory;
}

void ThemeSession::addExtraPage(const QString &filename)
{
    mExtraPage.append(filename);
}

QStringList ThemeSession::extraPages() const
{
    return mExtraPage;
}

void ThemeSession::setMainPageFileName(const QString &filename)
{
    mMainPageFileName = filename;
}

QString ThemeSession::mainPageFileName() const
{
    return mMainPageFileName;
}

bool ThemeSession::loadSession(const QString &session)
{
    KConfig config(session);
    if (config.hasGroup(QStringLiteral("Global"))) {
        KConfigGroup global = config.group(QStringLiteral("Global"));
        const int version = global.readEntry(QStringLiteral("version"), 0);
        if (version >= mVersion) {
            if (global.readEntry(QStringLiteral("themeTypeName")) != mThemeTypeName) {
                KMessageBox::error(0, i18n("Error during theme loading"), i18n("You are trying to load a theme which cannot be read by this application"));
                return false;
            }
        }
        mProjectDirectory = global.readEntry("path", QString());
        mMainPageFileName = global.readEntry(QStringLiteral("mainPageName"), QString());
        mExtraPage = global.readEntry(QStringLiteral("extraPagesName"), QStringList());
        return true;
    } else {
        qDebug() << QStringLiteral("\"%1\" is not a session file").arg(session);
        return false;
    }
}

void ThemeSession::writeSession(const QString &directory)
{
    QString themeDirectory = (directory.isEmpty() ? mProjectDirectory : directory);
    KConfig config(themeDirectory + QDir::separator() + QLatin1String("theme.themerc"));
    KConfigGroup global = config.group(QStringLiteral("Global"));
    global.writeEntry(QStringLiteral("path"), themeDirectory);
    global.writeEntry(QStringLiteral("mainPageName"), mMainPageFileName);
    global.writeEntry(QStringLiteral("extraPagesName"), mExtraPage);
    global.writeEntry(QStringLiteral("themeTypeName"), mThemeTypeName);
    global.writeEntry(QStringLiteral("version"), mVersion);
    config.sync();
}
