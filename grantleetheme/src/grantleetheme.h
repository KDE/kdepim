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
#ifndef GRANTLEETHEME_H
#define GRANTLEETHEME_H

#include "grantleetheme_export.h"

#include <QString>
#include <QStringList>
#include <QVariantHash>
#include <QSharedDataPointer>

class GrantleeThemeTest;

namespace GrantleeTheme
{

class ThemeManager;
class ThemePrivate;

class GRANTLEETHEME_EXPORT Theme
{
public:
    explicit Theme();
    Theme(const Theme &other);
    ~Theme();

    bool operator==(const Theme &other) const;
    Theme &operator=(const Theme &other);

    bool isValid() const;

    QString description() const;
    QString themeFilename() const;
    QString name() const;
    QStringList displayExtraVariables() const;
    QString dirName() const;
    QString absolutePath() const;
    QString author() const;
    QString authorEmail() const;

    QString render(const QString &templateName, const QVariantHash &data);

    static void addPluginPath(const QString &path);

private:
    friend class ::GrantleeThemeTest;
    friend class ThemeManager;
    Theme(const QString &themePath, const QString &dirName, const QString &defaultDesktopFileName);

    QSharedDataPointer<ThemePrivate> d;
};
}

#endif // GRANTLEETHEME_H
