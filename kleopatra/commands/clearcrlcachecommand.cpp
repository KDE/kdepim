/* -*- mode: c++; c-basic-offset:4 -*-
    commands/clearcrlcachecommand.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "clearcrlcachecommand.h"

#include "command_p.h"

#include <QString>
#include <QByteArray>
#include <QTimer>

#include <KProcess>
#include <KMessageBox>
#include <KLocale>

static const int PROCESS_TERMINATE_TIMEOUT = 5000; // milliseconds

using namespace Kleo;
using namespace Kleo::Commands;

class ClearCrlCacheCommand::Private : Command::Private {
    friend class ::Kleo::Commands::ClearCrlCacheCommand;
    ClearCrlCacheCommand * q_func() const { return static_cast<ClearCrlCacheCommand*>( q ); }
public:
    explicit Private( ClearCrlCacheCommand * qq, KeyListController * c );
    ~Private();

    QString errorString() const {
        return QString::fromLocal8Bit( errorBuffer );
    }

private:
    void init();

private:
    void slotProcessFinished( int, QProcess::ExitStatus );
    void slotProcessReadyReadStandardError();

private:
    KProcess process;
    QByteArray errorBuffer;
    bool canceled;
};

ClearCrlCacheCommand::Private * ClearCrlCacheCommand::d_func() { return static_cast<Private*>( d.get() ); }
const ClearCrlCacheCommand::Private * ClearCrlCacheCommand::d_func() const { return static_cast<const Private*>( d.get() ); }

#define d d_func()
#define q q_func()

ClearCrlCacheCommand::Private::Private( ClearCrlCacheCommand * qq, KeyListController * c )
    : Command::Private( qq, c ),
      process(),
      errorBuffer(),
      canceled( false )
{
    process.setOutputChannelMode( KProcess::OnlyStderrChannel );
    process << "dirmngr" << "--flush";
    //process << "gpgsm" << "--call-dirmngr" << "flush";
}

ClearCrlCacheCommand::Private::~Private() {}

ClearCrlCacheCommand::ClearCrlCacheCommand( KeyListController * c )
    : Command( new Private( this, c ) )
{
    d->init();
}

ClearCrlCacheCommand::ClearCrlCacheCommand( QAbstractItemView * v, KeyListController * c )
    : Command( v, new Private( this, c ) )
{
    d->init();
}

void ClearCrlCacheCommand::Private::init() {
    connect( &process, SIGNAL(finished(int,QProcess::ExitStatus)),
             q, SLOT(slotProcessFinished(int,QProcess::ExitStatus)) );
    connect( &process, SIGNAL(readyReadStandardError()),
             q, SLOT(slotProcessReadyReadStandardError()) );
}

ClearCrlCacheCommand::~ClearCrlCacheCommand() {}

void ClearCrlCacheCommand::doStart() {

    d->process.start();

    if ( !d->process.waitForStarted() ) {
        KMessageBox::error( d->view(),
                            i18n( "Unable to start process dirmngr. "
                                  "Please check your installation." ),
                            i18n( "Clear CRL Cache Error" ) );
        finished();
    }
}

void ClearCrlCacheCommand::doCancel() {
    d->canceled = true;
    if ( d->process.state() != QProcess::NotRunning ) {
        d->process.terminate();
        QTimer::singleShot( PROCESS_TERMINATE_TIMEOUT, &d->process, SLOT(kill()) );
    }
}

void ClearCrlCacheCommand::Private::slotProcessFinished( int code, QProcess::ExitStatus status ) {
    if ( canceled )
        /* be silent */ ;
    else if ( status == QProcess::CrashExit )
        KMessageBox::error( view(),
                            i18n( "The DirMngr process that tried to clear the CRL cache "
                                  "ended prematurely because of an unexpected error. "
                                  "Please check the output of dirmngr --flush for details." ),
                            i18n( "Clear CRL Cache Error" ) );
    else if ( code )
        KMessageBox::error( view(),
                            i18n( "An error occurred while trying to clear the CRL cache. "
                                  "The output from dirmngr was:\n%1", errorString() ),
                            i18n( "Clear CRL Cache Error" ) );
    else
        KMessageBox::information( view(),
                                  i18n( "CRL cache cleared successfully." ),
                                  i18n( "Clear CRL Cache Finished" ) );
    finished();
}

void ClearCrlCacheCommand::Private::slotProcessReadyReadStandardError() {
    errorBuffer += process.readAllStandardError();
}

#undef d
#undef q

#include "moc_clearcrlcachecommand.cpp"
