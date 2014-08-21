/* -*- mode: c++; c-basic-offset:4 -*-
    utils/formatting.cpp

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

#include <config-kleopatra.h>

#include "formatting.h"

#include <utils/kleo_assert.h>

#include <kleo/dn.h>

#include <kmime/kmime_header_parsing.h>

#include <gpgme++/key.h>
#include <gpgme++/importresult.h>

#include <KLocalizedString>
#include <KLocale>

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QTextDocument> // for Qt::escape
#include <QLocale>

using namespace GpgME;
using namespace Kleo;
using namespace KMime::Types;
using namespace KMime::HeaderParsing;

//
// Name
//

QString Formatting::prettyName( int proto, const char * id, const char * name_, const char * comment_ ) {

    if ( proto == OpenPGP ) {
        const QString name = QString::fromUtf8( name_ );
        if ( name.isEmpty() )
            return QString();
        const QString comment = QString::fromUtf8( comment_ );
        if ( comment.isEmpty() )
            return name;
        return QString::fromLatin1( "%1 (%2)" ).arg( name, comment );
    }

    if ( proto == CMS ) {
        const DN subject( id );
        const QString cn = subject[QLatin1String("CN")].trimmed();
        if ( cn.isEmpty() )
            return subject.prettyDN();
        return cn;
    }

    return QString();
}

QString Formatting::prettyNameAndEMail( int proto, const char * id, const char * name_, const char * email_, const char * comment_ ) {
    return prettyNameAndEMail( proto, QString::fromUtf8( id ), QString::fromUtf8( name_ ), prettyEMail( email_, id ), QString::fromUtf8( comment_ ) );
}

QString Formatting::prettyNameAndEMail( int proto, const QString & id, const QString & name, const QString & email, const QString & comment ) {

    if ( proto == OpenPGP ) {
        if ( name.isEmpty() ) {
            if ( email.isEmpty() )
                return QString();
            else if ( comment.isEmpty() )
                return QString::fromLatin1( "<%1>" ).arg( email );
            else
                return QString::fromLatin1( "(%2) <%1>" ).arg( email, comment );
        }
        if ( email.isEmpty() ) {
            if ( comment.isEmpty() )
                return name;
            else
                return QString::fromLatin1( "%1 (%2)" ).arg( name, comment );
        }
        if ( comment.isEmpty() )
            return QString::fromLatin1( "%1 <%2>" ).arg( name, email );
        else
            return QString::fromLatin1( "%1 (%3) <%2>" ).arg( name, email, comment );
    }

    if ( proto == CMS ) {
        const DN subject( id );
        const QString cn = subject[QLatin1String("CN")].trimmed();
        if ( cn.isEmpty() )
            return subject.prettyDN();
        return cn;
    }
    return QString();
}

QString Formatting::prettyUserID( const UserID & uid ) {
    if ( uid.parent().protocol() == OpenPGP )
        return prettyNameAndEMail( uid );
    const QByteArray id = QByteArray( uid.id() ).trimmed();
    if ( id.startsWith( '<' ) )
        return prettyEMail( uid.email(), uid.id() );
    if ( id.startsWith( '(' ) )
        // ### parse uri/dns:
        return QString::fromUtf8( uid.id() );
    else
        return DN( uid.id() ).prettyDN();
}

QString Formatting::prettyKeyID( const char * id ) {
    if ( !id )
        return QString();
    return QLatin1String("0x") + QString::fromLatin1( id ).toUpper();
}

QString Formatting::prettyNameAndEMail( const UserID & uid ) {
    return prettyNameAndEMail( uid.parent().protocol(), uid.id(), uid.name(), uid.email(), uid.comment() );
}

QString Formatting::prettyNameAndEMail( const Key & key ) {
    return prettyNameAndEMail( key.userID( 0 ) );
}

QString Formatting::prettyName( const Key & key ) {
    return prettyName( key.userID( 0 ) );
}

QString Formatting::prettyName( const UserID & uid ) {
    return prettyName( uid.parent().protocol(), uid.id(), uid.name(), uid.comment() );
}

QString Formatting::prettyName( const UserID::Signature & sig ) {
    return prettyName( OpenPGP, sig.signerUserID(), sig.signerName(), sig.signerComment() );
}

//
// EMail
//

QString Formatting::prettyEMail( const Key & key ) {
    for ( unsigned int i = 0, end = key.numUserIDs() ; i < end ; ++i ) {
        const QString email = prettyEMail( key.userID( i ) );
        if ( !email.isEmpty() )
            return email;
    }
    return QString();
}

QString Formatting::prettyEMail( const UserID & uid ) {
    return prettyEMail( uid.email(), uid.id() );
}

QString Formatting::prettyEMail( const UserID::Signature & sig ) {
    return prettyEMail( sig.signerEmail(), sig.signerUserID() );
}

QString Formatting::prettyEMail( const char * email_, const char * id ) {
    Mailbox mailBox;
    if ( email_ && parseMailbox( email_, email_ + strlen( email_ ), mailBox ) )
        return mailBox.addrSpec().asPrettyString();
    else
        return DN( id )[QLatin1String("EMAIL")].trimmed();
}

//
// Tooltip
//

namespace {

    static QString protect_whitespace( QString s ) {
        static const QLatin1Char SP( ' ' ), NBSP( '\xA0' );
        return s.replace( SP, NBSP );
    }

    template <typename T_arg>
    QString format_row( const QString & field, const T_arg & arg ) {
        return i18n( "<tr><th>%1:</th><td>%2</td></tr>", protect_whitespace( field ), arg );
    }
    QString format_row( const QString & field, const QString & arg ) {
        return i18n( "<tr><th>%1:</th><td>%2</td></tr>", protect_whitespace( field ), arg.toHtmlEscaped()  );
    }
    QString format_row( const QString & field, const char * arg ) {
        return format_row( field, QString::fromUtf8( arg ) );
    }

    QString format_keytype( const Key & key ) {
        const Subkey subkey = key.subkey( 0 );
        if ( key.hasSecret() )
            return i18n( "%1-bit %2 (secret key available)", subkey.length(), QLatin1String(subkey.publicKeyAlgorithmAsString()) );
        else
            return i18n( "%1-bit %2", subkey.length(), QLatin1String(subkey.publicKeyAlgorithmAsString()) );
    }

    QString format_keyusage( const Key & key ) {
        QStringList capabilities;
        if ( key.canReallySign() ) {
            if ( key.isQualified() )
                capabilities.push_back( i18n( "Signing EMails and Files (Qualified)" ) );
            else
                capabilities.push_back( i18n( "Signing EMails and Files" ) );
    }
        if ( key.canEncrypt() )
            capabilities.push_back( i18n( "Encrypting EMails and Files" ) );
        if ( key.canCertify() )
            capabilities.push_back( i18n( "Certifying other Certificates" ) );
        if ( key.canAuthenticate() )
            capabilities.push_back( i18n( "Authenticate against Servers" ) );
        return capabilities.join( QLatin1String(", ") );
    }

    static QString time_t2string( time_t t ) {
        QDateTime dt;
        dt.setTime_t( t );
        return QLocale().toString( dt, QLocale::ShortFormat );
    }

    static QString make_red( const QString & txt ) {
        return QLatin1String( "<font color=\"red\">" ) + txt.toHtmlEscaped() + QLatin1String( "</font>" );
    }

}

QString Formatting::toolTip( const Key & key, int flags ) {
    if ( flags == 0 || ( key.protocol() != CMS && key.protocol() != OpenPGP ) )
        return QString();

    const Subkey subkey = key.subkey( 0 );

    QString result;
    if ( flags & Validity )
        if ( key.protocol() == OpenPGP || ( key.keyListMode() & Validate ) )
            if ( key.isRevoked() )
                result += make_red( i18n( "This certificate has been revoked." ) );
            else if ( key.isExpired() )
                result += make_red( i18n( "This certificate has expired." ) );
            else if ( key.isDisabled() )
                result += i18n( "This certificate has been disabled locally." );
            else
                result += i18n( "This certificate is currently valid." );
        else
            result += i18n( "The validity of this certificate cannot be checked at the moment." );
    if ( flags == Validity )
        return result;

    result += QLatin1String( "<table border=\"0\">" );
    if ( key.protocol() == CMS ) {
        if ( flags & SerialNumber )
            result += format_row( i18n("Serial number"), key.issuerSerial() );
        if ( flags & Issuer )
            result += format_row( i18n("Issuer"), key.issuerName() );
    }
    if ( flags & UserIDs ) {
        const std::vector<UserID> uids = key.userIDs();
        if ( !uids.empty() )
            result += format_row( key.protocol() == CMS
                                  ? i18n("Subject")
                                  : i18n("User-ID"), prettyUserID( uids.front() ) );
        if ( uids.size() > 1 )
            for ( std::vector<UserID>::const_iterator it = uids.begin() + 1, end = uids.end() ; it != end ; ++it )
                if ( !it->isRevoked() && !it->isInvalid() )
                    result += format_row( i18n("a.k.a."), prettyUserID( *it ) );
    }
    if ( flags & ExpiryDates )
        result += format_row( i18n("Validity"),
                              subkey.neverExpires()
                              ? i18n( "from %1 until forever", time_t2string( subkey.creationTime() ) )
                              : i18n( "from %1 through %2", time_t2string( subkey.creationTime() ), time_t2string( subkey.expirationTime() ) ) );
    if ( flags & CertificateType )
        result += format_row( i18n("Certificate type"), format_keytype( key ) );
    if ( flags & CertificateUsage )
        result += format_row( i18n("Certificate usage"), format_keyusage( key ) );
    if ( flags & KeyID )
        result += format_row( i18n("Key-ID"), QString::fromLatin1( key.shortKeyID() ) ) ;
    if ( flags & Fingerprint )
        result += format_row( i18n("Fingerprint"), key.primaryFingerprint() );
    if ( flags & OwnerTrust )
        if ( key.protocol() == OpenPGP )
            result += format_row( i18n("Ownertrust"), ownerTrustShort( key ) );
        else if ( key.isRoot() )
            result += format_row( i18n("Trusted issuer?"),
                                  key.userID(0).validity() == UserID::Ultimate ? i18n("Yes") :
                                  /* else */                                     i18n("No") );
    if ( flags & StorageLocation )
        if ( const char * card = subkey.cardSerialNumber() )
            result += format_row( i18n("Stored"), i18nc("stored...","on SmartCard with serial no. %1", QString::fromUtf8( card ) ) );
        else
            result += format_row( i18n("Stored"), i18nc("stored...","on this computer") );
    result += QLatin1String( "</table>" );

    return result;
}

//
// Creation and Expiration
//

namespace {
    static QDate time_t2date( time_t t ) {
        if ( !t )
            return QDate();
        QDateTime dt;
        dt.setTime_t( t );
        return dt.date();
    }
    static QString date2string( const QDate & date ) {
        return QLocale().toString( date, QLocale::ShortFormat );
    }

    template <typename T>
    QString expiration_date_string( const T & tee ) {
        return tee.neverExpires() ? QString() : date2string( time_t2date( tee.expirationTime() ) ) ;
    }
    template <typename T>
    QDate creation_date( const T & tee ) {
        return time_t2date( tee.creationTime() );
    }
    template <typename T>
    QDate expiration_date( const T & tee ) {
        return time_t2date( tee.expirationTime() );
    }
}

QString Formatting::expirationDateString( const Key & key ) {
    return expiration_date_string( key.subkey( 0 ) );
}

QString Formatting::expirationDateString( const Subkey & subkey ) {
    return expiration_date_string( subkey );
}

QString Formatting::expirationDateString( const UserID::Signature & sig ) {
    return expiration_date_string( sig );
}

QDate Formatting::expirationDate( const Key & key ) {
    return expiration_date( key.subkey( 0 ) );
}

QDate Formatting::expirationDate( const Subkey & subkey ) {
    return expiration_date( subkey );
}

QDate Formatting::expirationDate( const UserID::Signature & sig ) {
    return expiration_date( sig );
}


QString Formatting::creationDateString( const Key & key ) {
    return date2string( creation_date( key.subkey( 0 ) ) );
}

QString Formatting::creationDateString( const Subkey & subkey ) {
    return date2string( creation_date( subkey ) );
}

QString Formatting::creationDateString( const UserID::Signature & sig ) {
    return date2string( creation_date( sig ) );
}

QDate Formatting::creationDate( const Key & key ) {
    return creation_date( key.subkey( 0 ) );
}

QDate Formatting::creationDate( const Subkey & subkey ) {
    return creation_date( subkey );
}

QDate Formatting::creationDate( const UserID::Signature & sig ) {
    return creation_date( sig );
}

//
// Types
//

QString Formatting::displayName( Protocol p ) {
    if ( p == CMS )
        return i18nc("X.509/CMS encryption standard", "X.509");
    if ( p == OpenPGP )
        return i18n("OpenPGP");
    return i18nc("Unknown encryption protocol", "Unknown");
}

QString Formatting::type( const Key & key ) {
    return displayName( key.protocol() );
}

QString Formatting::type( const Subkey & subkey ) {
    return QString::fromUtf8( subkey.publicKeyAlgorithmAsString() );
}

//
// Status / Validity
//

QString Formatting::ownerTrustShort( const Key & key ) {
    return ownerTrustShort( key.ownerTrust() );
}

QString Formatting::ownerTrustShort( Key::OwnerTrust trust ) {
    switch ( trust ) {
        case Key::Unknown:   return i18nc("unknown trust level", "unknown");
        case Key::Never:     return i18n("untrusted");
        case Key::Marginal:  return i18nc("marginal trust", "marginal");
        case Key::Full:      return i18nc("full trust", "full");
        case Key::Ultimate:  return i18nc("ultimate trust", "ultimate");
        case Key::Undefined: return i18nc("undefined trust", "undefined");
        default:
            assert( !"unexpected owner trust value" );
            break;
    }
    return QString();
}

QString Formatting::validityShort( const Subkey & subkey ) {
    if ( subkey.isRevoked() )
        return i18n("revoked");
    if ( subkey.isExpired() )
        return i18n("expired");
    if ( subkey.isDisabled() )
        return i18n("disabled");
    if ( subkey.isInvalid() )
        return i18n("invalid");
    return i18nc("as in good/valid signature", "good");
}

QString Formatting::validityShort( const UserID & uid ) {
    if ( uid.isRevoked() )
        return i18n("revoked");
    if ( uid.isInvalid() )
        return i18n("invalid");
    switch ( uid.validity() ) {
    case UserID::Unknown:   return i18nc("unknown trust level", "unknown");
    case UserID::Undefined: return i18nc("undefined trust", "undefined");
    case UserID::Never:     return i18n("untrusted");
    case UserID::Marginal:  return i18nc("marginal trust", "marginal");
    case UserID::Full:      return i18nc("full trust", "full");
    case UserID::Ultimate:  return i18nc("ultimate trust", "ultimate");
    }
    return QString();
}

QString Formatting::validityShort( const UserID::Signature & sig ) {
    switch ( sig.status() ) {
    case UserID::Signature::NoError:
        if ( !sig.isInvalid() ) {
            if ( sig.certClass() > 0 )
                return i18n("class %1", sig.certClass() );
            else
                return i18nc("good/valid signature", "good");
    }
        // fall through:
    case UserID::Signature::GeneralError:
        return i18n("invalid");
    case UserID::Signature::SigExpired:   return i18n("expired");
    case UserID::Signature::KeyExpired:   return i18n("certificate expired");
    case UserID::Signature::BadSignature: return i18nc("fake/invalid signature", "bad");
    case UserID::Signature::NoPublicKey:  return QString();
    }
    return QString();
}

QString Formatting::formatKeyLink( const Key & key ) {
    if ( key.isNull() )
        return QString();
    return QString::fromLatin1( "<a href=\"key:%1\">%2</a>" ).arg( QLatin1String(key.primaryFingerprint()), Formatting::prettyName( key ) );
}

QString Formatting::formatForComboBox( const GpgME::Key & key ) {
    const QString name = prettyName( key );
    QString mail = prettyEMail( key );
    if ( !mail.isEmpty() )
        mail = QLatin1Char('<') + mail + QLatin1Char('>');
    return i18nc( "name, email, key id", "%1 %2 (%3)", name, mail, QLatin1String(key.shortKeyID()) ).simplified();
}

namespace {

static QString keyToString( const Key & key ) {

    kleo_assert( !key.isNull() );

    const QString email = Formatting::prettyEMail( key );
    const QString name = Formatting::prettyName( key );

    if ( name.isEmpty() )
        return email;
    else if ( email.isEmpty() )
        return name;
    else
        return QString::fromLatin1( "%1 <%2>" ).arg( name, email );
}

}

const char * Formatting::summaryToString( const Signature::Summary summary )
{
    if ( summary & Signature::Red )
        return "RED";
    if ( summary & Signature::Green )
        return "GREEN";
    return "YELLOW";
}

QString Formatting::signatureToString( const Signature & sig, const Key & key )
{
    if ( sig.isNull() )
        return QString();

    const bool red   = (sig.summary() & Signature::Red);
    const bool valid = (sig.summary() & Signature::Valid);

    if ( red )
        if ( key.isNull() )
            if ( const char * fpr = sig.fingerprint() )
                return i18n("Bad signature by unknown certificate %1: %2", QString::fromLatin1( fpr ), QString::fromLocal8Bit( sig.status().asString() ) );
            else
                return i18n("Bad signature by an unknown certificate: %1", QString::fromLocal8Bit( sig.status().asString() ) );
        else
            return i18n("Bad signature by %1: %2", keyToString( key ), QString::fromLocal8Bit( sig.status().asString() ) );

    else if ( valid )
        if ( key.isNull() )
            if ( const char * fpr = sig.fingerprint() )
                return i18n("Good signature by unknown certificate %1.", QString::fromLatin1( fpr ) );
            else
                return i18n("Good signature by an unknown certificate.");
        else
            return i18n("Good signature by %1.", keyToString( key ) );

    else
        if ( key.isNull() )
            if ( const char * fpr = sig.fingerprint() )
                return i18n("Invalid signature by unknown certificate %1: %2", QString::fromLatin1( fpr ), QString::fromLocal8Bit( sig.status().asString() ) );
            else
                return i18n("Invalid signature by an unknown certificate: %1", QString::fromLocal8Bit( sig.status().asString() ) );
        else
            return i18n("Invalid signature by %1: %2", keyToString( key ), QString::fromLocal8Bit( sig.status().asString() ) );
}

//
// ImportResult
//

QString Formatting::importMetaData( const Import & import, const QStringList & ids ) {
    const QString result = importMetaData( import );
    if ( result.isEmpty() )
        return QString();
    else
        return result + QLatin1Char('\n') +
            i18n("This certificate was imported from the following sources:") + QLatin1Char('\n') +
            ids.join(QLatin1String("\n"));
}

QString Formatting::importMetaData( const Import & import ) {

    if ( import.isNull() )
        return QString();

    if ( import.error().isCanceled() )
        return i18n( "The import of this certificate was canceled." );
    if ( import.error() )
        return i18n( "An error occurred importing this certificate: %1",
                     QString::fromLocal8Bit( import.error().asString() ) );

    const unsigned int status = import.status();
    if ( status & Import::NewKey )
        return ( status & Import::ContainedSecretKey )
            ? i18n( "This certificate was new to your keystore. The secret key is available." )
            : i18n( "This certificate is new to your keystore." ) ;

    QStringList results;
    if ( status & Import::NewUserIDs )
        results.push_back( i18n( "New user-ids were added to this certificate by the import." ) );
    if ( status & Import::NewSignatures )
        results.push_back( i18n( "New signatures were added to this certificate by the import." ) );
    if ( status & Import::NewSubkeys )
        results.push_back( i18n( "New subkeys were added to this certificate by the import." ) );

    return results.empty()
        ? i18n( "The import contained no new data for this certificate. It is unchanged.")
        : results.join( QLatin1String("\n") );
}


//
// Overview in CertificateDetailsDialog
//

QString Formatting::formatOverview( const Key & key ) {
    return toolTip( key, AllOptions );
}
