/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef VACATIONHELPERJOB_H
#define VACATIONHELPERJOB_H


#include <QUrl>

#include <QObject>
namespace KManageSieve
{
class SieveJob;
}

namespace KSieveUi
{
class VacationHelperJob : public QObject
{
    Q_OBJECT
public:
    explicit VacationHelperJob(const QUrl &url, QObject *parent = Q_NULLPTR);
    ~VacationHelperJob();

    void searchActiveJob();

private Q_SLOTS:
    void slotGetScriptList(KManageSieve::SieveJob *job, bool success, const QStringList &scriptList, const QString &activeScript);

Q_SIGNALS:
    void canNotGetScriptList();
    void resourceHasNotSieveSupport();
    void scriptListResult(const QStringList &scriptList, const QString &activeScript, bool hasIncludeSupport);

private:
    void killJob();
    QUrl mUrl;
    KManageSieve::SieveJob *mSieveJob;
};
}

#endif // VACATIONHELPERJOB_H
