/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "ksieveui_export.h"

#include <KUrl>

#include <QObject>
namespace KManageSieve {
class SieveJob;
}

namespace KSieveUi {
class KSIEVEUI_EXPORT VacationHelperJob : public QObject
{
    Q_OBJECT
public:
    explicit VacationHelperJob(const QString &accountName, QObject *parent=0);
    ~VacationHelperJob();

    void searchActiveJob();

private Q_SLOTS:
    void slotGetScriptList(KManageSieve::SieveJob *job, bool success, const QStringList &scriptList, const QString &activeScript);

Q_SIGNALS:
    void canNotGetScriptList();
    void resourceHasNotSieveSupport();
    void hasActiveScript(const QString &);

private:
    KUrl mUrl;
    QString mAccountName;
    KManageSieve::SieveJob *mSieveJob;
};
}

#endif // VACATIONHELPERJOB_H
