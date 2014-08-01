/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef VACATIONCHECKJOB_H
#define VACATIONCHECKJOB_H

#include <QObject>
#include <QUrl>

namespace KManageSieve {
class SieveJob;
}

namespace KSieveUi {
class VacationCheckJob : public QObject
{
    Q_OBJECT
public:
    explicit VacationCheckJob(const QUrl &url, const QString &serverName, QObject *parent=0);
    ~VacationCheckJob();

Q_SIGNALS:
    void scriptActive(bool active, const QString &serverName);

private slots:
    void slotGetResult(KManageSieve::SieveJob *job, bool success, const QString &script, bool active);

private:
    QString mServerName;
    QUrl mUrl;
    KManageSieve::SieveJob * mSieveJob;
};
}

#endif // VACATIONCHECKJOB_H
