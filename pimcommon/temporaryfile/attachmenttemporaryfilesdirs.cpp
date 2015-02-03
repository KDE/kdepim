/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "attachmenttemporaryfilesdirs.h"

#include <QDir>
#include <QFile>
#include <QTimer>

using namespace PimCommon;

//10 secondes
static const int DELETEAFTER = 10000;

AttachmentTemporaryFilesDirs::AttachmentTemporaryFilesDirs(QObject *parent)
    : QObject(parent),
      mDelayRemoveAll(10000)
{

}

AttachmentTemporaryFilesDirs::~AttachmentTemporaryFilesDirs()
{
}

void AttachmentTemporaryFilesDirs::setDelayRemoveAllInMs(int ms)
{
    mDelayRemoveAll = (ms < 0) ? 0 : ms;
}

void AttachmentTemporaryFilesDirs::removeTempFiles()
{
    QTimer::singleShot(mDelayRemoveAll, this, SLOT(slotRemoveTempFiles()));
}

void AttachmentTemporaryFilesDirs::forceCleanTempFiles()
{
    QStringList::ConstIterator end = mTempFiles.constEnd();
    for (QStringList::ConstIterator it = mTempFiles.constBegin(); it != end; ++it) {
        QFile::remove(*it);
    }
    mTempFiles.clear();
    end = mTempDirs.constEnd();
    for (QStringList::ConstIterator it = mTempDirs.constBegin(); it != end; ++it) {
        QDir(*it).rmdir(*it);
    }
    mTempDirs.clear();
}

void AttachmentTemporaryFilesDirs::slotRemoveTempFiles()
{
    forceCleanTempFiles();
    //Delete it after cleaning
    deleteLater();
}

void AttachmentTemporaryFilesDirs::addTempFile(const QString &file)
{
    if (!mTempFiles.contains(file)) {
        mTempFiles.append(file);
    }
}

void AttachmentTemporaryFilesDirs::addTempDir(const QString &dir)
{
    if (!mTempDirs.contains(dir)) {
        mTempDirs.append(dir);
    }
}

QStringList AttachmentTemporaryFilesDirs::temporaryFiles() const
{
    return mTempFiles;
}

QStringList AttachmentTemporaryFilesDirs::temporaryDirs() const
{
    return mTempDirs;
}

