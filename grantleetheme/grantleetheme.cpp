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

#include "grantleetheme.h"
#include <KConfig>
#include <KConfigGroup>
#include <QDir>

using namespace GrantleeTheme;

Theme::Theme()
{
}

Theme::Theme(const QString &themePath, const QString &dirName, const QString &defaultDesktopFileName)
{
    const QString themeInfoFile = themePath + QDir::separator() + defaultDesktopFileName;
    KConfig config( themeInfoFile );
    KConfigGroup group( &config, QLatin1String( "Desktop Entry" ) );
    if (group.isValid()) {
        setDirName(dirName);
        setName( group.readEntry( "Name", QString() ) );
        setDescription( group.readEntry( "Description", QString() ) );
        setThemeFilename( group.readEntry( "FileName" , QString() ) );
        setDisplayExtraVariables( group.readEntry( "DisplayExtraVariables", QStringList() ) );
        setAbsolutePath(themePath);
    }
}

Theme::~Theme()
{
}



bool Theme::isValid() const
{
    return !mThemeFileName.isEmpty() && !mName.isEmpty();
}

QString Theme::description() const
{
    return mDescription;
}

void Theme::setDescription(const QString &description)
{
    mDescription = description;
}

QString Theme::themeFilename() const
{
    return mThemeFileName;
}

void Theme::setThemeFilename(const QString &file)
{
    mThemeFileName = file;
}

QString Theme::name() const
{
    return mName;
}

void Theme::setName(const QString &n)
{
    mName = n;
}

QStringList Theme::displayExtraVariables() const
{
    return mDisplayExtraVariables;
}

void Theme::setDisplayExtraVariables(const QStringList &variables)
{
    mDisplayExtraVariables = variables;
}

void Theme::setDirName(const QString &name)
{
    mDirName = name;
}

QString Theme::dirName() const
{
    return mDirName;
}

void Theme::setAbsolutePath(const QString &absPath)
{
    mAbsolutePath = absPath;
}

QString Theme::absolutePath() const
{
    return mAbsolutePath;
}

void Theme::setAuthor(const QString &author)
{
    mAuthor = author;
}

QString Theme::author() const
{
    return mAuthor;
}

void Theme::setAuthorEmail(const QString &email)
{
    mEmail = email;
}

QString Theme::authorEmail() const
{
    return mEmail;
}
