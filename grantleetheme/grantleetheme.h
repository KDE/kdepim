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
namespace GrantleeTheme {
class GRANTLEETHEME_EXPORT Theme
{
public:
    Theme();
    Theme(const QString &themePath, const QString &dirName, const QString &defaultDesktopFileName);
    ~Theme();

    bool isValid() const;

    QString description() const;
    void setDescription(const QString &description);

    QString themeFilename() const;
    void setThemeFilename(const QString &file);

    QString name() const;
    void setName(const QString &);

    QStringList displayExtraVariables() const;
    void setDisplayExtraVariables(const QStringList &);

    void setDirName(const QString &name);
    QString dirName() const;

    void setAbsolutePath(const QString &absPath);
    QString absolutePath() const;

    void setAuthor(const QString &);
    QString author() const;

    void setAuthorEmail(const QString &);
    QString authorEmail() const;

private:
    QStringList mDisplayExtraVariables;
    QString mThemeFileName;
    QString mDescription;
    QString mName;
    QString mDirName;
    QString mAbsolutePath;
    QString mAuthor;
    QString mEmail;
};
}

#endif // GRANTLEETHEME_H
