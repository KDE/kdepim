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

#include "vacationpagewidget.h"
#include "vacationeditwidget.h"
#include "vacationwarningwidget.h"
#include "vacationcreatescriptjob.h"
#include "vacationutils.h"
#include "sieve-vacation.h"

#include <kmime/kmime_header_parsing.h>

#include <kmanagesieve/sievejob.h>

#include "libksieve_debug.h"
#include <KLocalizedString>

#include <QStackedWidget>
#include <QVBoxLayout>
#include <QLabel>

using namespace KSieveUi;
VacationPageWidget::VacationPageWidget(QWidget *parent)
    : QWidget(parent),
      mSieveJob(Q_NULLPTR),
      mPageScript(Script),
      mWasActive(false),
      mHasDateSupport(false)
{
    QVBoxLayout *lay = new QVBoxLayout;
    lay->setMargin(0);
    mStackWidget = new QStackedWidget;
    lay->addWidget(mStackWidget);

    //Main Page
    QWidget *mainPage = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;
    mainPage->setLayout(vbox);
    mVacationWarningWidget = new VacationWarningWidget;
    vbox->addWidget(mVacationWarningWidget);

    mVacationEditWidget = new VacationEditWidget;
    vbox->addWidget(mVacationEditWidget);
    mStackWidget->addWidget(mainPage);

    QWidget *w = new QWidget;
    vbox = new QVBoxLayout;
    QLabel *lab = new QLabel(i18n("Your server did not list \"vacation\" in "
                                  "its list of supported Sieve extensions;"
                                  "without it, KMail cannot install out-of-"
                                  "office replies for you."
                                  "Please contact your system administrator."));
    QFont font = lab->font();
    font.setBold(true);
    lab->setFont(font);
    vbox->addWidget(lab);
    vbox->setAlignment(lab, Qt::AlignVCenter);

    lab->setWordWrap(true);
    w->setLayout(vbox);
    mStackWidget->addWidget(w);

    mStackWidget->setCurrentIndex(Script);
    setLayout(lay);
}

VacationPageWidget::~VacationPageWidget()
{
    if (mSieveJob) {
        mSieveJob->kill();
    }
    mSieveJob = Q_NULLPTR;
}

void VacationPageWidget::setServerUrl(const QUrl &url)
{
    mUrl = url;
    mVacationEditWidget->setEnabled(false);
    mSieveJob = KManageSieve::SieveJob::get(url);
    connect(mSieveJob, &KManageSieve::SieveJob::gotScript, this, &VacationPageWidget::slotGetResult);
}

void VacationPageWidget::setServerName(const QString &serverName)
{
    mServerName = serverName;
}

void VacationPageWidget::slotGetResult(KManageSieve::SieveJob *job, bool success, const QString &script, bool active)
{
    qCDebug(LIBKSIEVE_LOG) << success
                           << ", ?," << active << ")" << endl
                           << "script:" << endl
                           << script;
    mSieveJob = Q_NULLPTR; // job deletes itself after returning from this slot!

    if (mUrl.scheme() == QLatin1String("sieve") &&
            !job->sieveCapabilities().contains(QStringLiteral("vacation"))) {
        mStackWidget->setCurrentIndex(ScriptNotSupported);
        return;
    }

    KSieveUi::VacationUtils::Vacation vacation = KSieveUi::VacationUtils::parseScript(script);

    if (!vacation.isValid() && !script.trimmed().isEmpty()) {
        mVacationWarningWidget->setVisible(true);
    }

    mWasActive = active;
    mVacationEditWidget->setEnabled(true);
    mVacationEditWidget->setActivateVacation(active && vacation.active);
    mVacationEditWidget->setMessageText(vacation.messageText);
    mVacationEditWidget->setSubject(vacation.subject);
    mVacationEditWidget->setNotificationInterval(vacation.notificationInterval);
    mVacationEditWidget->setMailAliases(vacation.aliases);
    mVacationEditWidget->setSendForSpam(vacation.sendForSpam);
    mVacationEditWidget->setDomainName(vacation.excludeDomain);
    mVacationEditWidget->enableDomainAndSendForSpam(!VacationSettings::allowOutOfOfficeUploadButNoSettings());

    mVacationEditWidget->enableDates(mHasDateSupport);
    if (mHasDateSupport) {
        mVacationEditWidget->setStartDate(vacation.startDate);
        mVacationEditWidget->setEndDate(vacation.endDate);
    }
}

KSieveUi::VacationCreateScriptJob *VacationPageWidget::writeScript()
{
    if (mPageScript == Script) {
        KSieveUi::VacationCreateScriptJob *createJob = new KSieveUi::VacationCreateScriptJob;
        createJob->setServerUrl(mUrl);
        createJob->setServerName(mServerName);
        const bool active = mVacationEditWidget->activateVacation();
        VacationUtils::Vacation vacation;
        vacation.valid = true;
        vacation.active = active;
        vacation.messageText = mVacationEditWidget->messageText();
        vacation.subject = mVacationEditWidget->subject();
        vacation.notificationInterval = mVacationEditWidget->notificationInterval();
        vacation.aliases = mVacationEditWidget->mailAliases();
        vacation.sendForSpam = mVacationEditWidget->sendForSpam();
        vacation.excludeDomain =  mVacationEditWidget->domainName();
        vacation.startDate = mVacationEditWidget->startDate();
        vacation.endDate = mVacationEditWidget->endDate();
        const QString script = VacationUtils::composeScript(vacation);
        createJob->setStatus(active, mWasActive);
        createJob->setScript(script);
        return createJob;
    }
    return Q_NULLPTR;
}

void VacationPageWidget::setDefault()
{
    if (mVacationEditWidget->isEnabled()) {
        mVacationEditWidget->setDefault();
    }
}
