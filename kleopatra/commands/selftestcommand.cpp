/* -*- mode: c++; c-basic-offset:4 -*-
    commands/selftestcommand.cpp

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

#include "selftestcommand.h"

#include "command_p.h"

#include <dialogs/selftestdialog.h>

#include <selftest/enginecheck.h>
#ifdef Q_OS_WIN
# include <selftest/registrycheck.h>
#endif

#include <utils/stl_util.h>

#include <KLocale>
#include <KGlobal>
#include <KConfigGroup>

#include <boost/shared_ptr.hpp>
#include <boost/mem_fn.hpp>

#include <vector>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace Kleo::Dialogs;
using namespace boost;

class SelfTestCommand::Private : Command::Private {
    friend class ::Kleo::Commands::SelfTestCommand;
    SelfTestCommand * q_func() const { return static_cast<SelfTestCommand*>( q ); }
public:
    explicit Private( SelfTestCommand * qq, KeyListController * c );
    ~Private();

private:
    void init();

    void ensureDialogCreated() {
        if ( dialog )
            return;
        dialog = new SelfTestDialog( view() );
        dialog->setAttribute( Qt::WA_DeleteOnClose );

        connect( dialog, SIGNAL(updateRequested()),
                 q, SLOT(slotUpdateRequested()) );
        connect( dialog, SIGNAL(accepted()),
                 q, SLOT(slotDialogAccepted()) );
        connect( dialog, SIGNAL(rejected()),
                 q, SLOT(slotDialogRejected()) );

        dialog->setRunAtStartUp( runAtStartUp() );
    }

    void ensureDialogShown() {
        ensureDialogCreated();
        if ( dialog->isVisible() )
            dialog->raise();
        else
            dialog->show();
    }

    bool runAtStartUp() const {
        const KConfigGroup config( KGlobal::config(), "Self-Test" );
        return config.readEntry( "run-at-startup", true );
    }

    void setRunAtStartUp( bool on ) {
        KConfigGroup config( KGlobal::config(), "Self-Test" );
        config.writeEntry( "run-at-startup", on );
    }

    void runTests() {
        std::vector< shared_ptr<Kleo::SelfTest> > tests;

        //emit q->info( i18n("Checking gpg installation...") );
        tests.push_back( makeGpgEngineCheckSelfTest() );
        //emit q->info( i18n("Checking gpgsm installation...") );
        tests.push_back( makeGpgSmEngineCheckSelfTest() );
        //emit q->info( i18n("Checking gpgconf installation...") );
        tests.push_back( makeGpgConfEngineCheckSelfTest() );
#ifdef Q_OS_WIN
        //emit q->info( i18n("Checking Windows Registry...") );
        tests.push_back( makeGpgProgramRegistryCheckSelfTest() );
#endif

        if ( !dialog && kdtools::all( tests, mem_fn( &Kleo::SelfTest::passed ) ) ) {
            finished();
            return;
        }

        ensureDialogCreated();

        dialog->clear();
        dialog->addSelfTests( tests );

        ensureDialogShown();
    }

private:
    void slotDialogAccepted() {
        setRunAtStartUp( dialog->runAtStartUp() );
        finished();
    }
    void slotDialogRejected() {
        canceled = true;
        Command::Private::canceled();
    }
    void slotUpdateRequested() {
        runTests();
    }

private:
    QPointer<SelfTestDialog> dialog;
    bool canceled;
    bool automatic;
};

SelfTestCommand::Private * SelfTestCommand::d_func() { return static_cast<Private*>( d.get() ); }
const SelfTestCommand::Private * SelfTestCommand::d_func() const { return static_cast<const Private*>( d.get() ); }

#define d d_func()
#define q q_func()

SelfTestCommand::Private::Private( SelfTestCommand * qq, KeyListController * c )
    : Command::Private( qq, c ),
      canceled( false ),
      automatic( false )
{

}

SelfTestCommand::Private::~Private() {

}

SelfTestCommand::SelfTestCommand( KeyListController * c )
    : Command( new Private( this, c ) )
{
    d->init();
}

SelfTestCommand::SelfTestCommand( QAbstractItemView * v, KeyListController * c )
    : Command( v, new Private( this, c ) )
{
    d->init();
}

void SelfTestCommand::Private::init() {

}

SelfTestCommand::~SelfTestCommand() {}

void SelfTestCommand::setAutomaticMode( bool on ) {
    d->automatic = on;
    if ( d->dialog )
        d->dialog->setAutomaticMode( on );
}

bool SelfTestCommand::isCanceled() const {
    return d->canceled;
}

void SelfTestCommand::doStart() {

    if ( d->automatic ) {
        if ( !d->runAtStartUp() ) {
            d->finished();
            return;
        }
        d->ensureDialogCreated();
    }

    d->runTests();

}

void SelfTestCommand::doCancel() {
    d->canceled = true;
    if ( d->dialog )
        d->dialog->close();
    d->dialog = 0;
}

#undef d
#undef q

#include "moc_selftestcommand.cpp"
