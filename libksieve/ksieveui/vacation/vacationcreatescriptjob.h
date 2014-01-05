/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#ifndef VACATIONCREATESCRIPTJOB_H
#define VACATIONCREATESCRIPTJOB_H

#include <QObject>

#include "ksieveui_export.h"

#include <KUrl>

namespace KManageSieve {
class SieveJob;
}

namespace KSieveUi {
class KSIEVEUI_EXPORT VacationCreateScriptJob : public QObject
{
    Q_OBJECT
public:
    explicit VacationCreateScriptJob(QObject *parent=0);
    ~VacationCreateScriptJob();

    void start();

    void setServerUrl(const KUrl &url);
    void setScript(const QString &script);
    void setServerName(const QString &servername);
    void setStatus(bool activate, bool wasActive);

Q_SIGNALS:
    void result(bool);
    void scriptActive(bool activated, const QString &serverName);

private slots:
    void slotPutActiveResult(KManageSieve::SieveJob *job, bool success);
    void slotPutInactiveResult(KManageSieve::SieveJob *job, bool success);

private:
    void handlePutResult(KManageSieve::SieveJob *, bool success, bool activated);
    KUrl mUrl;
    QString mScript;
    QString mServerName;
    bool mActivate;
    bool mWasActive;
    KManageSieve::SieveJob *mSieveJob;
};
}

#endif // VACATIONCREATESCRIPTJOB_H
