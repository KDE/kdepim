/* -*- mode: c++; c-basic-offset:4 -*-
    command.cpp

    This file is part of KleopatraClient, the Kleopatra interface library
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    KleopatraClient is free software; you can redistribute it and/or modify
    it under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    KleopatraClient is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <config-kleopatra.h>

#include "command.h"
#include "command_p.h"

#include <QMutexLocker>

#include <assuan.h>
#include <gpg-error.h>

#include <algorithm>

using namespace KleopatraClient;

Command::Command( QObject * p )
    : QObject( p ), d( new Private( this ) )
{
    d->init();
}

Command::Command( Private * pp, QObject * p )
    : QObject( p ), d( pp )
{
    d->init();
}

Command::~Command() {
    delete d; d = 0;
}

void Command::Private::init() {
    connect( this, SIGNAL(started()),  q, SIGNAL(started())  );
    connect( this, SIGNAL(finished()), q, SIGNAL(finished()) );
}

void Command::setParentWId( WId wid ) {
    const QMutexLocker locker( &d->mutex );
    d->parentWId = wid;
}

WId Command::parentWId() const {
    const QMutexLocker locker( &d->mutex );
    return d->parentWId;
}


bool Command::waitForFinished() {
    return d->wait();
}

bool Command::waitForFinished( unsigned long ms ) {
    return d->wait( ms );
}


bool Command::error() const {
    const QMutexLocker locker( &d->mutex );
    return !d->errorString.isEmpty();
}

QString Command::errorString() const {
    const QMutexLocker locker( &d->mutex );
    return d->errorString;
}


qint64 Command::serverPid() const {
    const QMutexLocker locker( &d->mutex );
    return d->serverPid;
}


void Command::start() {
    d->start();
}

void Command::cancel() {
    qDebug( "Sorry, not implemented: KleopatraClient::Command::Cancel" );
}


void Command::setOptionValue( const char * name, const QVariant & value ) {
    if ( !name || !*name )
        return;
    const QMutexLocker locker( &d->mutex );
    d->valueOptions[name] = value;
}

QVariant Command::optionValue( const char * name ) const {
    if ( !name || !*name )
        return QVariant();
    const QMutexLocker locker( &d->mutex );

    const std::map<std::string,QVariant>::const_iterator it = d->valueOptions.find( name );
    if ( it == d->valueOptions.end() )
        return QVariant();
    else
        return it->second;
}


void Command::setOption( const char * name ) {
    if ( !name || !*name )
        return;
    const QMutexLocker locker( &d->mutex );

    if ( isOptionSet( name ) )
        unsetOption( name );

    d->nonValueOptions.push_back( name );
}

void Command::unsetOption( const char * name ) {
    if ( !name || !*name )
        return;
    const QMutexLocker locker( &d->mutex );
    d->nonValueOptions.erase( std::remove( d->nonValueOptions.begin(), d->nonValueOptions.end(), name ),
                              d->nonValueOptions.end() );
}

bool Command::isOptionSet( const char * name ) const {
    if ( !name || !*name )
        return false;
    const QMutexLocker locker( &d->mutex );
    return std::find( d->nonValueOptions.begin(), d->nonValueOptions.end(), name )
        != d->nonValueOptions.end() ;
}


QByteArray Command::receivedData() const {
    const QMutexLocker locker( &d->mutex );
    return d->data;
}


void Command::setCommand( const char * command ) {
    const QMutexLocker locker( &d->mutex );
    d->command = command;
}

QByteArray Command::command() const {
    const QMutexLocker locker( &d->mutex );
    return d->command;
}

void Command::Private::run() {

    const QMutexLocker locker( &mutex );
    errorString = tr("Not Implemented");

}

#include "moc_command_p.cpp"
#include "moc_command.cpp"
