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

#ifndef EXPORTADDRESSBOOKRESOURCEJOB_H
#define EXPORTADDRESSBOOKRESOURCEJOB_H

#include <QObject>
class KZip;
class PimSettingBackupThread;
class ExportResourceArchiveJob : public QObject
{
    Q_OBJECT
public:
    explicit ExportResourceArchiveJob(QObject *parent = Q_NULLPTR);
    ~ExportResourceArchiveJob();

    void setArchivePath(const QString &archivePath);

    void setUrl(const QString &url);

    void setIdentifier(const QString &identifier);

    void setArchive(KZip *zip);
    void start();
    void setArchiveName(const QString &archiveName);

Q_SIGNALS:
    void error(const QString &str);
    void info(const QString &str);
    void terminated();

private Q_SLOTS:
    void slotTerminated(bool success);

public Q_SLOTS:
    void slotTaskCanceled();

private:
    void finished();
    QString mArchiveName;
    QString mUrl;
    QString mArchivePath;
    QString mIdentifier;
    KZip *mZip;
    PimSettingBackupThread *mThread;
};

#endif // EXPORTADDRESSBOOKRESOURCEJOB_H
