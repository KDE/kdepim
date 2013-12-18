/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/createchecksumscommand.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2010 Klarälvdalens Datakonsult AB

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

#include "createchecksumscommand.h"

#include <crypto/createchecksumscontroller.h>

#include <kleo/exception.h>

#include <KLocalizedString>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace boost;

class CreateChecksumsCommand::Private {
private:
    friend class ::Kleo::CreateChecksumsCommand;
    CreateChecksumsCommand * const q;
public:
    explicit Private( CreateChecksumsCommand * qq )
        : q( qq ),
          controller()
    {

    }

private:
    void checkForErrors() const;

private:
    shared_ptr<CreateChecksumsController> controller;
};

CreateChecksumsCommand::CreateChecksumsCommand()
    : AssuanCommandMixin<CreateChecksumsCommand>(), d( new Private( this ) )
{

}

CreateChecksumsCommand::~CreateChecksumsCommand() {}

void CreateChecksumsCommand::Private::checkForErrors() const {

    if ( !q->numFiles() )
        throw Exception( makeError( GPG_ERR_ASS_NO_INPUT ),
                         i18n("At least one FILE must be present") );

}

int CreateChecksumsCommand::doStart() {

    d->checkForErrors();

    d->controller.reset( new CreateChecksumsController( shared_from_this() ) );

    d->controller->setAllowAddition( hasOption( "allow-addition" ) );

    d->controller->setFiles( fileNames() );

    connect( d->controller.get(), SIGNAL(done()),
             this, SLOT(done()), Qt::QueuedConnection );
    connect( d->controller.get(), SIGNAL(error(int,QString)),
             this, SLOT(done(int,QString)), Qt::QueuedConnection );

    d->controller->start();

    return 0;
}

void CreateChecksumsCommand::doCanceled() {
    if ( d->controller )
        d->controller->cancel();
}

