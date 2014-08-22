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

#ifndef THEMESESSION_H
#define THEMESESSION_H
#include "grantleethemeeditor_export.h"
#include <QString>
#include <QStringList>

namespace GrantleeThemeEditor
{
class GRANTLEETHEMEEDITOR_EXPORT ThemeSession
{
public:
    ThemeSession(const QString &projectDirectory, const QString &themeTypeName);
    ~ThemeSession();
    bool loadSession(const QString &session);
    void writeSession(const QString &directory = QString());

    QString projectDirectory() const;

    void addExtraPage(const QString &filename);
    QStringList extraPages() const;

    void setMainPageFileName(const QString &filename);
    QString mainPageFileName() const;

private:
    QString mProjectDirectory;
    QString mMainPageFileName;
    QStringList mExtraPage;
    QString mThemeTypeName;
    int mVersion;
};
}

#endif // THEMESESSION_H
