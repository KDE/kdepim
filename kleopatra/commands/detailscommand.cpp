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

#include "../certificateinfowidgetimpl.h"

#include <KDialog>

#include <QAbstractItemView>

#include <cassert>

using namespace Kleo;
//using namespace Kleo::Commands;
using namespace GpgME;

class DetailsCommand::Private : public Command::Private {
    friend class ::DetailsCommand;
    DetailsCommand * q_func() const { return static_cast<DetailsCommand*>( q ); }
public:
    explicit Private( DetailsCommand * qq, KeyListController * c );
    ~Private();

    void init();

private:
    Key key;
};

DetailsCommand::Private * DetailsCommand::d_func() { return static_cast<Private*>( d.get() ); }
const DetailsCommand::Private * DetailsCommand::d_func() const { return static_cast<const Private*>( d.get() ); }

#define q q_func()
#define d d_func()

DetailsCommand::Private::Private( DetailsCommand * qq, KeyListController * c )
    : Command::Private( qq, c ),
      key()
{

}

DetailsCommand::Private::~Private() {}

DetailsCommand::DetailsCommand( KeyListController * p )
    : Command( new Private( this, p ) )
{

}

DetailsCommand::DetailsCommand( QAbstractItemView * v, KeyListController * p )
    : Command( v, new Private( this, p ) )
{

}

DetailsCommand::DetailsCommand( const Key & key, KeyListController * p )
    : Command( new Private( this, p ) )
{
    assert( !key.isNull() );
    d->key = key;
}

DetailsCommand::DetailsCommand( const Key & key, QAbstractItemView * v, KeyListController * p )
    : Command( v, new Private( this, p ) )
{
    assert( !key.isNull() );
    d->key = key;
}

DetailsCommand::~DetailsCommand() {}

void DetailsCommand::doStart() {
    Key key;
    if ( !d->key.isNull() )
        key = d->key;
    else if ( d->indexes().size() == 1 )
        key = d->Command::Private::key();
    else
        qWarning( "DetailsCommand::doStart: can only work with one certificate at a time" );

    if ( !key.isNull() ) {
        KDialog * const dlg = CertificateInfoWidgetImpl::createDialog( key, d->view() );
        assert( dlg );
        dlg->setAttribute( Qt::WA_DeleteOnClose );
        dlg->show();
    }

    d->finished();
}


void DetailsCommand::doCancel() {}

#undef q_func
#undef d_func

#include "moc_detailscommand.cpp"
