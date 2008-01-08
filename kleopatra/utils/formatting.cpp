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

#include "formatting.h"

#include <kleo/dn.h>

#include <gpgme++/key.h>

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QCoreApplication>
#include <QTextDocument> // for Qt::escape

using namespace GpgME;
using namespace Kleo;

namespace {
struct hack {
    // now, if only this would work at namespace scope...
    Q_DECLARE_TR_FUNCTIONS( Formatting )
};
}

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
	const QString cn = subject["CN"].trimmed();
	if ( cn.isEmpty() )
	    return subject.prettyDN();
	return cn;
    }

    return QString();
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
    const QString email = QString::fromUtf8( email_ ).trimmed();
    if ( !email.isEmpty() )
	if ( email.startsWith( '<' ) && email.endsWith( '>' ) )
	    return email.mid( 1, email.length() - 2 );
	else
	    return email;
    return DN( id )["EMAIL"].trimmed();
}

//
// Tooltip
//

namespace {

    template <typename T_arg>
    QString format_row( const QString & field, const T_arg & arg ) {
	return hack::tr( "<tr><th>%1:</th><td>%2</td></tr>" ).arg( field ).arg( arg );
    }
    QString format_row( const QString & field, const QString & arg ) {
	return hack::tr( "<tr><th>%1:</th><td>%2</td></tr>" ).arg( field, Qt::escape( arg ) );
    }
    QString format_row( const QString & field, const char * arg ) {
	return format_row( field, QString::fromUtf8( arg ) );
    }

    QString format_keytype( const Key & key ) {
	const Subkey subkey = key.subkey( 0 );
	if ( key.hasSecret() )
	    return hack::tr( "%1-bit %2 (secret key available)" ).arg( subkey.length() ).arg( subkey.publicKeyAlgorithmAsString() );
	else
	    return hack::tr( "%1-bit %2" ).arg( subkey.length() ).arg( subkey.publicKeyAlgorithmAsString() );
    }

    QString format_keyusage( const Key & key ) {
	QStringList capabilites;
	if ( key.canSign() )
	    if ( key.isQualified() )
		capabilites.push_back( hack::tr( "Signing EMails and Files (Qualified)" ) );
	    else
		capabilites.push_back( hack::tr( "Signing EMails and Files" ) );
	if ( key.canEncrypt() )
	    capabilites.push_back( hack::tr( "Encrypting EMails and Files" ) );
	if ( key.canCertify() )
	    capabilites.push_back( hack::tr( "Certifying other Certificates" ) );
	if ( key.canAuthenticate() )
	    capabilites.push_back( hack::tr( "Authenticate against Servers" ) );
	return capabilites.join( hack::tr(", ") );
    }

    static QString time_t2string( time_t t ) {
	QDateTime dt;
	dt.setTime_t( t );
	return dt.toString();
    }

    static QString make_red( const QString & txt ) {
	return QLatin1String( "<font color=\"red\">" ) + Qt::escape( txt ) + QLatin1String( "</font>" );
    }

}

QString Formatting::toolTip( const Key & key ) {
    if ( key.protocol() != CMS && key.protocol() != OpenPGP )
        return QString();

    const Subkey subkey = key.subkey( 0 );

    QString result = QLatin1String( "<table border=\"0\">" );
    if ( key.protocol() == CMS ) {
        result += format_row( hack::tr("Serial number"), key.issuerSerial() );
        result += format_row( hack::tr("Issuer"), key.issuerName() );
    }
    result += format_row( key.protocol() == CMS
                          ? hack::tr("Subject")
                          : hack::tr("User-ID"), key.userID( 0 ).id() );
    for ( unsigned int i = 1, end = key.numUserIDs() ; i < end ; ++i )
        result += format_row( hack::tr("a.k.a."), key.userID( i ).id() );
    result += format_row( hack::tr("Validity"),
                          subkey.neverExpires()
                          ? hack::tr( "from %1 until forever" ).arg( time_t2string( subkey.creationTime() ) )
                          : hack::tr( "from %1 through %2" ).arg( time_t2string( subkey.creationTime() ), time_t2string( subkey.expirationTime() ) ) );
    result += format_row( hack::tr("Certificate type"), format_keytype( key ) );
    result += format_row( hack::tr("Certificate usage"), format_keyusage( key ) );
    result += format_row( hack::tr("Fingerprint"), key.primaryFingerprint() );
    result += QLatin1String( "</table><br>" );

    if ( key.protocol() == OpenPGP || ( key.keyListMode() & Validate ) )
        if ( key.isRevoked() )
            result += make_red( hack::tr( "This certificate has been revoked." ) );
        else if ( key.isExpired() )
            result += make_red( hack::tr( "This certificate has expired." ) );
        else if ( key.isDisabled() )
            result += hack::tr( "This certificate has been disabled locally." );

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

    template <typename T>
    QString expiration_date_string( const T & tee ) {
	return tee.neverExpires() ? QString() : time_t2date( tee.expirationTime() ).toString() ;
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
    return creation_date( key.subkey( 0 ) ).toString();
}

QString Formatting::creationDateString( const Subkey & subkey ) {
    return creation_date( subkey ).toString();
}

QString Formatting::creationDateString( const UserID::Signature & sig ) {
    return creation_date( sig ).toString();
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

QString Formatting::type( const Key & key ) {
    if ( key.protocol() == CMS )
        return hack::tr("X.509");
    if ( key.protocol() == OpenPGP )
        return hack::tr("OpenPGP");
    return hack::tr("Unknown");
}

QString Formatting::type( const Subkey & subkey ) {
    return QString::fromUtf8( subkey.publicKeyAlgorithmAsString() );
}

//
// Status / Validity
//

QString Formatting::validityShort( const Subkey & subkey ) {
    if ( subkey.isRevoked() )
	return hack::tr("revoked");
    if ( subkey.isExpired() )
	return hack::tr("expired");
    if ( subkey.isDisabled() )
	return hack::tr("disabled");
    if ( subkey.isInvalid() )
	return hack::tr("invalid");
    return hack::tr("good");
}

QString Formatting::validityShort( const UserID & uid ) {
    if ( uid.isRevoked() )
	return hack::tr("revoked");
    if ( uid.isInvalid() )
	return hack::tr("invalid");
    switch ( uid.validity() ) {
    case UserID::Unknown:   return hack::tr("unknown");
    case UserID::Undefined: return hack::tr("undefined");
    case UserID::Never:     return hack::tr("untrusted");
    case UserID::Marginal:  return hack::tr("marginal");
    case UserID::Full:      return hack::tr("full");
    case UserID::Ultimate:  return hack::tr("ultimate");
    }
    return QString();
}

QString Formatting::validityShort( const UserID::Signature & sig ) {
    switch ( sig.status() ) {
    case UserID::Signature::NoError:
	if ( !sig.isInvalid() )
	    if ( sig.certClass() > 0 )
		return hack::tr("class %d").arg( sig.certClass() );
	    else
		return hack::tr("good");
	// fall through:
    case UserID::Signature::GeneralError:
	return hack::tr("invalid");
    case UserID::Signature::SigExpired:   return hack::tr("expired");
    case UserID::Signature::KeyExpired:   return hack::tr("key expired");
    case UserID::Signature::BadSignature: return hack::tr("bad");
    case UserID::Signature::NoPublicKey:  return QString();
    }
    return QString();
}

QString Formatting::formatForComboBox( const GpgME::Key & key ) {
    const QString name = prettyName( key );
    QString mail = prettyEMail( key );
    if ( !mail.isEmpty() )
        mail = '<' + mail + '>';
    return hack::tr( "%1 %2 (%3)", "name, email, key id" ).arg( name, mail, key.shortKeyID() ).simplified();
}
