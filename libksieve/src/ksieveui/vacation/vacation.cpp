/*  -*- c++ -*-
    vacation.cpp

    KMail, the KDE mail client.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "vacation.h"
#include "vacationutils.h"
#include "vacationscriptextractor.h"

#include "sieve-vacation.h"
#include "util/util.h"
#include "vacationdialog.h"
#include <kmanagesieve/sievejob.h>

#include <agentinstance.h>
#include "libksieve_debug.h"
#include <KLocalizedString>
#include <kmessagebox.h>

#include <KIdentityManagement/kidentitymanagement/identity.h>
#include <KIdentityManagement/kidentitymanagement/identitymanager.h>

using namespace KSieveUi;

Vacation::Vacation(QObject *parent, bool checkOnly, const QUrl &url)
    : QObject(parent),
      mSieveJob(Q_NULLPTR),
      mDialog(Q_NULLPTR),
      mWasActive(false),
      mCheckOnly(checkOnly)
{
    if (url.isEmpty()) {
        mUrl = findURL(mServerName);
    } else {
        mUrl = url;
    }
    qCDebug(LIBKSIEVE_LOG) << "Vacation: found url \"" << mUrl.toDisplayString() << "\"";
    if (mUrl.isEmpty()) { // nothing to do...
        return;
    }
    mSieveJob = KManageSieve::SieveJob::get(mUrl);
    if (checkOnly) {
        mSieveJob->setInteractive(false);
    }
    connect(mSieveJob, &KManageSieve::SieveJob::gotScript, this, &Vacation::slotGetResult);
}

Vacation::~Vacation()
{
    if (mSieveJob) {
        mSieveJob->kill();
    }
    mSieveJob = Q_NULLPTR;
    delete mDialog;
    mDialog = Q_NULLPTR;
    qCDebug(LIBKSIEVE_LOG) << "~Vacation()";
}

bool Vacation::isUsable() const
{
    return !mUrl.isEmpty();
}

QString Vacation::serverName() const
{
    return mServerName;
}

QUrl Vacation::findURL(QString &serverName) const
{
    const Akonadi::AgentInstance::List instances = Util::imapAgentInstances();
    foreach (const Akonadi::AgentInstance &instance, instances) {
        if (instance.status() == Akonadi::AgentInstance::Broken) {
            continue;
        }

        const QUrl url = Util::findSieveUrlForAccount(instance.identifier());
        if (!url.isEmpty()) {
            serverName = instance.name();
            return url;
        }
    }

    return QUrl();
}

void Vacation::slotGetResult(KManageSieve::SieveJob *job, bool success,
                             const QString &script, bool active)
{
    qCDebug(LIBKSIEVE_LOG) << success
                           << ", ?," << active << ")" << endl
                           << "script:" << endl
                           << script;
    mSieveJob = Q_NULLPTR; // job deletes itself after returning from this slot!

    if (!mCheckOnly && mUrl.scheme() == QLatin1String("sieve") &&
            !job->sieveCapabilities().contains(QStringLiteral("vacation"))) {
        KMessageBox::sorry(Q_NULLPTR, i18n("Your server did not list \"vacation\" in "
                                           "its list of supported Sieve extensions;\n"
                                           "without it, KMail cannot install out-of-"
                                           "office replies for you.\n"
                                           "Please contact your system administrator."));
        Q_EMIT result(false);
        return;
    }

    const bool supportsDate = job->sieveCapabilities().contains(QStringLiteral("date"));

    if (!mDialog && !mCheckOnly) {
        mDialog = new VacationDialog(i18n("Configure \"Out of Office\" Replies"), Q_NULLPTR, false);
    }

    if (!success) {
        active = false;    // default to inactive
    }

    KSieveUi::VacationUtils::Vacation vacation = KSieveUi::VacationUtils::parseScript(script);

    if (!mCheckOnly && (!success || (!vacation.isValid()  && !script.trimmed().isEmpty()))) {
        KMessageBox::information(Q_NULLPTR, i18n("Someone (probably you) changed the "
                                 "vacation script on the server.\n"
                                 "KMail is no longer able to determine "
                                 "the parameters for the autoreplies.\n"
                                 "Default values will be used."));
    }
    mWasActive = active;
    if (mDialog) {
        mDialog->setActivateVacation(active && vacation.active);
        mDialog->setMailAction(vacation.mailAction, vacation.mailActionRecipient);
        mDialog->setSubject(vacation.subject);
        mDialog->setMessageText(vacation.messageText);
        mDialog->setNotificationInterval(vacation.notificationInterval);
        mDialog->setMailAliases(vacation.aliases);
        mDialog->setSendForSpam(vacation.sendForSpam);
        mDialog->setDomainName(vacation.excludeDomain);
        mDialog->enableDomainAndSendForSpam(!VacationSettings::allowOutOfOfficeUploadButNoSettings());
        mDialog->enableDates(supportsDate);

        if (supportsDate) {
            mDialog->setStartDate(vacation.startDate);
            mDialog->setStartTime(vacation.startTime);
            mDialog->setEndDate(vacation.endDate);
            mDialog->setEndTime(vacation.endTime);
        }

        connect(mDialog, &VacationDialog::okClicked, this, &Vacation::slotDialogOk);
        connect(mDialog, &VacationDialog::cancelClicked, this, &Vacation::slotDialogCancel);
        mDialog->show();
    }

    Q_EMIT scriptActive(mWasActive, mServerName);
    if (mCheckOnly && mWasActive) {
        if (KMessageBox::questionYesNo(Q_NULLPTR, i18n("There is still an active out-of-office reply configured.\n"
                                       "Do you want to edit it?"), i18n("Out-of-office reply still active"),
                                       KGuiItem(i18n("Edit"), QStringLiteral("document-properties")),
                                       KGuiItem(i18n("Ignore"), QStringLiteral("dialog-cancel")))
                == KMessageBox::Yes) {
            Q_EMIT requestEditVacation();
        }
    }
}

void Vacation::slotDialogOk()
{
    qCDebug(LIBKSIEVE_LOG);
    // compose a new script:
    const bool active = mDialog->activateVacation();
    VacationUtils::Vacation vacation;
    vacation.valid = true;
    vacation.active = active;
    vacation.messageText = mDialog->messageText();
    vacation.subject = mDialog->subject();
    vacation.mailAction = mDialog->mailAction();
    vacation.mailActionRecipient = mDialog->mailActionRecipient();
    vacation.notificationInterval = mDialog->notificationInterval();
    vacation.aliases = mDialog->mailAliases();
    vacation.sendForSpam = mDialog->sendForSpam();
    vacation.excludeDomain =  mDialog->domainName();
    vacation.startDate = mDialog->startDate();
    vacation.startTime = mDialog->startTime();
    vacation.endDate = mDialog->endDate();
    vacation.endTime = mDialog->endTime();
    const QString script = VacationUtils::composeScript(vacation);
    emit scriptActive(active, mServerName);

    qCDebug(LIBKSIEVE_LOG) << "script:" << endl << script;

    // and commit the dialog's settings to the server:
    mSieveJob = KManageSieve::SieveJob::put(mUrl, script, active, mWasActive);
    if (active) {
        connect(mSieveJob, &KManageSieve::SieveJob::gotScript, this, &Vacation::slotPutActiveResult);
    } else {
        connect(mSieveJob, &KManageSieve::SieveJob::gotScript, this, &Vacation::slotPutInactiveResult);
    }

    // destroy the dialog:
    //mDialog->delayedDestruct();
    mDialog->hide();
    mDialog->deleteLater();
    mDialog = Q_NULLPTR;
}

void Vacation::slotDialogCancel()
{
    qCDebug(LIBKSIEVE_LOG);
    mDialog->hide();
    mDialog->deleteLater();
    mDialog = Q_NULLPTR;
    Q_EMIT result(false);
}

void Vacation::slotPutActiveResult(KManageSieve::SieveJob *job, bool success)
{
    handlePutResult(job, success, true);
}

void Vacation::slotPutInactiveResult(KManageSieve::SieveJob *job, bool success)
{
    handlePutResult(job, success, false);
}

void Vacation::handlePutResult(KManageSieve::SieveJob *, bool success, bool activated)
{
    if (success)
        KMessageBox::information(Q_NULLPTR, activated
                                 ? i18n("Sieve script installed successfully on the server.\n"
                                        "Out of Office reply is now active.")
                                 : i18n("Sieve script installed successfully on the server.\n"
                                        "Out of Office reply has been deactivated."));

    qCDebug(LIBKSIEVE_LOG) << "( ???," << success << ", ? )";
    mSieveJob = Q_NULLPTR; // job deletes itself after returning from this slot!
    Q_EMIT result(success);
    Q_EMIT scriptActive(activated, mServerName);
}

void Vacation::showVacationDialog()
{
    if (mDialog) {
        mDialog->show();
        mDialog->raise();
        mDialog->activateWindow();
    }
}

