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

#include "detailscommand.h"
#include "command_p.h"

#include "../certificateinfowidgetimpl.h"

#include <KDialog>

#include <QAbstractItemView>

using namespace Kleo;

class DetailsCommand::Private : public Command::Private {
    friend class ::Kleo::DetailsCommand;
public:
    Private( DetailsCommand * qq, KeyListController * controller );
    ~Private();

private:
    QPointer<KDialog> dialog;
};

DetailsCommand::Private * DetailsCommand::d_func() { return static_cast<Private*>( d.get() ); }
const DetailsCommand::Private * DetailsCommand::d_func() const { return static_cast<const Private*>( d.get() ); }


DetailsCommand::Private::Private( DetailsCommand * qq, KeyListController* controller )
    : Command::Private( qq, controller ), dialog()
{

}

DetailsCommand::Private::~Private() {}



DetailsCommand::DetailsCommand( KeyListController * p )
    : Command( p, new Private( this, p ) )
{

}

#define d d_func()

DetailsCommand::~DetailsCommand() {
    if ( d->dialog )
        d->dialog->close();
}

void DetailsCommand::doStart() {
    if ( d->indexes().count() != 1 ) {
        qWarning( "DetailsCommand::doStart: can only work with one certificate at a time" );
        return;
    }

    if ( !d->dialog ) {
        d->dialog = CertificateInfoWidgetImpl::createDialog( d->key(), d->view() );
    }
    if ( d->dialog->isVisible() )
        d->dialog->raise();
    else
        d->dialog->show();
}


void DetailsCommand::doCancel() {
    if ( d->dialog )
        d->dialog->close();
}

#undef d

#include "moc_detailscommand.cpp"
