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

#include <KLocalizedString>
#include <KMessageBox>

using namespace KSieveUi;

class KSieveUi::VacationManagerPrivate
{
public:
    VacationManagerPrivate(QWidget *parent)
        : mWidget(parent)
        , mMultiImapVacationDialog(Q_NULLPTR)
        , mCheckVacation(Q_NULLPTR)
        , mQuestionAsked(false)
    {

    }

    QWidget *mWidget;
    QPointer<KSieveUi::MultiImapVacationDialog> mMultiImapVacationDialog;
    QPointer<KSieveUi::MultiImapVacationManager> mCheckVacation;
    bool mQuestionAsked;
};

VacationManager::VacationManager(QWidget *parent)
    : QObject(parent),
      d(new KSieveUi::VacationManagerPrivate(parent))
{
    d->mCheckVacation = new KSieveUi::MultiImapVacationManager(this);
    connect(d->mCheckVacation.data(), &KSieveUi::MultiImapVacationManager::scriptActive, this, &VacationManager::updateVacationScriptStatus);
    connect(d->mCheckVacation.data(), &KSieveUi::MultiImapVacationManager::scriptActive, this, &VacationManager::slotUpdateVacationScriptStatus);
}

VacationManager::~VacationManager()
{
    delete d;
}

void VacationManager::checkVacation()
{
    d->mCheckVacation->checkVacation();
}

void VacationManager::slotUpdateVacationScriptStatus(bool active, const QString &serverName)
{
    if (active) {
        if (!d->mQuestionAsked) {
            d->mQuestionAsked = true;
            if (KMessageBox::questionYesNo(0, i18n("There is still an active out-of-office reply configured.\n"
                                                   "Do you want to edit it?"), i18n("Out-of-office reply still active"),
                                           KGuiItem(i18n("Edit"), QStringLiteral("document-properties")),
                                           KGuiItem(i18n("Ignore"), QStringLiteral("dialog-cancel")))
                    == KMessageBox::Yes) {
                slotEditVacation(serverName);
            }
        }
    }
}

void VacationManager::slotEditVacation(const QString &serverName)
{
    if (d->mMultiImapVacationDialog) {
        d->mMultiImapVacationDialog->raise();
        d->mMultiImapVacationDialog->activateWindow();
    } else {
        d->mMultiImapVacationDialog = new KSieveUi::MultiImapVacationDialog(d->mCheckVacation, d->mWidget);
        connect(d->mMultiImapVacationDialog.data(), &KSieveUi::MultiImapVacationDialog::okClicked, this, &VacationManager::slotDialogOk);
        connect(d->mMultiImapVacationDialog.data(), &KSieveUi::MultiImapVacationDialog::cancelClicked, this, &VacationManager::slotDialogCanceled);
    }
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
        job->setKep14Support(d->mCheckVacation->kep14Support(job->serverName()));
        job->start();
    }
    if (d->mMultiImapVacationDialog->isVisible()) {
        d->mMultiImapVacationDialog->hide();
    }

    d->mMultiImapVacationDialog->deleteLater();

    d->mMultiImapVacationDialog = Q_NULLPTR;
}
