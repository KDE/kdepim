/*
   Copyright (C) 2015-2016 Montel Laurent <montel@kde.org>

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

#include "exportresourcearchivejob.h"
#include "utils.h"
#include "pimsettingexportcore_debug.h"
#include <pimsettingbackupthread.h>
#include <KLocalizedString>
#include <KZip>
#include <QFileInfo>

#include <AkonadiCore/ServerManager>

ExportResourceArchiveJob::ExportResourceArchiveJob(QObject *parent)
    : QObject(parent),
      mZip(Q_NULLPTR),
      mThread(Q_NULLPTR)
{

}

ExportResourceArchiveJob::~ExportResourceArchiveJob()
{

}

void ExportResourceArchiveJob::setArchive(KZip *zip)
{
    mZip = zip;
}

void ExportResourceArchiveJob::setIdentifier(const QString &identifier)
{
    mIdentifier = identifier;
}

void ExportResourceArchiveJob::setUrl(const QString &url)
{
    mUrl = url;
}

void ExportResourceArchiveJob::setArchivePath(const QString &archivePath)
{
    mArchivePath = archivePath;
}

void ExportResourceArchiveJob::setArchiveName(const QString &archiveName)
{
    mArchiveName = archiveName;
}

void ExportResourceArchiveJob::start()
{
    if (mZip) {
        mThread = new PimSettingBackupThread(mZip, mUrl, mArchivePath, mArchiveName);
        connect(mThread, &PimSettingBackupThread::error, this, &ExportResourceArchiveJob::error);
        connect(mThread, &PimSettingBackupThread::info, this, &ExportResourceArchiveJob::info);
        connect(mThread, &PimSettingBackupThread::terminated, this, &ExportResourceArchiveJob::slotTerminated);
        connect(mThread, &QThread::finished, mThread, &QObject::deleteLater);
        mThread->start();
    } else {
        qCDebug(PIMSETTINGEXPORTERCORE_LOG) << "zip not defined !";
        finished();
    }
}

void ExportResourceArchiveJob::slotTerminated(bool success)
{
    if (success) {
        const QString errorStr = Utils::storeResources(mZip, mIdentifier, mArchivePath);
        if (!errorStr.isEmpty()) {
            Q_EMIT error(errorStr);
        }
        const QString url = Akonadi::ServerManager::agentConfigFilePath(mIdentifier);
        if (!url.isEmpty()) {
            const QFileInfo fi(url);
            const QString filename = fi.fileName();
            const bool fileAdded  = mZip->addLocalFile(url, mArchivePath + filename);
            if (fileAdded) {
                Q_EMIT info(i18n("\"%1\" was backed up.", filename));
            } else {
                Q_EMIT error(i18n("\"%1\" file cannot be added to backup file.", filename));
            }
        }
    }
    finished();
}

void ExportResourceArchiveJob::slotTaskCanceled()
{
    qCDebug(PIMSETTINGEXPORTERCORE_LOG) << " void ExportResourceArchiveJob::slotTaskCanceled()";
    //TODO
    if (mThread) {
        mThread->exit();
    }
}

void ExportResourceArchiveJob::finished()
{
    Q_EMIT terminated();
    deleteLater();
}
