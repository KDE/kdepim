/* -*- mode: c++; c-basic-offset:4 -*-
    commands/exportsecretkeycommand.cpp

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

#include "exportsecretkeycommand.h"

#include "command_p.h"

#include <dialogs/exportsecretkeydialog.h>

#include <utils/gnupg-helper.h>

#include <gpgme++/key.h>

#include <KLocalizedString>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace Kleo::Dialogs;
using namespace GpgME;

ExportSecretKeyCommand::ExportSecretKeyCommand( KeyListController * c )
    : GnuPGProcessCommand( c )
{
}

ExportSecretKeyCommand::ExportSecretKeyCommand( QAbstractItemView * v, KeyListController * c )
    : GnuPGProcessCommand( v, c )
{
}

ExportSecretKeyCommand::ExportSecretKeyCommand( const Key & key )
    : GnuPGProcessCommand( key )
{
}

ExportSecretKeyCommand::~ExportSecretKeyCommand() {}

void ExportSecretKeyCommand::setFileName( const QString & fileName ) {
    m_filename = fileName;
}

void ExportSecretKeyCommand::setPassphraseCharset( const QByteArray & charset ) {
    m_charset = charset;
}

void ExportSecretKeyCommand::setUseArmor( bool armor ) {
    m_armor = armor;
}

bool ExportSecretKeyCommand::preStartHook( QWidget * parent ) const {
    if ( !m_filename.isEmpty() )
        return true;

    ExportSecretKeyDialog dlg( parent );
    dlg.setKey( d->key() );
    if ( !dlg.exec() )
        return false;
    
    m_filename = dlg.fileName();
    m_armor = dlg.useArmor();
    m_charset = dlg.charset();

    return true;
}

QStringList ExportSecretKeyCommand::arguments() const {
    const Key key = d->key();
    QStringList result;

    if ( key.protocol() == OpenPGP )
        result << gpgPath() << QLatin1String("--batch");
    else
        result << gpgSmPath();

    result << QLatin1String("--output") << m_filename;

    if ( m_armor )
        result << QLatin1String("--armor");

    if ( key.protocol() == CMS && !m_charset.isEmpty() )
        result << QLatin1String("--p12-charset") << QLatin1String(m_charset);

    if ( key.protocol() == OpenPGP )
        result << QLatin1String("--export-secret-key");
    else
        result << QLatin1String("--export-secret-key-p12");

    result << QLatin1String(key.primaryFingerprint());

    return result;
}

QString ExportSecretKeyCommand::errorCaption() const {
    return i18nc( "@title:window", "Secret Key Export Error" );
}

QString ExportSecretKeyCommand::successCaption() const {
    return i18nc( "@title:window", "Secret Key Export Finished" );
}

QString ExportSecretKeyCommand::crashExitMessage( const QStringList & args ) const {
    return i18nc("@info",
                 "<para>The GPG or GpgSM process that tried to export the secret key "
                 "ended prematurely because of an unexpected error.</para>"
                 "<para>Please check the output of <icode>%1</icode> for details.</para>", args.join( QLatin1String(" ") ) ) ;
}

QString ExportSecretKeyCommand::errorExitMessage( const QStringList & args ) const {
    return i18nc("@info",
                 "<para>An error occurred while trying to export the secret key.</para> "
                 "<para>The output from <command>%1</command> was: <message>%2</message></para>",
                 args[0], errorString() );
}

QString ExportSecretKeyCommand::successMessage( const QStringList & ) const {
    return i18nc( "@info", "Secret key successfully exported." );
}

