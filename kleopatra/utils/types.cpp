/* -*- mode: c++; c-basic-offset:4 -*-
    utils/types.cpp

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

#include "utils/types.h"


#include <QWidget>
#include <QVector>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

using namespace Kleo;
using namespace boost;

class ExecutionContextUser::Private {
    friend class ::Kleo::ExecutionContextUser;
    ExecutionContextUser * const q;
public:
    explicit Private( const shared_ptr<const ExecutionContext> & ctx, ExecutionContextUser * qq )
        : q( qq ),
          executionContext( ctx ),
          idApplied()
    {

    }

private:
    void applyWindowID( QWidget * w );

private:
    weak_ptr<const ExecutionContext> executionContext;
    QVector<QWidget*> idApplied;
};

void ExecutionContextUser::applyWindowID( QWidget * wid ) {
    if ( d->idApplied.contains( wid ) )
        return;
    if ( const shared_ptr<const ExecutionContext> ctx = d->executionContext.lock() ) {
        ctx->applyWindowID( wid );
        d->idApplied.append( wid );
    }
}

ExecutionContextUser::ExecutionContextUser()
    : d( new Private( shared_ptr<const ExecutionContext>(), this ) )
{

}

ExecutionContextUser::ExecutionContextUser( const shared_ptr<const ExecutionContext> & ctx )
    : d( new Private( ctx, this ) )
{

}

ExecutionContextUser::~ExecutionContextUser() {}


void ExecutionContextUser::setExecutionContext( const shared_ptr<const ExecutionContext> & ctx ) {
    d->executionContext = ctx;
    d->idApplied.clear();
}

shared_ptr<const ExecutionContext> ExecutionContextUser::executionContext() const {
    return d->executionContext.lock();
}

void ExecutionContextUser::bringToForeground( QWidget * wid ) {
    applyWindowID( wid );
    if ( wid->isVisible() )
        wid->raise();
    else
        wid->show();
#ifdef Q_OS_WIN
    SetForegroundWindow( wid->winId() );
#endif
}

