/*
  Copyright (c) 2015 Sandro Knauß <knauss@kolabsys.com>

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

#ifndef CHECKKEP14SUPPORTJOB_H
#define CHECKKEP14SUPPORTJOB_H

#include <QObject>
#include <QStringList>

#include "ksieveui_export.h"

#include <KUrl>

namespace KManageSieve {
class SieveJob;
}

namespace KSieveUi {
class KSIEVEUI_EXPORT CheckKep14SupportJob : public QObject
{
    Q_OBJECT
public:
    explicit CheckKep14SupportJob(QObject *parent=0);
    ~CheckKep14SupportJob();

    void start();

    void setServerUrl(const KUrl &url);
    void setServerName(const QString &name);
    QString serverName();

    QStringList availableScripts();
    bool hasKep14Support();
    KUrl serverUrl();

Q_SIGNALS:
    void result(CheckKep14SupportJob*, bool);

private:
    KUrl mUrl;
    KManageSieve::SieveJob *mSieveJob;
    QStringList mAvailableScripts;
    bool mKep14Support;
    QString mServerName;

private slots:
    void slotCheckKep14Support(KManageSieve::SieveJob *job, bool success, const QStringList &availableScripts, const QString &activeScript);
};
}

#endif // CHECKKEP14SUPPORTJOB_H
