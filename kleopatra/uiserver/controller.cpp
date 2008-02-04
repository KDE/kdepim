/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/controller.cpp

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

#include "controller.h"

#include "assuancommand.h"

#include <QDialog>
#include <QVector>

#include <KWindowSystem>

using namespace boost;
using namespace Kleo;

class Controller::Private {
    friend class ::Kleo::Controller;
    Controller * const q;
public:
    explicit Private( const shared_ptr<AssuanCommand> & cmd, Controller * qq );
    
    void applyWindowID( QDialog* dlg );
    
private:
    weak_ptr<AssuanCommand> command;
    QVector<QDialog*> idApplied;
    
};

Controller::Private::Private( const shared_ptr<AssuanCommand> & cmd, Controller * qq )
    : q( qq ), command( cmd )
{

}

void Controller::Private::applyWindowID( QDialog* dlg )
{
    if ( idApplied.contains( dlg ) )
        return;
    const shared_ptr<AssuanCommand> cmd = command.lock();
    if ( cmd ) {
        cmd->applyWindowID( dlg );
        idApplied.append( dlg );
    }
}

Controller::Controller( const shared_ptr<AssuanCommand> & cmd, QObject* parent )
    : QObject( parent ), d( new Private( cmd, this ) )
{
    
}

Controller::~Controller()
{
    
}

void Controller::setCommand( const shared_ptr<AssuanCommand> & cmd )
{
    d->command = cmd;
    d->idApplied.clear();
}

weak_ptr<AssuanCommand> Controller::command() const
{
    return d->command;
}

void Controller::bringToForeground( QDialog* dlg )
{
    d->applyWindowID( dlg );
    if ( dlg->isVisible() )
        dlg->raise();
    else
        dlg->show();
#if Q_WS_WIN
    KWindowSystem::forceActiveWindow( dlg->winId() );
#endif
}

#include "controller.moc"
