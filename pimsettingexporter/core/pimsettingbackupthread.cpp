/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "pimsettingbackupthread.h"

#include <KZip>
#include <QTemporaryFile>
#include <KLocalizedString>
#include <QDebug>

PimSettingBackupThread::PimSettingBackupThread(KZip *zip, const QString &url, const QString &archivePath, const QString &archivename, QObject *parent)
    : QThread(parent),
      mUrl(url),
      mArchivePath(archivePath),
      mArchiveName(archivename),
      mZip(zip)
{
    qDebug() << " PimSettingBackupThread::PimSettingBackupThread" << this;
}

PimSettingBackupThread::~PimSettingBackupThread()
{
    qDebug() << " PimSettingBackupThread::~PimSettingBackupThread()" << this;
}

void PimSettingBackupThread::run()
{
    QTemporaryFile tmp;
    tmp.open();
    KZip *archiveFile = new KZip(tmp.fileName());
    archiveFile->setCompression(KZip::NoCompression);
    bool result = archiveFile->open(QIODevice::WriteOnly);
    if (!result) {
        Q_EMIT error(i18n("Impossible to open archive file."));
        Q_EMIT terminated(false);
        delete archiveFile;
        return;
    }
    const bool vcarddirAdded = archiveFile->addLocalDirectory(mUrl, QString());
    if (!vcarddirAdded) {
        Q_EMIT error(i18n("Impossible to backup \"%1\".", mUrl));
        Q_EMIT terminated(false);
        delete archiveFile;
        return;
    }
    archiveFile->close();
    tmp.close();

    const bool fileAdded = mZip->addLocalFile(tmp.fileName(), mArchivePath  + mArchiveName);
    if (fileAdded) {
        Q_EMIT info(i18n("\"%1\" was backed up.", mUrl));
    } else {
        Q_EMIT error(i18n("\"%1\" file cannot be added to backup file.", mUrl));
    }
    delete archiveFile;
    Q_EMIT terminated(fileAdded);
}
