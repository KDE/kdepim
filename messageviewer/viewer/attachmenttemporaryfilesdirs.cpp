/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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
#include <QDebug>

using namespace MessageViewer;

//10 secondes
static int DELETEAFTER = 10000;

AttachmentTemporaryFilesDirs::AttachmentTemporaryFilesDirs(QObject *parent)
    : QObject(parent)
{

}

AttachmentTemporaryFilesDirs::~AttachmentTemporaryFilesDirs()
{
}

void AttachmentTemporaryFilesDirs::removeTempFiles()
{
    QTimer::singleShot(DELETEAFTER, this, SLOT(slotRemoveTempFiles()));
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

void AttachmentTemporaryFilesDirs::addTempFile( const QString &file )
{
    mTempFiles.append( file );
}

void AttachmentTemporaryFilesDirs::addTempDir( const QString &dir )
{
    mTempDirs.append( dir );
}

QStringList AttachmentTemporaryFilesDirs::temporaryFiles() const
{
    return mTempFiles;
}



#include "moc_attachmenttemporaryfilesdirs.cpp"
