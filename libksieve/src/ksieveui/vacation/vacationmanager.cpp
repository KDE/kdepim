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

#include "vacationmanager.h"
#include "ksieveui/vacation/multiimapvacationmanager.h"
#include "ksieveui/vacation/multiimapvacationdialog.h"
#include "ksieveui/vacation/vacationcreatescriptjob.h"

#include <QWidget>

using namespace KSieveUi;

class KSieveUi::VacationManagerPrivate
{
public:
    VacationManagerPrivate(QWidget *parent)
        : mWidget(parent)
    {

    }

    QPointer<KSieveUi::MultiImapVacationDialog> mMultiImapVacationDialog;
    QPointer<KSieveUi::MultiImapVacationManager> mCheckVacation;
    QWidget *mWidget;
};

VacationManager::VacationManager(QWidget *parent)
    : QObject(parent),
      d(new KSieveUi::VacationManagerPrivate(parent))
{
}

VacationManager::~VacationManager()
{
    delete d;
}

void VacationManager::checkVacation()
{
    delete d->mCheckVacation;

    d->mCheckVacation = new KSieveUi::MultiImapVacationManager(this);
    connect(d->mCheckVacation.data(), &MultiImapVacationManager::scriptActive, this, &VacationManager::updateVacationScriptStatus);
    connect(d->mCheckVacation.data(), &MultiImapVacationManager::requestEditVacation, this, &VacationManager::editVacation);
    d->mCheckVacation->checkVacation();
}

void VacationManager::slotEditVacation(const QString &serverName)
{
    if (d->mMultiImapVacationDialog) {
        d->mMultiImapVacationDialog->show();
        d->mMultiImapVacationDialog->raise();
        d->mMultiImapVacationDialog->activateWindow();
        if (!serverName.isEmpty()) {
            d->mMultiImapVacationDialog->switchToServerNamePage(serverName);
        }
        return;
    }

    d->mMultiImapVacationDialog = new KSieveUi::MultiImapVacationDialog(d->mWidget);
    connect(d->mMultiImapVacationDialog.data(), &KSieveUi::MultiImapVacationDialog::okClicked, this, &VacationManager::slotDialogOk);
    connect(d->mMultiImapVacationDialog.data(), &KSieveUi::MultiImapVacationDialog::cancelClicked, this, &VacationManager::slotDialogCanceled);
    d->mMultiImapVacationDialog->show();
    if (!serverName.isEmpty()) {
        d->mMultiImapVacationDialog->switchToServerNamePage(serverName);
    }

}

void VacationManager::slotDialogCanceled()
{
    if (d->mMultiImapVacationDialog->isVisible()) {
        d->mMultiImapVacationDialog->hide();
    }

    d->mMultiImapVacationDialog->deleteLater();
    d->mMultiImapVacationDialog = Q_NULLPTR;
}

void VacationManager::slotDialogOk()
{
    QList<KSieveUi::VacationCreateScriptJob *> listJob = d->mMultiImapVacationDialog->listCreateJob();
    Q_FOREACH (KSieveUi::VacationCreateScriptJob *job, listJob) {
        connect(job, &VacationCreateScriptJob::scriptActive, this, &VacationManager::updateVacationScriptStatus);
        job->start();
    }
    if (d->mMultiImapVacationDialog->isVisible()) {
        d->mMultiImapVacationDialog->hide();
    }

    d->mMultiImapVacationDialog->deleteLater();

    d->mMultiImapVacationDialog = Q_NULLPTR;
}
