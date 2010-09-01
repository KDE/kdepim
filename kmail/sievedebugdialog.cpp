/*
    sievedebugdialog.cpp

    KMail, the KDE mail client.
    Copyright (c) 2005 Martijn Klingens <klingens@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

// This file is only compiled when debug is enabled, it is
// not useful enough for non-developers to have this in releases.
#if !defined(NDEBUG)

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "sievedebugdialog.h"

#include <cassert>
#include <limits.h>

#include <tqdatetime.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kmime_header_parsing.h>
#include <ksieve/error.h>
#include <ksieve/parser.h>
#include <ksieve/scriptbuilder.h>
#include <libkpimidentities/identity.h>
#include <libkpimidentities/identitymanager.h>

#include "kmacctimap.h"
#include "accountmanager.h"
using KMail::AccountManager;
#include "kmkernel.h"
#include "sievejob.h"
#include <tqtextedit.h>

using KMail::SieveJob;
using KMime::Types::AddrSpecList;

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
        kdDebug( 5006 ) << k_funcinfo << endl;
    }

    virtual ~SieveDebugDataExtractor()
    {
        kdDebug( 5006 ) << k_funcinfo << endl;
    }

private:
    void commandStart( const TQString & identifier )
    {
        kdDebug( 5006 ) << k_funcinfo << "Identifier: '" << identifier << "'" << endl;
        reset();
    }

    void commandEnd()
    {
        kdDebug( 5006 ) << k_funcinfo << endl;
    }

    void testStart( const TQString & )
    {
        kdDebug( 5006 ) << k_funcinfo << endl;
    }

    void testEnd()
    {
        kdDebug( 5006 ) << k_funcinfo << endl;
    }

    void testListStart()
    {
        kdDebug( 5006 ) << k_funcinfo << endl;
    }

    void testListEnd()
    {
        kdDebug( 5006 ) << k_funcinfo << endl;
    }

    void blockStart()
    {
        kdDebug( 5006 ) << k_funcinfo << endl;
    }

    void blockEnd()
    {
        kdDebug( 5006 ) << k_funcinfo << endl;
    }

    void hashComment( const TQString & )
    {
        kdDebug( 5006 ) << k_funcinfo << endl;
    }

    void bracketComment( const TQString & )
    {
        kdDebug( 5006 ) << k_funcinfo << endl;
    }

    void lineFeed()
    {
        kdDebug( 5006 ) << k_funcinfo << endl;
    }

    void error( const KSieve::Error & e )
    {
        kdDebug( 5006 ) << "### " << k_funcinfo << "Error: " <<
            e.asString() << " @ " << e.line() << "," << e.column() << endl;
    }

    void finished()
    {
        kdDebug( 5006 ) << k_funcinfo << endl;
    }

    void taggedArgument( const TQString & tag )
    {
        kdDebug( 5006 ) << k_funcinfo << "Tag: '" << tag << "'" << endl;
    }

    void stringArgument( const TQString & string, bool, const TQString & )
    {
        kdDebug( 5006 ) << k_funcinfo << "String: '" << string << "'" << endl;
    }

    void numberArgument( unsigned long number, char )
    {
        kdDebug( 5006 ) << k_funcinfo << "Number: " << number << endl;
    }

    void stringListArgumentStart()
    {
        kdDebug( 5006 ) << k_funcinfo << endl;
    }

    void stringListEntry( const TQString & string, bool, const TQString & )
    {
        kdDebug( 5006 ) << k_funcinfo << "String: '" << string << "'" << endl;
    }

    void stringListArgumentEnd()
    {
        kdDebug( 5006 ) << k_funcinfo << endl;
    }

private:
    void reset()
    {
        kdDebug( 5006 ) << k_funcinfo << endl;
    }
};

} // Anon namespace

namespace KMail
{

SieveDebugDialog::SieveDebugDialog( TQWidget *parent, const char *name )
:   KDialogBase( parent, name, true, i18n( "Sieve Diagnostics" ), KDialogBase::Ok,
    KDialogBase::Ok, true ),
    mSieveJob( 0 )
{
    // Collect all accounts
    AccountManager *am = kmkernel->acctMgr();
    assert( am );
    for ( KMAccount *a = am->first(); a; a = am->next() )
        mAccountList.append( a );

    mEdit = new TQTextEdit( this );
    mEdit->setReadOnly(true);
    setMainWidget( mEdit );

    mEdit->setText( i18n( "Collecting diagnostic information about Sieve support...\n\n" ) );

    setInitialSize( TQSize( 640, 480 ) );

    if ( !mAccountList.isEmpty() )
        TQTimer::singleShot( 0, this, TQT_SLOT( slotDiagNextAccount() ) );
}

SieveDebugDialog::~SieveDebugDialog()
{
    if ( mSieveJob )
    {
        mSieveJob->kill();
        mSieveJob = 0;
    }
    kdDebug( 5006 ) << k_funcinfo << endl;
}

static KURL urlFromAccount( const KMail::ImapAccountBase * a ) {
    const SieveConfig sieve = a->sieveConfig();
    if ( !sieve.managesieveSupported() )
        return KURL();

    KURL u;
    if ( sieve.reuseConfig() ) {
        // assemble Sieve url from the settings of the account:
        u.setProtocol( "sieve" );
        u.setHost( a->host() );
        u.setUser( a->login() );
        u.setPass( a->passwd() );
        u.setPort( sieve.port() );

        // Translate IMAP LOGIN to PLAIN:
        u.addQueryItem( "x-mech", a->auth() == "*" ? "PLAIN" : a->auth() );
        if ( !a->useSSL() && !a->useTLS() )
            u.addQueryItem( "x-allow-unencrypted", "true" );
    } else {
        u = sieve.alternateURL();
        if ( u.protocol().lower() == "sieve" && !a->useSSL() && !a->useTLS() && u.queryItem("x-allow-unencrypted").isEmpty() )
            u.addQueryItem( "x-allow-unencrypted", "true" );
    }
    return u;
}

void SieveDebugDialog::slotDiagNextAccount()
{
    if ( mAccountList.isEmpty() )
        return;

    KMAccount *acc = mAccountList.first();
    mAccountList.pop_front();

    mEdit->append( i18n( "Collecting data for account '%1'...\n" ).arg( acc->name() ) );
    mEdit->append( i18n( "------------------------------------------------------------\n" ) );
    mAccountBase = dynamic_cast<KMail::ImapAccountBase *>( acc );
    if ( mAccountBase )
    {
        // Detect URL for this IMAP account
        const KURL url = urlFromAccount( mAccountBase );
        if ( !url.isValid() )
        {
            mEdit->append( i18n( "(Account does not support Sieve)\n\n" ) );
        } else {
            mUrl = url;

            mSieveJob = SieveJob::list( mUrl );

            connect( mSieveJob, TQT_SIGNAL( gotList( KMail::SieveJob *, bool, const TQStringList &, const TQString & ) ),
                TQT_SLOT( slotGetScriptList( KMail::SieveJob *, bool, const TQStringList &, const TQString & ) ) );

            // Bypass the singleShot timer -- it's fired when we get our data
            return;
        }
    } else {
        mEdit->append( i18n( "(Account is not an IMAP account)\n\n" ) );
    }

    // Handle next account async
    TQTimer::singleShot( 0, this, TQT_SLOT( slotDiagNextAccount() ) );
}

void SieveDebugDialog::slotDiagNextScript()
{
    if ( mScriptList.isEmpty() )
    {
        // Continue handling accounts instead
        mScriptList.clear();
        TQTimer::singleShot( 0, this, TQT_SLOT( slotDiagNextAccount() ) );
        return;
    }

    TQString scriptFile = mScriptList.first();
    mScriptList.pop_front();

    mEdit->append( i18n( "Contents of script '%1':\n" ).arg( scriptFile ) );

    mUrl = urlFromAccount( mAccountBase );
    mUrl.setFileName( scriptFile );

    mSieveJob = SieveJob::get( mUrl );

    connect( mSieveJob, TQT_SIGNAL( gotScript( KMail::SieveJob *, bool, const TQString &, bool ) ),
        TQT_SLOT( slotGetScript( KMail::SieveJob *, bool, const TQString &, bool ) ) );
}

void SieveDebugDialog::slotGetScript( SieveJob * /* job */, bool success,
    const TQString &script, bool active )
{
    kdDebug( 5006 ) << "SieveDebugDialog::slotGetScript( ??, " << success
              << ", ?, " << active << " )" << endl
              << "script:" << endl
              << script << endl;
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
            "------------------------------------------------------------\n\n" ).arg( script ) );
    }

    // Fetch next script
    TQTimer::singleShot( 0, this, TQT_SLOT( slotDiagNextScript() ) );
}

void SieveDebugDialog::slotGetScriptList( SieveJob *job, bool success,
    const TQStringList &scriptList, const TQString &activeScript )
{
    kdDebug( 5006 ) << k_funcinfo << "Success: " << success << ", List: " << scriptList.join( ", " ) <<
        ", active: " << activeScript << endl;
    mSieveJob = 0; // job deletes itself after returning from this slot!

    mEdit->append( i18n( "Sieve capabilities:\n" ) );
    TQStringList caps = job->sieveCapabilities();
    if ( caps.isEmpty() )
    {
        mEdit->append( i18n( "(No special capabilities available)" ) );
    }
    else
    {
        for ( TQStringList::const_iterator it = caps.begin(); it != caps.end(); ++it )
            mEdit->append( "* " + *it + "\n" );
        mEdit->append( "\n" );
    }

    mEdit->append( i18n( "Available Sieve scripts:\n" ) );

    if ( scriptList.isEmpty() )
    {
        mEdit->append( i18n( "(No Sieve scripts available on this server)\n\n" ) );
    }
    else
    {
        mScriptList = scriptList;
        for ( TQStringList::const_iterator it = scriptList.begin(); it != scriptList.end(); ++it )
            mEdit->append( "* " + *it + "\n" );
        mEdit->append( "\n" );
        mEdit->append( i18n( "Active script: %1\n\n" ).arg( activeScript ) );
    }

    // Handle next job: dump scripts for this server
    TQTimer::singleShot( 0, this, TQT_SLOT( slotDiagNextScript() ) );
}

void SieveDebugDialog::slotDialogOk()
{
    kdDebug(5006) << "SieveDebugDialog::slotDialogOk()" << endl;
}

void SieveDebugDialog::slotPutActiveResult( SieveJob * job, bool success )
{
    handlePutResult( job, success, true );
}

void SieveDebugDialog::slotPutInactiveResult( SieveJob * job, bool success )
{
    handlePutResult( job, success, false );
}

void SieveDebugDialog::handlePutResult( SieveJob *, bool success, bool activated )
{
    if ( success )
    {
        KMessageBox::information( 0, activated ? i18n(
            "Sieve script installed successfully on the server.\n"
            "Out of Office reply is now active." )
            : i18n( "Sieve script installed successfully on the server.\n"
            "Out of Office reply has been deactivated." ) );
    }

    kdDebug( 5006 ) << "SieveDebugDialog::handlePutResult( ???, " << success << ", ? )" << endl;
    mSieveJob = 0; // job deletes itself after returning from this slot!
}


} // namespace KMail

#include "sievedebugdialog.moc"

#endif // NDEBUG

