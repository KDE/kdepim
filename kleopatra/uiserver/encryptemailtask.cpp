/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/encryptemailtask.cpp

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

#include "encryptemailtask.h"

#include "kleo-assuan.h"

#include "input.h"
#include "output.h"

#include <utils/stl_util.h>

#include <KLocale>

#include <boost/bind.hpp>

using namespace Kleo;
using namespace boost;
using namespace GpgME;

class EncryptEMailTask::Private {
    friend class ::Kleo::EncryptEMailTask;
    EncryptEMailTask * const q;
public:
    explicit Private( EncryptEMailTask * qq );

private:
    // ### 
};

EncryptEMailTask::Private::Private( EncryptEMailTask * qq )
    : q( qq )
{

}

EncryptEMailTask::EncryptEMailTask( QObject * p )
    : Task( p ), d( new Private( this ) )
{

}

EncryptEMailTask::~EncryptEMailTask() {}

void EncryptEMailTask::setInput( const shared_ptr<Input> & input ) {
    notImplemented();
}

void EncryptEMailTask::setOutput( const shared_ptr<Output> & output ) {
    notImplemented();
}

void EncryptEMailTask::setRecipients( const std::vector<GpgME::Key> & recipients ) {
    notImplemented();
}

Protocol EncryptEMailTask::protocol() const {
    // ### d->recipients.front()->protocol() + error handling
    notImplemented();
}

void EncryptEMailTask::start() {
    notImplemented();
}


#include "moc_encryptemailtask.cpp"


