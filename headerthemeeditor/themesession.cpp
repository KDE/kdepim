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

#include "themesession.h"

#include <KConfig>
#include <KConfigGroup>
#include <KDebug>

#include <QDir>

ThemeSession::ThemeSession(const QString &projectDirectory)
    : mProjectDirectory(projectDirectory)
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

void ThemeSession::loadSession(const QString &session)
{
    KConfig config(session);
    if (config.hasGroup(QLatin1String("Global"))) {
        KConfigGroup global = config.group(QLatin1String("Global"));
        mProjectDirectory = global.readEntry("path", QString());
        mMainPageFileName = global.readEntry(QLatin1String("mainPageName"), QString());
        mExtraPage = global.readEntry(QLatin1String("extraPagesName"), QStringList());
    } else {
        kDebug()<<QString::fromLatin1("\"%1\" is not a session file").arg(session);
    }
}

void ThemeSession::writeSession()
{
    KConfig config(mProjectDirectory + QDir::separator() + QLatin1String("theme.themerc"));
    KConfigGroup global = config.group(QLatin1String("Global"));
    global.writeEntry(QLatin1String("path"), mProjectDirectory);
    global.writeEntry(QLatin1String("mainPageName"), mMainPageFileName);
    global.writeEntry(QLatin1String("extraPagesName"), mExtraPage);
    config.sync();
}
