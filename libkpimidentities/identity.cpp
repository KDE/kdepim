// -*- mode: C++; c-file-style: "gnu" -*-
// kmidentity.cpp
// License: GPL

#include "identity.h"

#include <libkdepim/kfileio.h>
#include <libkdepim/collectingprocess.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kurl.h>

#include <QFileInfo>
#include <QMimeData>
#include <QByteArray>

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

using namespace KPIM;


Signature::Signature()
  : mType( Disabled )
{

}

Signature::Signature( const QString & text )
  : mText( text ),
    mType( Inlined )
{

}

Signature::Signature( const QString & url, bool isExecutable )
  : mUrl( url ),
    mType( isExecutable ? FromCommand : FromFile )
{
}

bool Signature::operator==( const Signature & other ) const {
  if ( mType != other.mType ) return false;
  switch ( mType ) {
  case Inlined: return mText == other.mText;
  case FromFile:
  case FromCommand: return mUrl == other.mUrl;
  default:
  case Disabled: return true;
  }
}

QString Signature::rawText( bool * ok ) const
{
  switch ( mType ) {
  case Disabled:
    if ( ok ) *ok = true;
    return QString();
  case Inlined:
    if ( ok ) *ok = true;
    return mText;
  case FromFile:
    return textFromFile( ok );
  case FromCommand:
    return textFromCommand( ok );
  };
  kFatal( 5006 ) << "Signature::type() returned unknown value!" << endl;
  return QString(); // make compiler happy
}

QString Signature::textFromCommand( bool * ok ) const
{
  assert( mType == FromCommand );

  // handle pathological cases:
  if ( mUrl.isEmpty() ) {
    if ( ok ) *ok = true;
    return QString();
  }

  // create a shell process:
  CollectingProcess proc;
  proc.setUseShell(true);
  proc << mUrl;

  // run the process:
  int rc = 0;
  if ( !proc.start( KProcess::Block, KProcess::Stdout ) )
    rc = -1;
  else
    rc = ( proc.normalExit() ) ? proc.exitStatus() : -1 ;

  // handle errors, if any:
  if ( rc != 0 ) {
    if ( ok ) *ok = false;
    QString wmsg = i18n("<qt>Failed to execute signature script<br><b>%1</b>:<br>%2</qt>",
        mUrl, strerror(rc) );
    KMessageBox::error(0, wmsg);
    return QString();
  }

  // no errors:
  if ( ok ) *ok = true;

  // get output:
  QByteArray output = proc.collectedStdout();

  // ### hmm, should we allow other encodings, too?
  return QString::fromLocal8Bit( output.data(), output.size() );
}

QString Signature::textFromFile( bool * ok ) const
{
  assert( mType == FromFile );

  // ### FIXME: Use KIO::NetAccess to download non-local files!
  if ( !KUrl(mUrl).isLocalFile() && !(QFileInfo(mUrl).isRelative()
                                      && QFileInfo(mUrl).exists()) ) {
    kDebug( 5006 ) << "Signature::textFromFile: non-local URLs are unsupported" << endl;
    if ( ok ) *ok = false;
    return QString();
  }
  if ( ok ) *ok = true;
  // ### hmm, should we allow other encodings, too?
  const QByteArray ba = kFileToByteArray( mUrl, false );
  return QString::fromLocal8Bit( ba.data(), ba.size() );
}

QString Signature::withSeparator( bool * ok ) const
{
  bool internalOK = false;
  QString signature = rawText( &internalOK );
  if ( !internalOK ) {
    if ( ok ) *ok = false;
    return QString();
  }
  if ( ok ) *ok = true;
  if ( signature.isEmpty() ) return signature; // don't add a separator in this case
  if ( signature.startsWith( QString::fromLatin1("-- \n") ) )
    // already have signature separator at start of sig:
    return QString::fromLatin1("\n") += signature;
  else if ( signature.indexOf( QString::fromLatin1("\n-- \n") ) != -1 )
    // already have signature separator inside sig; don't prepend '\n'
    // to improve abusing signatures as templates:
    return signature;
  else
    // need to prepend one:
    return QString::fromLatin1("\n-- \n") + signature;
}


void Signature::setUrl( const QString & url, bool isExecutable )
{
  mUrl = url;
  mType = isExecutable ? FromCommand : FromFile ;
}

// config keys and values:
static const char sigTypeKey[] = "Signature Type";
static const char sigTypeInlineValue[] = "inline";
static const char sigTypeFileValue[] = "file";
static const char sigTypeCommandValue[] = "command";
static const char sigTypeDisabledValue[] = "disabled";
static const char sigTextKey[] = "Inline Signature";
static const char sigFileKey[] = "Signature File";
static const char sigCommandKey[] = "Signature Command";

void Signature::readConfig( const KConfigGroup &config )
{
  QString sigType = config.readEntry( sigTypeKey );
  if ( sigType == sigTypeInlineValue ) {
    mType = Inlined;
  } else if ( sigType == sigTypeFileValue ) {
    mType = FromFile;
    mUrl = config.readPathEntry( sigFileKey );
  } else if ( sigType == sigTypeCommandValue ) {
    mType = FromCommand;
    mUrl = config.readPathEntry( sigCommandKey );
  } else {
    mType = Disabled;
  }
  mText = config.readEntry( sigTextKey );
}

void Signature::writeConfig( KConfigGroup & config ) const
{
  switch ( mType ) {
  case Inlined:
    config.writeEntry( sigTypeKey, sigTypeInlineValue );
    break;
  case FromFile:
    config.writeEntry( sigTypeKey, sigTypeFileValue );
    config.writePathEntry( sigFileKey, mUrl );
    break;
  case FromCommand:
    config.writeEntry( sigTypeKey, sigTypeCommandValue );
    config.writePathEntry( sigCommandKey, mUrl );
    break;
  case Disabled:
    config.writeEntry( sigTypeKey, sigTypeDisabledValue );
  default: ;
  }
  config.writeEntry( sigTextKey, mText );
}

QDataStream & KPIM::operator<<( QDataStream & stream, const KPIM::Signature & sig ) {
  return stream << static_cast<quint8>(sig.mType)
		<< sig.mUrl
		<< sig.mText;
}

QDataStream & KPIM::operator>>( QDataStream & stream, KPIM::Signature & sig ) {
    quint8 s;
    stream >> s
           >> sig.mUrl
           >> sig.mText;
    sig.mType = static_cast<Signature::Type>(s);
    return stream;
}

// ### should use a kstaticdeleter?
static Identity* identityNull = 0;
const Identity& Identity::null()
{
    if ( !identityNull )
        identityNull = new Identity;
    return *identityNull;
}

bool Identity::isNull() const {
  return mIdentity.isEmpty() && mFullName.isEmpty() && mEmailAddr.isEmpty() &&
    mOrganization.isEmpty() && mReplyToAddr.isEmpty() && mBcc.isEmpty() &&
    mVCardFile.isEmpty() &&
    mFcc.isEmpty() && mDrafts.isEmpty() && mTemplates.isEmpty() &&
    mPGPEncryptionKey.isEmpty() && mPGPSigningKey.isEmpty() &&
    mSMIMEEncryptionKey.isEmpty() && mSMIMESigningKey.isEmpty() &&
    mTransport.isEmpty() && mDictionary.isEmpty() &&
#ifdef HAVE_GPGME
    mPreferredCryptoMessageFormat == Kleo::AutoFormat &&
#endif
    mSignature.type() == Signature::Disabled &&
    mXFace.isEmpty();
}

bool Identity::operator==( const Identity & other ) const {
  bool same = mUoid == other.mUoid &&
      mIdentity == other.mIdentity && mFullName == other.mFullName &&
      mEmailAddr == other.mEmailAddr && mOrganization == other.mOrganization &&
      mReplyToAddr == other.mReplyToAddr && mBcc == other.mBcc &&
      mVCardFile == other.mVCardFile &&
      mFcc == other.mFcc &&
      mPGPEncryptionKey == other.mPGPEncryptionKey &&
      mPGPSigningKey == other.mPGPSigningKey &&
      mSMIMEEncryptionKey == other.mSMIMEEncryptionKey &&
      mSMIMESigningKey == other.mSMIMESigningKey &&
#ifdef HAVE_GPGME
      mPreferredCryptoMessageFormat == other.mPreferredCryptoMessageFormat &&
#endif
      mDrafts == other.mDrafts && mTemplates == other.mTemplates &&
      mTransport == other.mTransport &&
      mDictionary == other.mDictionary && mSignature == other.mSignature &&
      mXFace == other.mXFace && mXFaceEnabled == other.mXFaceEnabled;

#if 0
  if ( same )
    return true;
  if ( mUoid != other.mUoid ) kDebug() << "mUoid differs : " << mUoid << " != " << other.mUoid << endl;
  if ( mIdentity != other.mIdentity ) kDebug() << "mIdentity differs : " << mIdentity << " != " << other.mIdentity << endl;
  if ( mFullName != other.mFullName ) kDebug() << "mFullName differs : " << mFullName << " != " << other.mFullName << endl;
  if ( mEmailAddr != other.mEmailAddr ) kDebug() << "mEmailAddr differs : " << mEmailAddr << " != " << other.mEmailAddr << endl;
  if ( mOrganization != other.mOrganization ) kDebug() << "mOrganization differs : " << mOrganization << " != " << other.mOrganization << endl;
  if ( mReplyToAddr != other.mReplyToAddr ) kDebug() << "mReplyToAddr differs : " << mReplyToAddr << " != " << other.mReplyToAddr << endl;
  if ( mBcc != other.mBcc ) kDebug() << "mBcc differs : " << mBcc << " != " << other.mBcc << endl;
  if ( mVCardFile != other.mVCardFile ) kDebug() << "mVCardFile differs : " << mVCardFile << " != " << other.mVCardFile << endl;
  if ( mFcc != other.mFcc ) kDebug() << "mFcc differs : " << mFcc << " != " << other.mFcc << endl;
  if ( mPGPEncryptionKey != other.mPGPEncryptionKey ) kDebug() << "mPGPEncryptionKey differs : " << mPGPEncryptionKey << " != " << other.mPGPEncryptionKey << endl;
  if ( mPGPSigningKey != other.mPGPSigningKey ) kDebug() << "mPGPSigningKey differs : " << mPGPSigningKey << " != " << other.mPGPSigningKey << endl;
  if ( mSMIMEEncryptionKey != other.mSMIMEEncryptionKey ) kDebug() << "mSMIMEEncryptionKey differs : '" << mSMIMEEncryptionKey << "' != '" << other.mSMIMEEncryptionKey << "'" << endl;
  if ( mSMIMESigningKey != other.mSMIMESigningKey ) kDebug() << "mSMIMESigningKey differs : " << mSMIMESigningKey << " != " << other.mSMIMESigningKey << endl;
  if ( mPreferredCryptoMessageFormat != other.mPreferredCryptoMessageFormat ) kDebug() << "mPreferredCryptoMessageFormat differs : " << mPreferredCryptoMessageFormat << " != " << other.mPreferredCryptoMessageFormat << endl;
  if ( mDrafts != other.mDrafts ) kDebug() << "mDrafts differs : " << mDrafts << " != " << other.mDrafts << endl;
  if ( mTemplates != other.mTemplates ) kDebug() << "mTemplates differs : " << mTemplates << " != " << other.mTemplates << endl;
  if ( mTransport != other.mTransport ) kDebug() << "mTransport differs : " << mTransport << " != " << other.mTransport << endl;
  if ( mDictionary != other.mDictionary ) kDebug() << "mDictionary differs : " << mDictionary << " != " << other.mDictionary << endl;
  if ( ! ( mSignature == other.mSignature ) ) kDebug() << "mSignature differs" << endl;
#endif
  return same;
}

Identity::Identity( const QString & id, const QString & fullName,
			const QString & emailAddr, const QString & organization,
			const QString & replyToAddr )
  : mUoid( 0 ), mIdentity( id ), mFullName( fullName ),
    mEmailAddr( emailAddr ), mOrganization( organization ),
    mReplyToAddr( replyToAddr ),
    // Using "" instead of null to make operator==() not fail
    // (readConfig returns "")
    mBcc( "" ), mVCardFile( "" ), mPGPEncryptionKey( "" ), mPGPSigningKey( "" ),
    mSMIMEEncryptionKey( "" ), mSMIMESigningKey( "" ), mFcc( "" ),
    mDrafts( "" ), mTemplates( "" ), mTransport( "" ),
    mDictionary( "" ),
    mXFace( "" ), mXFaceEnabled( false ),
    mIsDefault( false )
#ifdef HAVE_GPGME
    , mPreferredCryptoMessageFormat( Kleo::AutoFormat )
#endif
{
}

Identity::~Identity()
{
}


void Identity::readConfig( const KConfigGroup & config )
{
  mUoid = config.readEntry("uoid",QVariant(0)).toUInt();

  mIdentity = config.readEntry("Identity");
  mFullName = config.readEntry("Name");
  mEmailAddr = config.readEntry("Email Address");
  mVCardFile = config.readPathEntry("VCardFile");
  mOrganization = config.readEntry("Organization");
  mPGPSigningKey = config.readEntry("PGP Signing Key").toLatin1();
  mPGPEncryptionKey = config.readEntry("PGP Encryption Key").toLatin1();
  mSMIMESigningKey = config.readEntry("SMIME Signing Key").toLatin1();
  mSMIMEEncryptionKey = config.readEntry("SMIME Encryption Key").toLatin1();
#ifdef HAVE_GPGME
  mPreferredCryptoMessageFormat = Kleo::stringToCryptoMessageFormat( config.readEntry("Preferred Crypto Message Format", "none" ) );
#endif
  mReplyToAddr = config.readEntry("Reply-To Address");
  mBcc = config.readEntry("Bcc");
  mFcc = config.readEntry("Fcc", "sent-mail");
  if( mFcc.isEmpty() )
    mFcc = "sent-mail";
  mDrafts = config.readEntry("Drafts", "drafts");
  if( mDrafts.isEmpty() )
    mDrafts = "drafts";
  mTemplates = config.readEntry("Templates", "templates");
  if( mTemplates.isEmpty() )
    mTemplates = "templates";
  mTransport = config.readEntry("Transport");
  mDictionary = config.readEntry( "Dictionary" );
  mXFace = config.readEntry( "X-Face" );
  mXFaceEnabled = config.readEntry( "X-FaceEnabled", QVariant(false) ).toBool();

  mSignature.readConfig( config );
  kDebug(5006) << "Identity::readConfig(): UOID = " << mUoid
	    << " for identity named \"" << mIdentity << "\"" << endl;
}


void Identity::writeConfig( KConfigGroup & config ) const
{
  config.writeEntry("uoid", mUoid);

  config.writeEntry("Identity", mIdentity);
  config.writeEntry("Name", mFullName);
  config.writeEntry("Organization", mOrganization);
  config.writeEntry("PGP Signing Key", mPGPSigningKey.data());
  config.writeEntry("PGP Encryption Key", mPGPEncryptionKey.data());
  config.writeEntry("SMIME Signing Key", mSMIMESigningKey.data());
  config.writeEntry("SMIME Encryption Key", mSMIMEEncryptionKey.data());
#ifdef HAVE_GPGME
  config.writeEntry("Preferred Crypto Message Format", Kleo::cryptoMessageFormatToString( mPreferredCryptoMessageFormat ) );
#else
  config.writeEntry("Preferred Crypto Message Format", QString() );
#endif
  config.writeEntry("Email Address", mEmailAddr);
  config.writeEntry("Reply-To Address", mReplyToAddr);
  config.writeEntry("Bcc", mBcc);
  config.writePathEntry("VCardFile", mVCardFile);
  config.writeEntry("Transport", mTransport);
  config.writeEntry("Fcc", mFcc);
  config.writeEntry("Drafts", mDrafts);
  config.writeEntry("Templates", mTemplates);
  config.writeEntry( "Dictionary", mDictionary );
  config.writeEntry( "X-Face", mXFace );
  config.writeEntry( "X-FaceEnabled", mXFaceEnabled );

  mSignature.writeConfig( config );
}

QDataStream & KPIM::operator<<( QDataStream & stream, const KPIM::Identity & i ) {
  return stream << static_cast<quint32>(i.uoid())
		<< i.identityName()
		<< i.fullName()
		<< i.organization()
		<< i.pgpSigningKey()
		<< i.pgpEncryptionKey()
		<< i.smimeSigningKey()
		<< i.smimeEncryptionKey()
		<< i.emailAddr()
		<< i.replyToAddr()
		<< i.bcc()
		<< i.vCardFile()
		<< i.transport()
		<< i.fcc()
		<< i.drafts()
		<< i.templates()
		<< i.mSignature
                << i.dictionary()
                << i.xface()
#ifdef HAVE_GPGME
                << QString( Kleo::cryptoMessageFormatToString( i.mPreferredCryptoMessageFormat ) );
#else
                << QString();
#endif
}

QDataStream & KPIM::operator>>( QDataStream & stream, KPIM::Identity & i ) {
  quint32 uoid;
  QString format;
  stream        >> uoid
		>> i.mIdentity
		>> i.mFullName
		>> i.mOrganization
		>> i.mPGPSigningKey
		>> i.mPGPEncryptionKey
		>> i.mSMIMESigningKey
		>> i.mSMIMEEncryptionKey
		>> i.mEmailAddr
		>> i.mReplyToAddr
		>> i.mBcc
		>> i.mVCardFile
		>> i.mTransport
		>> i.mFcc
		>> i.mDrafts
		>> i.mTemplates
		>> i.mSignature
                >> i.mDictionary
                >> i.mXFace
		>> format;
  i.mUoid = uoid;
#ifdef HAVE_GPGME
  i.mPreferredCryptoMessageFormat = Kleo::stringToCryptoMessageFormat( format.toLatin1() );
#endif

  return stream;
}

//-----------------------------------------------------------------------------
bool Identity::mailingAllowed() const
{
  return !mEmailAddr.isEmpty();
}


void Identity::setIsDefault( bool flag ) {
  mIsDefault = flag;
}

void Identity::setIdentityName( const QString & name ) {
  mIdentity = name;
}

void Identity::setFullName(const QString &str)
{
  mFullName = str;
}


//-----------------------------------------------------------------------------
void Identity::setOrganization(const QString &str)
{
  mOrganization = str;
}

void Identity::setPGPSigningKey(const QByteArray &str)
{
  mPGPSigningKey = str;
  if ( mPGPSigningKey.isNull() )
    mPGPSigningKey = "";
}

void Identity::setPGPEncryptionKey(const QByteArray &str)
{
  mPGPEncryptionKey = str;
  if ( mPGPEncryptionKey.isNull() )
    mPGPEncryptionKey = "";
}

void Identity::setSMIMESigningKey(const QByteArray &str)
{
  mSMIMESigningKey = str;
  if ( mSMIMESigningKey.isNull() )
    mSMIMESigningKey = "";
}

void Identity::setSMIMEEncryptionKey(const QByteArray &str)
{
  mSMIMEEncryptionKey = str;
  if ( mSMIMEEncryptionKey.isNull() )
    mSMIMEEncryptionKey = "";
}

//-----------------------------------------------------------------------------
void Identity::setEmailAddr(const QString &str)
{
  mEmailAddr = str;
}


//-----------------------------------------------------------------------------
void Identity::setVCardFile(const QString &str)
{
  mVCardFile = str;
}


//-----------------------------------------------------------------------------
QString Identity::fullEmailAddr(void) const
{
  if (mFullName.isEmpty()) return mEmailAddr;

  const QString specials("()<>@,.;:[]");

  QString result;

  // add DQUOTE's if necessary:
  bool needsQuotes=false;
  for (int i=0; i < mFullName.length(); i++) {
    if ( specials.contains( mFullName[i] ) )
      needsQuotes = true;
    else if ( mFullName[i] == '\\' || mFullName[i] == '"' ) {
      needsQuotes = true;
      result += '\\';
    }
    result += mFullName[i];
  }

  if (needsQuotes) {
    result.insert(0,'"');
    result += '"';
  }

  result += " <" + mEmailAddr + '>';

  return result;
}

//-----------------------------------------------------------------------------
void Identity::setReplyToAddr(const QString& str)
{
  mReplyToAddr = str;
}


//-----------------------------------------------------------------------------
void Identity::setSignatureFile(const QString &str)
{
  mSignature.setUrl( str, signatureIsCommand() );
}


//-----------------------------------------------------------------------------
void Identity::setSignatureInlineText(const QString &str )
{
  mSignature.setText( str );
}


//-----------------------------------------------------------------------------
void Identity::setTransport(const QString &str)
{
  mTransport = str;
  if ( mTransport.isNull() )
    mTransport = "";
}

//-----------------------------------------------------------------------------
void Identity::setFcc(const QString &str)
{
  mFcc = str;
  if ( mFcc.isNull() )
    mFcc = "";
}

//-----------------------------------------------------------------------------
void Identity::setDrafts(const QString &str)
{
  mDrafts = str;
  if ( mDrafts.isNull() )
    mDrafts = "";
}


//-----------------------------------------------------------------------------
void Identity::setTemplates(const QString &str)
{
  mTemplates = str;
  if ( mTemplates.isNull() )
    mTemplates = "";
}


//-----------------------------------------------------------------------------
void Identity::setDictionary( const QString &str )
{
  mDictionary = str;
  if ( mDictionary.isNull() )
    mDictionary = "";
}


//-----------------------------------------------------------------------------
void Identity::setXFace( const QString &str )
{
  mXFace = str;
  mXFace.remove( " " );
  mXFace.remove( "\n" );
  mXFace.remove( "\r" );
}


//-----------------------------------------------------------------------------
void Identity::setXFaceEnabled( const bool on )
{
  mXFaceEnabled = on;
}


//-----------------------------------------------------------------------------
QString Identity::signatureText( bool * ok ) const
{
  bool internalOK = false;
  QString signatureText = mSignature.withSeparator( &internalOK );
  if ( internalOK ) {
    if ( ok ) *ok=true;
    return signatureText;
  }

  // OK, here comes the funny part. The call to
  // Signature::withSeparator() failed, so we should probably fix the
  // cause:
  if ( ok ) *ok = false;
  return QString();

#if 0 // ### FIXME: error handling
  if (mSignatureFile.endsWith("|"))
  {
  }
  else
  {
  }
#endif

  return QString();
}


QString Identity::mimeDataType()
{
  return "application/x-kmail-identity-drag";
}

bool Identity::canDecode( const QMimeData*md )
{
  return md->hasFormat( mimeDataType() );
}

void Identity::populateMimeData( QMimeData*md )
{
  QByteArray a;
  {
    QDataStream s( &a, QIODevice::WriteOnly );
    s << this;
  }
  md->setData( mimeDataType(), a );
}

Identity Identity::fromMimeData( const QMimeData*md )
{
  Identity i;
  if ( canDecode( md ) ) {
    QByteArray ba = md->data( mimeDataType() );
    QDataStream s( &ba, QIODevice::ReadOnly );
    s >> i;
  }
  return i;
}
