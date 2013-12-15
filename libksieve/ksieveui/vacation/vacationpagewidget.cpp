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

#include "vacationpagewidget.h"
#include "vacationeditwidget.h"
#include "vacationwarningwidget.h"
#include "vacationcreatescriptjob.h"
#include "vacationutils.h"

#include <kmime/kmime_header_parsing.h>

#include <kmanagesieve/sievejob.h>

#include <KDebug>

#include <QVBoxLayout>

using namespace KSieveUi;
VacationPageWidget::VacationPageWidget(QWidget *parent)
    : QWidget(parent),
      mSieveJob(0),
      mWasActive(false)
{
    QVBoxLayout *lay = new QVBoxLayout;
    lay->setMargin(0);

    mVacationWarningWidget = new VacationWarningWidget;
    lay->addWidget(mVacationWarningWidget);

    mVacationEditWidget = new VacationEditWidget;
    lay->addWidget(mVacationEditWidget);
    setLayout(lay);
}

VacationPageWidget::~VacationPageWidget()
{
    if ( mSieveJob )
        mSieveJob->kill();
    mSieveJob = 0;
}

void VacationPageWidget::setServerUrl(const KUrl &url)
{
    mUrl = url;
    mSieveJob = KManageSieve::SieveJob::get( url );
    connect( mSieveJob, SIGNAL(gotScript(KManageSieve::SieveJob*,bool,QString,bool)),
             SLOT(slotGetResult(KManageSieve::SieveJob*,bool,QString,bool)) );
}

void VacationPageWidget::setServerName(const QString &serverName)
{
    mServerName = serverName;
}

void VacationPageWidget::slotGetResult( KManageSieve::SieveJob * job, bool success, const QString & script, bool active )
{
    kDebug() << success
             << ", ?," << active << ")" << endl
             << "script:" << endl
             << script;
    mSieveJob = 0; // job deletes itself after returning from this slot!
    //TODO

#if 0
    if ( !mCheckOnly && mUrl.protocol() == QLatin1String("sieve") && !job->sieveCapabilities().isEmpty() &&
         !job->sieveCapabilities().contains(QLatin1String("vacation")) ) {
        KMessageBox::sorry( 0, i18n( "Your server did not list \"vacation\" in "
                                     "its list of supported Sieve extensions;\n"
                                     "without it, KMail cannot install out-of-"
                                     "office replies for you.\n"
                                     "Please contact your system administrator." ) );
        emit result( false );
        return;
    }

    if ( !mDialog && !mCheckOnly )
        mDialog = new VacationDialog( i18n("Configure \"Out of Office\" Replies"), 0, false );

    QString messageText = VacationUtils::defaultMessageText();
    int notificationInterval = VacationUtils::defaultNotificationInterval();
    QStringList aliases = VacationUtils::defaultMailAliases();
    bool sendForSpam = VacationUtils::defaultSendForSpam();
    QString domainName = VacationUtils::defaultDomainName();
    if ( !success ) active = false; // default to inactive

    if ( !mCheckOnly && ( !success || !KSieveUi::VacationUtils::parseScript( script, messageText, notificationInterval, aliases, sendForSpam, domainName ) ) )
        KMessageBox::information( 0, i18n("Someone (probably you) changed the "
                                          "vacation script on the server.\n"
                                          "KMail is no longer able to determine "
                                          "the parameters for the autoreplies.\n"
                                          "Default values will be used." ) );

    mWasActive = active;
    if ( mDialog ) {
        mDialog->setActivateVacation( active );
        mDialog->setMessageText( messageText );
        mDialog->setNotificationInterval( notificationInterval );
        mDialog->setMailAliases( aliases.join(QLatin1String(", ")) );
        mDialog->setSendForSpam( sendForSpam );
        mDialog->setDomainName( domainName );
        mDialog->enableDomainAndSendForSpam( !VacationSettings::allowOutOfOfficeUploadButNoSettings() );

        connect( mDialog, SIGNAL(okClicked()), SLOT(slotDialogOk()) );
        connect( mDialog, SIGNAL(cancelClicked()), SLOT(slotDialogCancel()) );
        connect( mDialog, SIGNAL(defaultClicked()), SLOT(slotDialogDefaults()) );

        mDialog->show();
    }

    emit scriptActive( mWasActive, mServerName );
    if ( mCheckOnly && mWasActive ) {
        if ( KMessageBox::questionYesNo( 0, i18n( "There is still an active out-of-office reply configured.\n"
                                                  "Do you want to edit it?"), i18n("Out-of-office reply still active"),
                                         KGuiItem( i18n( "Edit"), QLatin1String("document-properties") ),
                                         KGuiItem( i18n("Ignore"), QLatin1String("dialog-cancel") ) )
             == KMessageBox::Yes ) {
            emit requestEditVacation();
        }
    }

#endif

}

void VacationPageWidget::writeScript()
{
    KSieveUi::VacationCreateScriptJob *createJob = new KSieveUi::VacationCreateScriptJob;
    createJob->setServerUrl(mUrl);
    createJob->setServerName(mServerName);
    const QString script = VacationUtils::composeScript( mVacationEditWidget->messageText(),
                                          mVacationEditWidget->notificationInterval(),
                                          mVacationEditWidget->mailAliases(),
                                          mVacationEditWidget->sendForSpam(),
                                          mVacationEditWidget->domainName() );
    const bool active = mVacationEditWidget->activateVacation();
    //Q_EMIT scriptActive( active, mServerName);
    createJob->setScript(script);

}
