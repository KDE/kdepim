/* -*- mode: c++; c-basic-offset:4 -*-
    commands/gnupgprocesscommand.cpp

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

#include <config-kleopatra.h>

#include "gnupgprocesscommand.h"

#include "command_p.h"

#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QTimer>

#include <KProcess>
#include <KMessageBox>
#include <KLocale>

static const int PROCESS_TERMINATE_TIMEOUT = 5000; // milliseconds

using namespace Kleo;
using namespace Kleo::Commands;

class GnuPGProcessCommand::Private : Command::Private {
    friend class ::Kleo::Commands::GnuPGProcessCommand;
    GnuPGProcessCommand * q_func() const { return static_cast<GnuPGProcessCommand*>( q ); }
public:
    explicit Private( GnuPGProcessCommand * qq, KeyListController * c );
    ~Private();

private:
    void init();

private:
    void slotProcessFinished( int, QProcess::ExitStatus );
    void slotProcessReadyReadStandardError();

private:
    KProcess process;
    QStringList arguments;
    QByteArray errorBuffer;
    bool canceled;
};

GnuPGProcessCommand::Private * GnuPGProcessCommand::d_func() { return static_cast<Private*>( d.get() ); }
const GnuPGProcessCommand::Private * GnuPGProcessCommand::d_func() const { return static_cast<const Private*>( d.get() ); }

#define d d_func()
#define q q_func()

GnuPGProcessCommand::Private::Private( GnuPGProcessCommand * qq, KeyListController * c )
    : Command::Private( qq, c ),
      process(),
      errorBuffer(),
      canceled( false )
{
    process.setOutputChannelMode( KProcess::OnlyStderrChannel );
}

GnuPGProcessCommand::Private::~Private() {}

GnuPGProcessCommand::GnuPGProcessCommand( KeyListController * c )
    : Command( new Private( this, c ) )
{
    d->init();
}

GnuPGProcessCommand::GnuPGProcessCommand( QAbstractItemView * v, KeyListController * c )
    : Command( v, new Private( this, c ) )
{
    d->init();
}

GnuPGProcessCommand::GnuPGProcessCommand( const GpgME::Key & key )
    : Command( key, new Private( this, 0 ) )
{
    d->init();
}

void GnuPGProcessCommand::Private::init() {
    connect( &process, SIGNAL(finished(int,QProcess::ExitStatus)),
             q, SLOT(slotProcessFinished(int,QProcess::ExitStatus)) );
    connect( &process, SIGNAL(readyReadStandardError()),
             q, SLOT(slotProcessReadyReadStandardError()) );
}

GnuPGProcessCommand::~GnuPGProcessCommand() {}

bool GnuPGProcessCommand::preStartHook( QWidget * ) const {
    return true;
}

void GnuPGProcessCommand::doStart() {

    if ( !preStartHook( d->view() ) ) {
        d->finished();
        return;
    }

    d->arguments = arguments();

    d->process << d->arguments;

    d->process.start();

    if ( !d->process.waitForStarted() ) {
        KMessageBox::error( d->view(),
                            i18n( "Unable to start process %1. "
                                  "Please check your installation.", d->arguments[0] ),
                            errorCaption() );
        d->finished();
    }
}

void GnuPGProcessCommand::doCancel() {
    d->canceled = true;
    if ( d->process.state() != QProcess::NotRunning ) {
        d->process.terminate();
        QTimer::singleShot( PROCESS_TERMINATE_TIMEOUT, &d->process, SLOT(kill()) );
    }
}

void GnuPGProcessCommand::Private::slotProcessFinished( int code, QProcess::ExitStatus status ) {
    if ( !canceled )
        if ( status == QProcess::CrashExit )
            KMessageBox::error( view(), q->crashExitMessage( arguments ), q->errorCaption() );
        else if ( code )
            KMessageBox::error( view(), q->errorExitMessage( arguments ), q->errorCaption() );
        else
            KMessageBox::information( view(), q->successMessage( arguments ), q->successCaption() );
    finished();
}

void GnuPGProcessCommand::Private::slotProcessReadyReadStandardError() {
    errorBuffer += process.readAllStandardError();
}

QString GnuPGProcessCommand::errorString() const {
    return QString::fromLocal8Bit( d->errorBuffer );
}

#undef d
#undef q

#include "moc_gnupgprocesscommand.cpp"
