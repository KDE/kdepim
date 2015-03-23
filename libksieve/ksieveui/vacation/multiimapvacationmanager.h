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

#ifndef MULTIIMAPVACATIONMANAGER_H
#define MULTIIMAPVACATIONMANAGER_H

#include <QObject>
#include <QMap>

#include "ksieveui_export.h"

class KUrl;

namespace KSieveUi {
class CheckKep14SupportJob;
class VacationCheckJob;
class KSIEVEUI_EXPORT MultiImapVacationManager : public QObject
{
    Q_OBJECT
public:
    explicit MultiImapVacationManager(QObject *parent=0);
    ~MultiImapVacationManager();

    void checkVacation();
    QMap<QString, KUrl> serverList();
    void checkVacation(const QString &serverName, const KUrl &url);

Q_SIGNALS:
    void scriptActive(bool active, const QString &serverName);
    void scriptAvailable(const QString &serverName, const QStringList &sieveCapabilities, const QString &scriptName, const QString &script, bool active);

private slots:
    void slotScriptActive(VacationCheckJob* job, QString scriptName, bool active);
    void slotCheckKep14Ended(CheckKep14SupportJob *job, bool success);

private:
    int mNumberOfJobs;
    bool mCheckInProgress;

    QMap<QString, bool> mKep14Support;      //if the server has KEP:14 support
};
}
#endif // MULTIIMAPVACATIONMANAGER_H
