/* -*- mode: c++; c-basic-offset:4 -*-
    commands/refreshopenpgpcertscommand.cpp

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

#include "refreshopenpgpcertscommand.h"

#include <utils/gnupg-helper.h>

#include <KLocale>
#include <KMessageBox>

using namespace Kleo;
using namespace Kleo::Commands;

RefreshOpenPGPCertsCommand::RefreshOpenPGPCertsCommand( KeyListController * c )
    : GnuPGProcessCommand( c )
{

}

RefreshOpenPGPCertsCommand::RefreshOpenPGPCertsCommand( QAbstractItemView * v, KeyListController * c )
    : GnuPGProcessCommand( v, c )
{

}

RefreshOpenPGPCertsCommand::~RefreshOpenPGPCertsCommand() {}

bool RefreshOpenPGPCertsCommand::preStartHook( QWidget * parent ) const {
    return KMessageBox::warningContinueCancel( parent,
                                               i18n("Refreshing OpenPGP certificates implies downloading all certificates anew, "
                                                    "to check if any of them have been revoked in the meantime. "
                                                    "This can put a severe strain on your own as well as other people's network "
                                                    "connection, and can take up to an hour or more to complete, depending on "
                                                    "your network connection, and the number of certificates to check. "
                                                    "Are you sure you want to continue?"),
                                               i18n("OpenPGP Certitifcate Refresh"),
                                               KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                                               QLatin1String( "warn-refresh-openpgp-expensive" ) )
        == KMessageBox::Continue;
}

QStringList RefreshOpenPGPCertsCommand::arguments() const {
    return QStringList() << gpgPath() << "--refresh-keys";
}

QString RefreshOpenPGPCertsCommand::errorCaption() const {
    return i18n( "OpenPGP Certificate Refresh Error" );
}

QString RefreshOpenPGPCertsCommand::successCaption() const {
    return i18n( "OpenPGP Certificate Refresh Finished" );
}

QString RefreshOpenPGPCertsCommand::crashExitMessage( const QStringList & args ) const {
    return i18n( "The GPG process that tried to refresh OpenPGP certificates "
                 "ended prematurely because of an unexpected error. "
                 "Please check the output of %1 for details.", args.join( " " ) ) ;
}

QString RefreshOpenPGPCertsCommand::errorExitMessage( const QStringList & args ) const {
    return i18n( "An error occurred while trying to refresh OpenPGP certificates. "
                 "The output from %1 was:\n%2", args[0], errorString() );
}

QString RefreshOpenPGPCertsCommand::successMessage( const QStringList & ) const {
    return i18n( "OpenPGP certificates refreshed successfully." );
    // ### --check-trustdb
}

#include "moc_refreshopenpgpcertscommand.cpp"
