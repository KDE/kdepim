/* -*- mode: c++; c-basic-offset:4 -*-
    commands/refreshx509certscommand.cpp

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

#include "refreshx509certscommand.h"

#include <utils/gnupg-helper.h>

#include <KLocale>
#include <KMessageBox>

using namespace Kleo;
using namespace Kleo::Commands;

RefreshX509CertsCommand::RefreshX509CertsCommand( KeyListController * c )
    : GnuPGProcessCommand( c )
{

}

RefreshX509CertsCommand::RefreshX509CertsCommand( QAbstractItemView * v, KeyListController * c )
    : GnuPGProcessCommand( v, c )
{

}

RefreshX509CertsCommand::~RefreshX509CertsCommand() {}

bool RefreshX509CertsCommand::preStartHook( QWidget * parent ) const {
    return KMessageBox::warningContinueCancel( parent,
                                               i18nc("@info",
                                                     "<para>Refreshing X.509 certificates implies downloading CRLs for all certificates, "
                                                     "even if they might otherwise still be valid.</para>"
                                                     "<para>This can put a severe strain on your own as well as other people's network "
                                                     "connections, and can take up to an hour or more to complete, depending on "
                                                     "your network connection, and the number of certificates to check.</para> "
                                                     "<para>Are you sure you want to continue?</para>"),
                                               i18nc("@title:window", "X.509 Certificate Refresh"),
                                               KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                                               QLatin1String( "warn-refresh-x509-expensive" ) )
        == KMessageBox::Continue;
}

QStringList RefreshX509CertsCommand::arguments() const {
    return QStringList() << gpgSmPath() << QLatin1String("-k") << QLatin1String("--with-validation") << QLatin1String("--force-crl-refresh") << QLatin1String("--enable-crl-checks");
}

QString RefreshX509CertsCommand::errorCaption() const {
    return i18nc( "@title:window", "X.509 Certificate Refresh Error" );
}

QString RefreshX509CertsCommand::successCaption() const {
    return i18nc( "@title:window", "X.509 Certificate Refresh Finished" );
}

QString RefreshX509CertsCommand::crashExitMessage( const QStringList & args ) const {
    return i18nc( "@info",
                  "<para>The GpgSM process that tried to refresh X.509 certificates "
                  "ended prematurely because of an unexpected error.</para>"
                  "<para>Please check the output of <icode>%1</icode> for details.</para>", args.join( QLatin1String(" ") ) ) ;
}

QString RefreshX509CertsCommand::errorExitMessage( const QStringList & args ) const {
    return i18nc( "@info",
                  "<para>An error occurred while trying to refresh X.509 certificates.</para>"
                  "<para>The output from <command>%1</command> was: <message>%2</message></para>",
                 args[0], errorString() );
}

QString RefreshX509CertsCommand::successMessage( const QStringList & ) const {
    return i18nc( "@info", "X.509 certificates refreshed successfully." );
}

