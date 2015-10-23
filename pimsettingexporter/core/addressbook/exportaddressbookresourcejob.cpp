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

#include <pimsettingbackupthread.h>

ExportAddressbookResourceJob::ExportAddressbookResourceJob(KZip *zip, const QString &url, const QString &archivePath, const QString &archivename, QObject *parent)
    : QObject(parent)
{

}

ExportAddressbookResourceJob::~ExportAddressbookResourceJob()
{

}

void ExportAddressbookResourceJob::start()
{
    PimSettingBackupThread *thread = new PimSettingBackupThread(archive(), url, archivePath, QStringLiteral("addressbook.zip"));
    connect(thread, &PimSettingBackupThread::error, this, &ExportAddressbookResourceJob::error);
    connect(thread, &PimSettingBackupThread::info, this, &ExportAddressbookResourceJob::info);
    connect(thread, &PimSettingBackupThread::terminated, this, &ExportAddressbookResourceJob::slotTerminated);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
}

void ExportAddressbookResourceJob::slotTerminated(bool success)
{
    if (success) {
        const QString errorStr = Utils::storeResources(archive(), identifier, archivePath);
        if (!errorStr.isEmpty()) {
            Q_EMIT error(errorStr);
        }
        url = Utils::akonadiAgentConfigPath(identifier);
        if (!url.isEmpty()) {
            QFileInfo fi(url);
            const QString filename = fi.fileName();
            const bool fileAdded  = archive()->addLocalFile(url, archivePath + filename);
            if (fileAdded) {
                Q_EMIT info(i18n("\"%1\" was backed up.", filename));
            } else {
                Q_EMIT error(i18n("\"%1\" file cannot be added to backup file.", filename));
            }
        }
    }
    Q_EMIT terminated();
}
