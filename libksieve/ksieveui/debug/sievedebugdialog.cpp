/*
    sievedebugdialog.cpp

    Copyright (c) 2005 Martijn Klingens <klingens@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "sievedebugdialog.h"

#include <akonadi/agentinstance.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksieve/error.h>
#include <ksieve/parser.h>
#include <ksieve/scriptbuilder.h>
#include <kmanagesieve/sievejob.h>
#include <ksieveui/util.h>
#include <ktextedit.h>

#include <QtCore/QTimer>

namespace
{

class SieveDebugDataExtractor : public KSieve::ScriptBuilder
{
    enum Context
    {
        None = 0,

        // command itself:
        SieveDebugCommand,

        // tagged args:
        Days, Addresses
    };

public:
    SieveDebugDataExtractor()
        :   KSieve::ScriptBuilder()
    {
        kDebug() ;
    }

    virtual ~SieveDebugDataExtractor()
    {
        kDebug() ;
    }

private:
    void commandStart( const QString & identifier )
    {
        kDebug() << "Identifier: '" << identifier <<"'";
        reset();
    }

    void commandEnd()
    {
        kDebug() ;
    }

    void testStart( const QString & )
    {
        kDebug() ;
    }

    void testEnd()
    {
        kDebug() ;
    }

    void testListStart()
    {
        kDebug() ;
    }

    void testListEnd()
    {
        kDebug() ;
    }

    void blockStart()
    {
        kDebug() ;
    }

    void blockEnd()
    {
        kDebug() ;
    }

    void hashComment( const QString & )
    {
        kDebug() ;
    }

    void bracketComment( const QString & )
    {
        kDebug() ;
    }

    void lineFeed()
    {
        kDebug() ;
    }

    void error( const KSieve::Error & e )
    {
        kDebug() << "###" <<"Error:" <<
                    e.asString() << "@" << e.line() << "," << e.column();
    }

    void finished()
    {
        kDebug() ;
    }

    void taggedArgument( const QString & tag )
    {
        kDebug() << "Tag: '" << tag <<"'";
    }

    void stringArgument( const QString & string, bool, const QString & )
    {
        kDebug() << "String: '" << string <<"'";
    }

    void numberArgument( unsigned long number, char )
    {
        kDebug() << "Number:" << number;
    }

    void stringListArgumentStart()
    {
        kDebug() ;
    }

    void stringListEntry( const QString & string, bool, const QString & )
    {
        kDebug() << "String: '" << string <<"'";
    }

    void stringListArgumentEnd()
    {
        kDebug() ;
    }

private:
    void reset()
    {
        kDebug() ;
    }
};

}

using namespace KSieveUi;

SieveDebugDialog::SieveDebugDialog( QWidget *parent )
    : KDialog( parent ),
      mSieveJob( 0 )
{
    setCaption( i18n( "Sieve Diagnostics" ) );
    setButtons( Close );

    // Collect all accounts
    const Akonadi::AgentInstance::List lst = KSieveUi::Util::imapAgentInstances();
    foreach ( const Akonadi::AgentInstance& type, lst )
    {
        mResourceIdentifier << type.identifier();
    }

    mEdit = new KTextEdit( this );
    mEdit->setReadOnly( true );
    setMainWidget( mEdit );

    mEdit->setText( i18n( "Collecting diagnostic information about Sieve support...\n\n" ) );

    setInitialSize( QSize( 640, 480 ) );

    if ( !mResourceIdentifier.isEmpty() )
        QTimer::singleShot( 0, this, SLOT(slotDiagNextAccount()) );
}

SieveDebugDialog::~SieveDebugDialog()
{
    if ( mSieveJob )
    {
        mSieveJob->kill();
        mSieveJob = 0;
    }
    kDebug() ;
}


void SieveDebugDialog::slotDiagNextAccount()
{
    if ( mResourceIdentifier.isEmpty() )
        return;
    QString ident = mResourceIdentifier.first();

    mEdit->append( i18n( "Collecting data for account '%1'...\n", ident ) );
    mEdit->append( i18n( "------------------------------------------------------------\n" ) );

    // Detect URL for this IMAP account
    const KUrl url = KSieveUi::Util::findSieveUrlForAccount( ident );
    if ( !url.isValid() ) {
        mEdit->append( i18n( "(Account does not support Sieve)\n\n" ) );
    } else {
        mUrl = url;

        mSieveJob = KManageSieve::SieveJob::list( mUrl );

        connect( mSieveJob, SIGNAL(gotList(KManageSieve::SieveJob*,bool,QStringList,QString)),
                 SLOT(slotGetScriptList(KManageSieve::SieveJob*,bool,QStringList,QString)) );

        // Bypass the singleShot timer -- it's fired when we get our data
        return;
    }

    // Handle next account async
    mResourceIdentifier.pop_front();
    QTimer::singleShot( 0, this, SLOT(slotDiagNextAccount()) );
}

void SieveDebugDialog::slotDiagNextScript()
{
    if ( mScriptList.isEmpty() )
    {
        // Continue handling accounts instead
        mScriptList.clear();
        mResourceIdentifier.pop_front();
        QTimer::singleShot( 0, this, SLOT(slotDiagNextAccount()) );
        return;
    }

    QString scriptFile = mScriptList.first();
    mScriptList.pop_front();

    mEdit->append( i18n( "Contents of script '%1':\n", scriptFile ) );

    mUrl = KSieveUi::Util::findSieveUrlForAccount( mResourceIdentifier.first() );

    mUrl.setFileName( scriptFile );

    mSieveJob = KManageSieve::SieveJob::get( mUrl );

    connect( mSieveJob, SIGNAL(gotScript(KManageSieve::SieveJob*,bool,QString,bool)),
             SLOT(slotGetScript(KManageSieve::SieveJob*,bool,QString,bool)) );
}

void SieveDebugDialog::slotGetScript( KManageSieve::SieveJob * /* job */, bool success,
                                      const QString &script, bool active )
{
    kDebug() << "( ??," << success
             << ", ?," << active << ")" << endl
             << "script:" << endl
             << script;
    mSieveJob = 0; // job deletes itself after returning from this slot!

    if ( script.isEmpty() )
    {
        mEdit->append( i18n( "(This script is empty.)\n\n" ) );
    }
    else
    {
        mEdit->append( i18n(
                           "------------------------------------------------------------\n"
                           "%1\n"
                           "------------------------------------------------------------\n\n", script ) );
    }

    // Fetch next script
    QTimer::singleShot( 0, this, SLOT(slotDiagNextScript()) );
}

void SieveDebugDialog::slotGetScriptList( KManageSieve::SieveJob *job, bool success,
                                          const QStringList &scriptList, const QString &activeScript )
{
    kDebug() << "Success:" << success <<", List:" << scriptList.join(QLatin1String(",") ) <<
                ", active:" << activeScript;
    mSieveJob = 0; // job deletes itself after returning from this slot!

    mEdit->append( i18n( "Sieve capabilities:\n" ) );
    const QStringList caps = job->sieveCapabilities();
    if ( caps.isEmpty() )
    {
        mEdit->append( i18n( "(No special capabilities available)" ) );
    }
    else
    {
        QStringList::const_iterator end = caps.constEnd();
        for ( QStringList::const_iterator it = caps.constBegin(); it !=end; ++it )
            mEdit->append( QLatin1String("* ") + *it + QLatin1Char('\n') );
        mEdit->append( QLatin1String("\n") );
    }

    mEdit->append( i18n( "Available Sieve scripts:\n" ) );

    if ( scriptList.isEmpty() )
    {
        mEdit->append( i18n( "(No Sieve scripts available on this server)\n\n" ) );
    }
    else
    {
        mScriptList = scriptList;
        QStringList::const_iterator end = scriptList.constEnd();
        for ( QStringList::const_iterator it = scriptList.constBegin(); it != end; ++it )
            mEdit->append( QLatin1String("* ") + *it + QLatin1Char('\n') );
        mEdit->append( QLatin1String("\n") );
        mEdit->append( i18n( "Active script: %1\n\n", activeScript ) );
    }

    // Handle next job: dump scripts for this server
    QTimer::singleShot( 0, this, SLOT(slotDiagNextScript()) );
}

void SieveDebugDialog::slotDialogOk()
{
    kDebug();
}

void SieveDebugDialog::slotPutActiveResult( KManageSieve::SieveJob * job, bool success )
{
    handlePutResult( job, success, true );
}

void SieveDebugDialog::slotPutInactiveResult( KManageSieve::SieveJob * job, bool success )
{
    handlePutResult( job, success, false );
}

void SieveDebugDialog::handlePutResult( KManageSieve::SieveJob *, bool success, bool activated )
{
    if ( success )
    {
        KMessageBox::information( 0, activated ? i18n(
                                                     "Sieve script installed successfully on the server.\n"
                                                     "Out of Office reply is now active." )
                                               : i18n( "Sieve script installed successfully on the server.\n"
                                                       "Out of Office reply has been deactivated." ) );
    }

    kDebug() << "( ???," << success <<", ? )";
    mSieveJob = 0; // job deletes itself after returning from this slot!
}

#include "sievedebugdialog.moc"
