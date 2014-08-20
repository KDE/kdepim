/* -*- mode: c++; c-basic-offset:4 -*-
    commands/detailscommand.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

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

#include "detailscommand.h"
#include "command_p.h"

#include <dialogs/certificatedetailsdialog.h>

#include <QDebug>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace Kleo::Dialogs;
using namespace GpgME;

class DetailsCommand::Private : public Command::Private {
    friend class ::Kleo::Commands::DetailsCommand;
    DetailsCommand * q_func() const { return static_cast<DetailsCommand*>( q ); }
public:
    explicit Private( DetailsCommand * qq, KeyListController * c );
    ~Private();

private:
    void ensureDialogCreated() {
        if ( dialog )
            return;

        CertificateDetailsDialog * dlg = new CertificateDetailsDialog;
        applyWindowID( dlg );
        dlg->setAttribute( Qt::WA_DeleteOnClose );
        connect( dlg, SIGNAL(rejected()), q_func(), SLOT(slotDialogClosed()) );

        dialog = dlg;
    }

    void ensureDialogVisible() {
        ensureDialogCreated();
        if ( dialog->isVisible() )
            dialog->raise();
        else
            dialog->show();
    }

    void init() {
        q->setWarnWhenRunningAtShutdown( false );
    }

private:
    void slotDialogClosed();

private:
    QPointer<CertificateDetailsDialog> dialog;
};

DetailsCommand::Private * DetailsCommand::d_func() { return static_cast<Private*>( d.get() ); }
const DetailsCommand::Private * DetailsCommand::d_func() const { return static_cast<const Private*>( d.get() ); }

#define q q_func()
#define d d_func()

DetailsCommand::Private::Private( DetailsCommand * qq, KeyListController * c )
    : Command::Private( qq, c ),
      dialog()
{

}

DetailsCommand::Private::~Private() {}

DetailsCommand::DetailsCommand( KeyListController * p )
    : Command( new Private( this, p ) )
{
    d->init();
}

DetailsCommand::DetailsCommand( QAbstractItemView * v, KeyListController * p )
    : Command( v, new Private( this, p ) )
{
    d->init();
}

DetailsCommand::DetailsCommand( const Key & key, KeyListController * p )
    : Command( new Private( this, p ) )
{
    assert( !key.isNull() );
    d->init();
    setKey( key );
}

DetailsCommand::DetailsCommand( const Key & key, QAbstractItemView * v, KeyListController * p )
    : Command( v, new Private( this, p ) )
{
    assert( !key.isNull() );
    d->init();
    setKey( key );
}

DetailsCommand::~DetailsCommand() {}

void DetailsCommand::doStart() {
    const std::vector<Key> keys = d->keys();
    Key key;
    if ( keys.size() == 1 )
        key = keys.front();
    else
        qWarning() << "can only work with one certificate at a time";

    if ( key.isNull() ) {
        d->finished();
        return;
    }

    d->ensureDialogCreated();

    d->dialog->setKey( key );

    d->ensureDialogVisible();
}


void DetailsCommand::doCancel() {
    if ( d->dialog )
        d->dialog->close();
}

void DetailsCommand::Private::slotDialogClosed() {
    finished();
}


#undef q_func
#undef d_func

#include "moc_detailscommand.cpp"
