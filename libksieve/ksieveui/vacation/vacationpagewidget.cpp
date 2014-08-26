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

#include "vacationpagewidget.h"
#include "vacationeditwidget.h"
#include "vacationwarningwidget.h"
#include "vacationcreatescriptjob.h"
#include "vacationutils.h"
#include "sieve-vacation.h"

#include <kmime/kmime_header_parsing.h>

#include <kmanagesieve/sievejob.h>

#include <QDebug>
#include <KLocalizedString>

#include <QStackedWidget>
#include <QVBoxLayout>
#include <QLabel>

using namespace KSieveUi;
VacationPageWidget::VacationPageWidget(QWidget *parent)
    : QWidget(parent),
      mPageScript(Script),
      mSieveJob(0),
      mWasActive(false)
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
    mSieveJob = 0;
}

void VacationPageWidget::setServerUrl(const QUrl &url)
{
    mUrl = url;
    mVacationEditWidget->setEnabled(false);
    mSieveJob = KManageSieve::SieveJob::get(url);
    connect(mSieveJob, SIGNAL(gotScript(KManageSieve::SieveJob *, bool, QString, bool)),
            SLOT(slotGetResult(KManageSieve::SieveJob *, bool, QString, bool)));
}

void VacationPageWidget::setServerName(const QString &serverName)
{
    mServerName = serverName;
}

void VacationPageWidget::slotGetResult(KManageSieve::SieveJob *job, bool success, const QString &script, bool active)
{
    qDebug() << success
             << ", ?," << active << ")" << endl
             << "script:" << endl
             << script;
    mSieveJob = 0; // job deletes itself after returning from this slot!

    if (mUrl.scheme() == QLatin1String("sieve") &&
            !job->sieveCapabilities().contains(QLatin1String("vacation"))) {
        mStackWidget->setCurrentIndex(ScriptNotSupported);
        return;
    }
    mVacationEditWidget->setEnabled(true);
    QString messageText = VacationUtils::defaultMessageText();
    int notificationInterval = VacationUtils::defaultNotificationInterval();
    QStringList aliases = VacationUtils::defaultMailAliases();
    bool sendForSpam = VacationUtils::defaultSendForSpam();
    QString domainName = VacationUtils::defaultDomainName();
    if (!success) {
        active = false;    // default to inactive
    }

    if ((!success || !KSieveUi::VacationUtils::parseScript(script, messageText, notificationInterval, aliases, sendForSpam, domainName))) {
        mVacationWarningWidget->setVisible(true);
    }

    mWasActive = active;
    mVacationEditWidget->setActivateVacation(active);
    mVacationEditWidget->setMessageText(messageText);
    mVacationEditWidget->setNotificationInterval(notificationInterval);
    mVacationEditWidget->setMailAliases(aliases.join(QLatin1String(", ")));
    mVacationEditWidget->setSendForSpam(sendForSpam);
    mVacationEditWidget->setDomainName(domainName);
    mVacationEditWidget->enableDomainAndSendForSpam(!VacationSettings::allowOutOfOfficeUploadButNoSettings());

    //emit scriptActive( mWasActive, mServerName );
}

KSieveUi::VacationCreateScriptJob *VacationPageWidget::writeScript()
{
    if (mPageScript == Script) {
        KSieveUi::VacationCreateScriptJob *createJob = new KSieveUi::VacationCreateScriptJob;
        createJob->setServerUrl(mUrl);
        createJob->setServerName(mServerName);
        const QString script = VacationUtils::composeScript(mVacationEditWidget->messageText(),
                               mVacationEditWidget->notificationInterval(),
                               mVacationEditWidget->mailAliases(),
                               mVacationEditWidget->sendForSpam(),
                               mVacationEditWidget->domainName());
        const bool active = mVacationEditWidget->activateVacation();
        createJob->setStatus(active, mWasActive);
        //Q_EMIT scriptActive( active, mServerName);
        createJob->setScript(script);
        return createJob;
    }
    return 0;
}

void VacationPageWidget::setDefault()
{
    if (mVacationEditWidget->isEnabled()) {
        mVacationEditWidget->setDefault();
    }
}
