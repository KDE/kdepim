/*
   Copyright (C) 2012-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "archivestorage.h"
#include <KZip>
#include <KLocalizedString>

ArchiveStorage::ArchiveStorage(const QString &filename, QObject *parent)
    : QObject(parent),
      mArchive(new KZip(filename))
{
}

ArchiveStorage::~ArchiveStorage()
{
    closeArchive();
    delete mArchive;
    mArchive = Q_NULLPTR;
}

void ArchiveStorage::closeArchive()
{
    if (mArchive && mArchive->isOpen()) {
        mArchive->close();
    }
}

QString ArchiveStorage::filename() const
{
    if (mArchive) {
        return mArchive->fileName();
    }
    return QString();
}

bool ArchiveStorage::openArchive(bool write)
{
    const bool result = mArchive->open(write ? QIODevice::WriteOnly : QIODevice::ReadOnly);
    if (!result) {
        if (write) {
            Q_EMIT error(i18n("Archive cannot be opened in write mode."));
        } else {
            Q_EMIT error(i18n("Archive cannot be opened in read mode."));
        }
    }
    return result;
}

KZip *ArchiveStorage::archive() const
{
    return mArchive;
}

