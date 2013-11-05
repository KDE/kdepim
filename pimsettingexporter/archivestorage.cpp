/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>
  
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

#include "archivestorage.h"
#include <KZip>
#include <KLocale>

ArchiveStorage::ArchiveStorage(const QString& filename, QObject *parent)
    :QObject(parent), mArchive(new KZip(filename))
{
}

ArchiveStorage::~ArchiveStorage()
{
    closeArchive();
    delete mArchive;
    mArchive = 0;
}

void ArchiveStorage::closeArchive()
{
    if (mArchive && mArchive->isOpen()) {
        mArchive->close();
    }
}

bool ArchiveStorage::openArchive(bool write)
{
    bool result = mArchive->open(write ? QIODevice::WriteOnly : QIODevice::ReadOnly);
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

