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

#include "exportaddressbookresourcejob.h"
#include "utils.h"
#include <pimsettingbackupthread.h>
#include <KLocalizedString>
#include <KZip>
#include <QDebug>
#include <QFileInfo>

ExportAddressbookResourceJob::ExportAddressbookResourceJob(QObject *parent)
    : QObject(parent),
      mZip(Q_NULLPTR)
{

}

ExportAddressbookResourceJob::~ExportAddressbookResourceJob()
{

}

void ExportAddressbookResourceJob::setArchive(KZip *zip)
{
    mZip = zip;
}

void ExportAddressbookResourceJob::finished()
{
    Q_EMIT terminated();
    deleteLater();
}

void ExportAddressbookResourceJob::start()
{
    if (mZip) {
        PimSettingBackupThread *thread = new PimSettingBackupThread(mZip, mUrl, mArchivePath, QStringLiteral("addressbook.zip"));
        connect(thread, &PimSettingBackupThread::error, this, &ExportAddressbookResourceJob::error);
        connect(thread, &PimSettingBackupThread::info, this, &ExportAddressbookResourceJob::info);
        connect(thread, &PimSettingBackupThread::terminated, this, &ExportAddressbookResourceJob::slotTerminated);
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        thread->start();
    } else {
        qDebug() << "zip not defined !";
        finished();
    }
}

void ExportAddressbookResourceJob::slotTerminated(bool success)
{
    if (success) {
        const QString errorStr = Utils::storeResources(mZip, mIdentifier, mArchivePath);
        if (!errorStr.isEmpty()) {
            Q_EMIT error(errorStr);
        }
        QString url = Utils::akonadiAgentConfigPath(mIdentifier);
        if (!url.isEmpty()) {
            QFileInfo fi(url);
            const QString filename = fi.fileName();
            const bool fileAdded  = mZip->addLocalFile(mUrl, mArchivePath + filename);
            if (fileAdded) {
                Q_EMIT info(i18n("\"%1\" was backed up.", filename));
            } else {
                Q_EMIT error(i18n("\"%1\" file cannot be added to backup file.", filename));
            }
        }
    }
    finished();
}

void ExportAddressbookResourceJob::setIdentifier(const QString &identifier)
{
    mIdentifier = identifier;
}

void ExportAddressbookResourceJob::setUrl(const QString &url)
{
    mUrl = url;
}

void ExportAddressbookResourceJob::setArchivePath(const QString &archivePath)
{
    mArchivePath = archivePath;
}
