/*
    This file is part of Akregator.

    Copyright (C) 2008 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "command.h"

#include <QEventLoop>

using namespace Akregator;

class Command::Private
{
public:
    Private();    
    QWidget* m_parentWidget;
};

Command::Private::Private() : m_parentWidget( 0 )
{
}

Command::Command( QObject* parent ) : QObject( parent ), d( new Private )
{
    
}

Command::~Command()
{
    delete d;
}

QWidget* Command::parentWidget() const
{
    return d->m_parentWidget;
}

void Command::setParentWidget( QWidget* parentWidget )
{
    d->m_parentWidget = parentWidget;
}

void Command::start()
{
    doStart();
    emit started();
}

void Command::abort()
{
    doAbort();
}

void Command::done()
{
    emit finished();
    deleteLater();
}

void Command::waitForFinished()
{
    QEventLoop loop;
    connect( this, SIGNAL( finished() ), &loop, SLOT( quit() ) );
    loop.exec();    
}

#include "command.moc"
