/* -*- mode: c++; c-basic-offset:4 -*-
    commands/exportopenpgpcertstoservercommand.cpp

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

#include "exportopenpgpcertstoservercommand.h"

#include "command_p.h"

#include <utils/gnupg-helper.h>

#include <kleo/cryptobackendfactory.h>
#include <kleo/cryptoconfig.h>

#include <gpgme++/key.h>

#include <KLocalizedString>
#include <KMessageBox>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace GpgME;

static bool haveKeyserverConfigured() {
    const Kleo::CryptoConfig * const config = Kleo::CryptoBackendFactory::instance()->config();
    if ( !config )
        return false;
    const Kleo::CryptoConfigEntry * const entry = config->entry( QLatin1String("gpg"), QLatin1String("Keyserver"), QLatin1String("keyserver") );
    return entry && !entry->stringValue().isEmpty();
}

ExportOpenPGPCertsToServerCommand::ExportOpenPGPCertsToServerCommand( KeyListController * c )
    : GnuPGProcessCommand( c )
{

}

ExportOpenPGPCertsToServerCommand::ExportOpenPGPCertsToServerCommand( QAbstractItemView * v, KeyListController * c )
    : GnuPGProcessCommand( v, c )
{

}


ExportOpenPGPCertsToServerCommand::ExportOpenPGPCertsToServerCommand( const Key & key )
    : GnuPGProcessCommand( key )
{

}

ExportOpenPGPCertsToServerCommand::~ExportOpenPGPCertsToServerCommand() {}

bool ExportOpenPGPCertsToServerCommand::preStartHook( QWidget * parent ) const {
    if ( !haveKeyserverConfigured() )
        if ( KMessageBox::warningContinueCancel( parent,
                                                 xi18nc("@info",
                                                       "<para>No OpenPGP directory services have been configured.</para>"
                                                       "<para>Since none is configured, <application>Kleopatra</application> will use "
                                                       "<resource>keys.gnupg.net</resource> as the server to export to.</para>"
                                                       "<para>You can configure OpenPGP directory servers in <application>Kleopatra</application>'s "
                                                       "configuration dialog.</para>"
                                                       "<para>Do you want to continue with <resource>keys.gnupg.net</resource> "
                                                       "as the server to export to?</para>" ),
                                                 i18nc("@title:window", "OpenPGP Certificate Export"),
                                                 KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                                                 QLatin1String( "warn-export-openpgp-missing-keyserver" ) )
             != KMessageBox::Continue )
            return false;
    return KMessageBox::warningContinueCancel( parent,
                                               xi18nc("@info",
                                                     "<para>When OpenPGP certificates have been exported to a public directory server, "
                                                     "it is nearly impossible to remove them again.</para>"
                                                     "<para>Before exporting your certificate to a public directory server, make sure that you "
                                                     "have created a revocation certificate so you can revoke the certificate if needed later.</para>"
                                                     "<para>Are you sure you want to continue?</para>"),
                                               i18nc("@title:window", "OpenPGP Certificate Export"),
                                               KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                                               QLatin1String( "warn-export-openpgp-nonrevocable" ) )
        == KMessageBox::Continue;
}

QStringList ExportOpenPGPCertsToServerCommand::arguments() const {
    QStringList result;
    result << gpgPath();
    if ( !haveKeyserverConfigured() )
        result << QLatin1String("--keyserver") << QLatin1String("keys.gnupg.net");
    result << QLatin1String("--send-keys");
    Q_FOREACH( const Key & key, d->keys() ) //krazy:exclude=foreach
        result << QLatin1String(key.primaryFingerprint());
    return result;
}

QString ExportOpenPGPCertsToServerCommand::errorCaption() const {
    return i18nc( "@title:window", "OpenPGP Certificate Export Error" );
}

QString ExportOpenPGPCertsToServerCommand::successCaption() const {
    return i18nc( "@title:window", "OpenPGP Certificate Export Finished" );
}

QString ExportOpenPGPCertsToServerCommand::crashExitMessage( const QStringList & args ) const {
    return xi18nc("@info",
                 "<para>The GPG process that tried to export OpenPGP certificates "
                 "ended prematurely because of an unexpected error.</para>"
                 "<para>Please check the output of <icode>%1</icode> for details.</para>", args.join( QLatin1String(" ") ) ) ;
}

QString ExportOpenPGPCertsToServerCommand::errorExitMessage( const QStringList & args ) const {
    return xi18nc("@info",
                 "<para>An error occurred while trying to export OpenPGP certificates.</para> "
                 "<para>The output from <command>%1</command> was: <message>%2</message></para>",
                 args[0], errorString() );
}

QString ExportOpenPGPCertsToServerCommand::successMessage( const QStringList & ) const {
    return i18nc( "@info", "OpenPGP certificates exported successfully." );
}

