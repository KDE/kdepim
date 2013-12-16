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

#ifndef MULTIIMAPVACATIONMANAGER_H
#define MULTIIMAPVACATIONMANAGER_H

#include <QObject>

#include "ksieveui_export.h"

namespace KSieveUi {
class KSIEVEUI_EXPORT MultiImapVacationManager : public QObject
{
    Q_OBJECT
public:
    explicit MultiImapVacationManager(QObject *parent=0);
    ~MultiImapVacationManager();

    void checkVacation();

Q_SIGNALS:
    void scriptActive(bool active, const QString &serverName);
    void requestEditVacation();

private slots:
    void slotScriptActive(bool active, const QString &serverName);

private:
    int mNumberOfJobs;
    bool mQuestionAsked;
    bool mCheckInProgress;
};
}
#endif // MULTIIMAPVACATIONMANAGER_H
