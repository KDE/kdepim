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

#ifndef VACATIONMANAGER_H
#define VACATIONMANAGER_H

#include <QObject>
#include "ksieveui_export.h"

#include <QPointer>


class QWidget;
namespace KSieveUi {
class MultiImapVacationManager;
class MultiImapVacationDialog;
class KSIEVEUI_EXPORT VacationManager : public QObject
{
    Q_OBJECT
public:
    explicit VacationManager(QWidget *parent);
    ~VacationManager();

    void checkVacation();

public Q_SLOTS:
    void slotEditVacation(const QString &serverName);

Q_SIGNALS:
    void updateVacationScriptStatus(bool active, const QString &serverName);
    void editVacation();

private slots:
    void slotDialogCanceled();
    void slotDialogOk();
    void slotUpdateVacationScriptStatus(bool active, const QString &serverName);

private:
    QPointer<KSieveUi::MultiImapVacationDialog> mMultiImapVacationDialog;
    QPointer<KSieveUi::MultiImapVacationManager> mCheckVacation;
    QWidget *mWidget;
    bool mQuestionAsked;
};
}

#endif // VACATIONMANAGER_H
