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
    const QString email = QString::fromUtf8( uid.email() ).trimmed();
    if ( !email.isEmpty() )
	if ( email.startsWith( '<' ) && email.endsWith( '>' ) )
	    return email.mid( 1, email.length() - 2 );
	else
	    return email;
    return DN( uid.id() )["EMAIL"].trimmed();
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
}

QString Formatting::expirationDateString( const Key & key ) {
    return expirationDateString( key.subkey( 0 ) );
}

QString Formatting::expirationDateString( const Subkey & subkey ) {
    return subkey.neverExpires() ? QString() : expirationDate( subkey ).toString() ;
}

QDate Formatting::expirationDate( const Key & key ) {
    return expirationDate( key.subkey( 0 ) );
}

QDate Formatting::expirationDate( const Subkey & subkey ) {
    return time_t2date( subkey.expirationTime() );
}


QString Formatting::creationDateString( const Key & key ) {
    return expirationDateString( key.subkey( 0 ) );
}

QString Formatting::creationDateString( const Subkey & subkey ) {
    return creationDate( subkey ).toString();
}

QDate Formatting::creationDate( const Key & key ) {
    return creationDate( key.subkey( 0 ) );
}

QDate Formatting::creationDate( const Subkey & subkey ) {
    return time_t2date( subkey.creationTime() );
}

//
// Types
//

QString Formatting::type( const Key & key ) {
    return QString::fromUtf8( key.protocolAsString() );
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

