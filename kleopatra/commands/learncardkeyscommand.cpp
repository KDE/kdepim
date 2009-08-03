/* -*- mode: c++; c-basic-offset:4 -*-
    commands/learncardkeyscommand.cpp

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

#include "learncardkeyscommand.h"

#include "command_p.h"

#include <smartcard/readerstatus.h>

#include <utils/gnupg-helper.h>

#include <gpgme++/key.h>

#include <KLocale>
#include <KMessageBox>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace GpgME;

LearnCardKeysCommand::LearnCardKeysCommand( GpgME::Protocol proto )
    : GnuPGProcessCommand( 0 ), m_protocol( proto )
{

}

LearnCardKeysCommand::~LearnCardKeysCommand() {}

Protocol LearnCardKeysCommand::protocol() const {
    return m_protocol;
}

QStringList LearnCardKeysCommand::arguments() const {
    if ( protocol() == OpenPGP )
        return QStringList() << gpgPath() << "--learn-card" << "-v";
    else
        return QStringList() << gpgSmPath() << "--learn-card" << "-v";
}

QString LearnCardKeysCommand::errorCaption() const {
    return i18n( "Error Learning SmartCard" );
}

QString LearnCardKeysCommand::successCaption() const {
    return i18n( "Finished Learning SmartCard" );
}

QString LearnCardKeysCommand::crashExitMessage( const QStringList & args ) const {
    return i18nc("@info",
                 "<para>The GPG or GpgSM process that tried to learn the smart card "
                 "ended prematurely because of an unexpected error.</para>"
                 "<para>Please check the output of <icode>%1</icode> for details.</para>", args.join( " " ) ) ;
}

QString LearnCardKeysCommand::errorExitMessage( const QStringList & args ) const {
    return i18nc("@info",
                 "<para>An error occurred while trying to learn the smart card's keys.</para> "
                 "<para>The output from <command>%1</command> was: <message>%2</message></para>",
                 args[0], errorString() );
}

QString LearnCardKeysCommand::successMessage( const QStringList & ) const {
    return i18n( "Smart card keys successfully learned." );
}

void LearnCardKeysCommand::postSuccessHook( QWidget * ) {
    SmartCard::ReaderStatus::mutableInstance()->updateStatus();
}

#include "moc_learncardkeyscommand.cpp"
