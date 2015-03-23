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
#include "multiimapvacationmanager.h"
#include <managescriptsjob/parseuserscriptjob.h>
#include "sieve-vacation.h"

#include <kmime/kmime_header_parsing.h>

#include <kmanagesieve/sievejob.h>

#include <KDebug>
#include <KLocalizedString>

#include <QStackedWidget>
#include <QVBoxLayout>
#include <QLabel>

using namespace KSieveUi;
VacationPageWidget::VacationPageWidget(QWidget *parent)
    : QWidget(parent),
      mPageScript(Script),
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
    QLabel *lab = new QLabel(i18n( "Your server did not list \"vacation\" in "
                                  "its list of supported Sieve extensions;"
                                  "without it, KMail cannot install out-of-"
                                  "office replies for you."
                                  "Please contact your system administrator." ) );
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
}

void VacationPageWidget::setServerUrl(const KUrl &url)
{
    mUrl = url;
    mVacationEditWidget->setEnabled(false);
}

void VacationPageWidget::setVacationManager(MultiImapVacationManager *vacationManager)
{
    mVacationManager = vacationManager;
    connect(mVacationManager, SIGNAL(scriptAvailable(QString,QStringList,QString,QString,bool)),
            SLOT(slotGetResult(QString,QStringList,QString,QString,bool)));
    mVacationManager->checkVacation(mServerName, mUrl);
}

void VacationPageWidget::setServerName(const QString &serverName)
{
    mServerName = serverName;
}

void VacationPageWidget::slotGetResult(const QString &serverName, const QStringList &sieveCapabilities, const QString &scriptName, const QString &script, bool active)
{
    if (serverName != mServerName) {
        return;
    }

    kDebug() << serverName << sieveCapabilities << endl
             << scriptName << "(" << active << ")" << endl
             << "script:" << endl
             << script;

    if ( mUrl.protocol() == QLatin1String("sieve") &&
         !sieveCapabilities.contains(QLatin1String("vacation")) ) {
        mStackWidget->setCurrentIndex(ScriptNotSupported);
        return;
    }

    // Whether the server supports the "date" extension
    const bool supportsSieveDate = mUrl.protocol() == QLatin1String("sieve") && sieveCapabilities.contains(QLatin1String("date"));

    QString messageText = VacationUtils::defaultMessageText();
    QString subject = VacationUtils::defaultSubject();
    int notificationInterval = VacationUtils::defaultNotificationInterval();
    QStringList aliases = VacationUtils::defaultMailAliases();
    bool sendForSpam = VacationUtils::defaultSendForSpam();
    QString domainName = VacationUtils::defaultDomainName();
    QDate startDate = VacationUtils::defaultStartDate();
    QDate endDate = VacationUtils::defaultEndDate();

    const bool bParse = KSieveUi::VacationUtils::parseScript(script, messageText, subject, notificationInterval, aliases, sendForSpam, domainName, startDate, endDate);

    if (!bParse) {
        mVacationWarningWidget->setVisible(true);
    }

    mWasActive = active;
    mVacationEditWidget->setEnabled(true);
    mVacationEditWidget->setActivateVacation( active );
    mVacationEditWidget->setMessageText( messageText );
    mVacationEditWidget->setSubject( subject );
    mVacationEditWidget->setNotificationInterval( notificationInterval );
    mVacationEditWidget->setMailAliases( aliases.join(QLatin1String(", ")) );
    mVacationEditWidget->setSendForSpam( sendForSpam );
    mVacationEditWidget->setDomainName( domainName );
    mVacationEditWidget->enableDomainAndSendForSpam( !VacationSettings::allowOutOfOfficeUploadButNoSettings() );

    mVacationEditWidget->enableDates( supportsSieveDate );
    if ( supportsSieveDate ) {
        mVacationEditWidget->setStartDate( startDate );
        mVacationEditWidget->setEndDate( endDate );
    }

    //emit scriptActive( mWasActive, mServerName );
}



KSieveUi::VacationCreateScriptJob *VacationPageWidget::writeScript()
{
    if (mPageScript == Script) {
        KSieveUi::VacationCreateScriptJob *createJob = new KSieveUi::VacationCreateScriptJob;
        createJob->setServerUrl(mUrl);
        createJob->setServerName(mServerName);
        const QString script = VacationUtils::composeScript( mVacationEditWidget->messageText(),
                                                             mVacationEditWidget->subject(),
                                                             mVacationEditWidget->notificationInterval(),
                                                             mVacationEditWidget->mailAliases(),
                                                             mVacationEditWidget->sendForSpam(),
                                                             mVacationEditWidget->domainName(),
                                                             mVacationEditWidget->startDate(),
                                                             mVacationEditWidget->endDate() );
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
    if (mVacationEditWidget->isEnabled())
        mVacationEditWidget->setDefault();
}
