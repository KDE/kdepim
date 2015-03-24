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

#include "vacationmanager.h"
#include "ksieveui/vacation/multiimapvacationmanager.h"
#include "ksieveui/vacation/multiimapvacationdialog.h"
#include "ksieveui/vacation/vacationcreatescriptjob.h"

#include <KMessageBox>
#include <KLocale>

#include <QWidget>

using namespace KSieveUi;

VacationManager::VacationManager(QWidget *parent)
    : QObject(parent),
      mWidget(parent)
    , mMultiImapVacationDialog(0)
    , mQuestionAsked(false)
{
    mCheckVacation = new KSieveUi::MultiImapVacationManager( this );
    connect( mCheckVacation, SIGNAL(scriptActive(bool,QString)), SIGNAL(updateVacationScriptStatus(bool,QString)) );
    connect( mCheckVacation, SIGNAL(scriptActive(bool,QString)), SLOT(slotUpdateVacationScriptStatus(bool,QString)) );
}

VacationManager::~VacationManager()
{
    mCheckVacation = 0;
    mMultiImapVacationDialog = 0;
}

void VacationManager::checkVacation()
{
    mCheckVacation->checkVacation();
}

void VacationManager::slotUpdateVacationScriptStatus(bool active, const QString &serverName)
{
    if (active) {
        if (!mQuestionAsked) {
            mQuestionAsked = true;
            if ( KMessageBox::questionYesNo( 0, i18n( "There is still an active out-of-office reply configured.\n"
                                                      "Do you want to edit it?"), i18n("Out-of-office reply still active"),
                                             KGuiItem( i18n( "Edit"), QLatin1String("document-properties") ),
                                             KGuiItem( i18n("Ignore"), QLatin1String("dialog-cancel") ) )
                 == KMessageBox::Yes ) {
                slotEditVacation(serverName);
            }
        }
    }
}


void VacationManager::slotEditVacation(const QString &serverName)
{
    if ( mMultiImapVacationDialog ) {
        mMultiImapVacationDialog->raise();
        mMultiImapVacationDialog->activateWindow();
    } else {
        mMultiImapVacationDialog = new KSieveUi::MultiImapVacationDialog(mCheckVacation, mWidget);
        connect( mMultiImapVacationDialog, SIGNAL(okClicked()), SLOT(slotDialogOk()) );
        connect( mMultiImapVacationDialog, SIGNAL(cancelClicked()), SLOT(slotDialogCanceled()) );
    }

    mMultiImapVacationDialog->show();
    if (!serverName.isEmpty()) {
        mMultiImapVacationDialog->switchToServerNamePage(serverName);
    }

}

void VacationManager::slotDialogCanceled()
{
    mMultiImapVacationDialog->delayedDestruct();
    mMultiImapVacationDialog = 0;
}

void VacationManager::slotDialogOk()
{
    QList<KSieveUi::VacationCreateScriptJob *> listJob = mMultiImapVacationDialog->listCreateJob();
    Q_FOREACH (KSieveUi::VacationCreateScriptJob *job, listJob) {
        connect(job, SIGNAL(scriptActive(bool,QString)), SIGNAL(updateVacationScriptStatus(bool,QString)));
        job->setKep14Support(mCheckVacation->kep14Support(job->serverName()));
        job->start();
    }
    mMultiImapVacationDialog->delayedDestruct();
    mMultiImapVacationDialog = 0;
}
