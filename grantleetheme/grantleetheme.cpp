/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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
using namespace GrantleeTheme;

Theme::Theme()
{
}

Theme::~Theme()
{
}

bool Theme::isValid() const
{
    return !mFileName.isEmpty() && !mName.isEmpty();
}

QString Theme::description() const
{
    return mDescription;
}

void Theme::setDescription(const QString &description)
{
    mDescription = description;
}

QString Theme::filename() const
{
    return mFileName;
}

void Theme::setFilename(const QString &file)
{
    mFileName = file;
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
