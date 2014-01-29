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

#include <akonadi/agentinstance.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kpimidentities/identity.h>
#include <kpimidentities/identitymanager.h>


using namespace KSieveUi;

Vacation::Vacation(QObject * parent, bool checkOnly, const KUrl &url)
    : QObject( parent ),
      mSieveJob( 0 ),
      mDialog( 0 ),
      mWasActive( false ),
      mCheckOnly( checkOnly )
{
    if (url.isEmpty()) {
        mUrl = findURL(mServerName);
    } else {
        mUrl = url;
    }
    kDebug() << "Vacation: found url \"" << mUrl.prettyUrl() <<"\"";
    if ( mUrl.isEmpty() ) // nothing to do...
        return;
    mSieveJob = KManageSieve::SieveJob::get( mUrl );
    if (checkOnly) {
        mSieveJob->setInteractive( false );
    }
    connect( mSieveJob, SIGNAL(gotScript(KManageSieve::SieveJob*,bool,QString,bool)),
             SLOT(slotGetResult(KManageSieve::SieveJob*,bool,QString,bool)) );
}

Vacation::~Vacation() {
    if ( mSieveJob )
        mSieveJob->kill();
    mSieveJob = 0;
    delete mDialog;
    mDialog = 0;
    kDebug() << "~Vacation()";
}


KUrl Vacation::findURL(QString &serverName) const
{
    const Akonadi::AgentInstance::List instances = Util::imapAgentInstances();
    foreach ( const Akonadi::AgentInstance &instance, instances ) {
        if ( instance.status() == Akonadi::AgentInstance::Broken )
            continue;

        const KUrl url = Util::findSieveUrlForAccount( instance.identifier() );
        if ( !url.isEmpty() ) {
            serverName = instance.name();
            return url;
        }
    }

    return KUrl();
}

void Vacation::slotGetResult( KManageSieve::SieveJob * job, bool success,
                              const QString & script, bool active ) {
    kDebug() << success
             << ", ?," << active << ")" << endl
             << "script:" << endl
             << script;
    mSieveJob = 0; // job deletes itself after returning from this slot!

    if ( !mCheckOnly && mUrl.protocol() == QLatin1String("sieve") &&
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
}

void Vacation::slotDialogOk() {
    kDebug();
    // compose a new script:
    const QString script = VacationUtils::composeScript( mDialog->messageText(),
                                          mDialog->notificationInterval(),
                                          mDialog->mailAliases(),
                                          mDialog->sendForSpam(),
                                          mDialog->domainName() );
    const bool active = mDialog->activateVacation();
    emit scriptActive( active, mServerName);

    kDebug() << "script:" << endl << script;

    // and commit the dialog's settings to the server:
    mSieveJob = KManageSieve::SieveJob::put( mUrl, script, active, mWasActive );
    if ( active )
        connect( mSieveJob, SIGNAL(gotScript(KManageSieve::SieveJob*,bool,QString,bool)),
                 SLOT(slotPutActiveResult(KManageSieve::SieveJob*,bool)) );
    else
        connect( mSieveJob, SIGNAL(gotScript(KManageSieve::SieveJob*,bool,QString,bool)),
                 SLOT(slotPutInactiveResult(KManageSieve::SieveJob*,bool)) );

    // destroy the dialog:
    mDialog->delayedDestruct();
    mDialog = 0;
}

void Vacation::slotDialogCancel() {
    kDebug();
    mDialog->delayedDestruct();
    mDialog = 0;
    emit result( false );
}

void Vacation::slotPutActiveResult( KManageSieve::SieveJob * job, bool success ) {
    handlePutResult( job, success, true );
}

void Vacation::slotPutInactiveResult( KManageSieve::SieveJob * job, bool success ) {
    handlePutResult( job, success, false );
}

void Vacation::handlePutResult( KManageSieve::SieveJob *, bool success, bool activated ) {
    if ( success )
        KMessageBox::information( 0, activated
                                  ? i18n("Sieve script installed successfully on the server.\n"
                                         "Out of Office reply is now active.")
                                  : i18n("Sieve script installed successfully on the server.\n"
                                         "Out of Office reply has been deactivated.") );

    kDebug() << "( ???," << success << ", ? )";
    mSieveJob = 0; // job deletes itself after returning from this slot!
    emit result( success );
    emit scriptActive( activated, mServerName );
}

void Vacation::showVacationDialog()
{
    if (mDialog) {
        mDialog->show();
        mDialog->raise();
        mDialog->activateWindow();
    }
}


#include "moc_vacation.cpp"
