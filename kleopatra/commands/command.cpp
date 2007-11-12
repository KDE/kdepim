/* -*- mode: c++; c-basic-offset:4 -*-
    commands/command.cpp

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

#include "command.h"
#include "command_p.h"

#include <QAbstractItemView>

using namespace Kleo;

Command::Private::Private( Command * qq, KeyListController * controller )
    : q( qq ),
      indexes_(),
      view_(),
      controller_( controller )
{

}

Command::Private::~Private() {}

Command::Command( KeyListController * p )
    : QObject( p ), d( new Private( this, p ) )
{

}

Command::Command( KeyListController * p, Private * pp )
    : QObject( p ), d( pp )
{

}

Command::~Command() {}



void Command::setView( QAbstractItemView * view ) {
    d->view_ = view;
}

void Command::setIndex( const QModelIndex & idx ) {
    d->indexes_.clear();
    d->indexes_.push_back( idx );
}

void Command::setIndexes( const QList<QModelIndex> & idx ) {
    d->indexes_ = idx;
}

void Command::start() {
    doStart();
}

void Command::cancel() {
    doCancel();
    emit canceled();
}

#include "moc_command.cpp"

