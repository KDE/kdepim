/*
  Copyright (c) 2015-2016 Montel Laurent <montel@kde.org>

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

#ifndef PIMSETTINGBACKUPTHREAD_H
#define PIMSETTINGBACKUPTHREAD_H

#include <QThread>
class KZip;
class PimSettingBackupThread : public QThread
{
    Q_OBJECT
public:
    explicit PimSettingBackupThread(KZip *zip, const QString &url, const QString &archivePath, const QString &archivename, QObject *parent = Q_NULLPTR);
    ~PimSettingBackupThread();

Q_SIGNALS:
    void error(const QString &str);
    void info(const QString &str);
    void terminated(bool success);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    QString mUrl;
    QString mArchivePath;
    QString mArchiveName;
    KZip *mZip;
};

#endif // PIMSETTINGBACKUPTHREAD_H
