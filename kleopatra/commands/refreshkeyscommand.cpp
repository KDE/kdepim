/* -*- mode: c++; c-basic-offset:4 -*-
    refreshkeyscommand.cpp

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

#include "refreshkeyscommand.h"
#include "command_p.h"

#include <models/keycache.h>

#include <gpgme++/keylistresult.h>

#include <boost/bind.hpp>

using namespace Kleo;
using namespace boost;
using namespace GpgME;

class RefreshKeysCommand::Private : public Command::Private {
    friend class ::Kleo::RefreshKeysCommand;
public:
    Private( RefreshKeysCommand * qq, KeyListController* controller );
    ~Private();

    void keyListingDone( const KeyListResult & result );
};

RefreshKeysCommand::Private * RefreshKeysCommand::d_func() { return static_cast<Private*>( d.get() ); }
const RefreshKeysCommand::Private * RefreshKeysCommand::d_func() const { return static_cast<const Private*>( d.get() ); }


RefreshKeysCommand::RefreshKeysCommand( KeyListController * p )
    : Command( new Private( this, p ) )
{

}

RefreshKeysCommand::RefreshKeysCommand( QAbstractItemView * v, KeyListController * p )
    : Command( v, new Private( this, p ) )
{

}

RefreshKeysCommand::~RefreshKeysCommand() {}

RefreshKeysCommand::Private::Private( RefreshKeysCommand * qq, KeyListController * controller )
    : Command::Private( qq, controller )
{
}

RefreshKeysCommand::Private::~Private() {}

void RefreshKeysCommand::Private::keyListingDone( const KeyListResult & result ) {
    //TODO: handle result
    finished();
}

#define d d_func()

void RefreshKeysCommand::doStart() {
    connect( KeyCache::mutableInstance().get(), SIGNAL(keyListingDone(GpgME::KeyListResult)),
             this, SLOT(keyListingDone(GpgME::KeyListResult)) );
    KeyCache::mutableInstance()->startKeyListing();
}

void RefreshKeysCommand::doCancel() {
    KeyCache::mutableInstance()->cancelKeyListing();
}

#undef d

#include "moc_refreshkeyscommand.cpp"
