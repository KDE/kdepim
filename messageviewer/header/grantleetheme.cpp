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

#include "grantleetheme.h"
using namespace MessageViewer;

GrantleeTheme::GrantleeTheme()
{
}

GrantleeTheme::~GrantleeTheme()
{
}

bool GrantleeTheme::isValid() const
{
    return !mFileName.isEmpty() && !mName.isEmpty();
}

QString GrantleeTheme::description() const
{
    return mDescription;
}

void GrantleeTheme::setDescription(const QString &description)
{
    mDescription = description;
}

QString GrantleeTheme::filename() const
{
    return mFileName;
}

void GrantleeTheme::setFilename(const QString &file)
{
    mFileName = file;
}

QString GrantleeTheme::name() const
{
    return mName;
}

void GrantleeTheme::setName(const QString &n)
{
    mName = n;
}

QStringList GrantleeTheme::displayExtraHeaders() const
{
    return mDisplayExtraHeaders;
}

void GrantleeTheme::setDisplayExtraHeaders(const QStringList &variables)
{
    mDisplayExtraHeaders = variables;
}

void GrantleeTheme::setDirName(const QString &name)
{
    mDirName = name;
}

QString GrantleeTheme::dirName() const
{
    return mDirName;
}

void GrantleeTheme::setAbsolutePath(const QString &absPath)
{
    mAbsolutePath = absPath;
}

QString GrantleeTheme::absolutePath() const
{
    return mAbsolutePath;
}

void GrantleeTheme::setAuthor(const QString &author)
{
    mAuthor = author;
}

QString GrantleeTheme::author() const
{
    return mAuthor;
}

void GrantleeTheme::setAuthorEmail(const QString &email)
{
    mEmail = email;
}

QString GrantleeTheme::authorEmail() const
{
    return mEmail;
}
