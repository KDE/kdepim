// -*- mode: C++; c-file-style: "gnu" -*-
// kmidentity.cpp
// License: GPL

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "identity.h"

#include <libkdepim/kfileio.h>
#include <libkdepim/collectingprocess.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kurl.h>

#include <tqfileinfo.h>

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

Signature::Signature( const TQString & text )
  : mText( text ),
    mType( Inlined )
{

}

Signature::Signature( const TQString & url, bool isExecutable )
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

TQString Signature::rawText( bool * ok ) const
{
  switch ( mType ) {
  case Disabled:
    if ( ok ) *ok = true;
    return TQString::null;
  case Inlined:
    if ( ok ) *ok = true;
    return mText;
  case FromFile:
    return textFromFile( ok );
  case FromCommand:
    return textFromCommand( ok );
  };
  kdFatal( 5006 ) << "Signature::type() returned unknown value!" << endl;
  return TQString::null; // make compiler happy
}

TQString Signature::textFromCommand( bool * ok ) const
{
  assert( mType == FromCommand );

  // handle pathological cases:
  if ( mUrl.isEmpty() ) {
    if ( ok ) *ok = true;
    return TQString::null;
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
    TQString wmsg = i18n("<qt>Failed to execute signature script<br><b>%1</b>:<br>%2</qt>")
      .arg( mUrl ).arg( strerror(rc) );
    KMessageBox::error(0, wmsg);
    return TQString::null;
  }

  // no errors:
  if ( ok ) *ok = true;

  // get output:
  TQByteArray output = proc.collectedStdout();

  // ### hmm, should we allow other encodings, too?
  return TQString::fromLocal8Bit( output.data(), output.size() );
}

TQString Signature::textFromFile( bool * ok ) const
{
  assert( mType == FromFile );

  // ### FIXME: Use KIO::NetAccess to download non-local files!
  if ( !KURL(mUrl).isLocalFile() && !(TQFileInfo(mUrl).isRelative()
                                      && TQFileInfo(mUrl).exists()) ) {
    kdDebug( 5006 ) << "Signature::textFromFile: non-local URLs are unsupported" << endl;
    if ( ok ) *ok = false;
    return TQString::null;
  }
  if ( ok ) *ok = true;
  // ### hmm, should we allow other encodings, too?
  return TQString::fromLocal8Bit( kFileToString( mUrl, false ) );
}

TQString Signature::withSeparator( bool * ok ) const
{
  bool internalOK = false;
  TQString signature = rawText( &internalOK );
  if ( !internalOK ) {
    if ( ok ) *ok = false;
    return TQString::null;
  }
  if ( ok ) *ok = true;
  if ( signature.isEmpty() ) return signature; // don't add a separator in this case
  if ( signature.startsWith( TQString::fromLatin1("-- \n") ) )
    // already have signature separator at start of sig:
    return TQString::fromLatin1("\n") += signature;
  else if ( signature.find( TQString::fromLatin1("\n-- \n") ) != -1 )
    // already have signature separator inside sig; don't prepend '\n'
    // to improve abusing signatures as templates:
    return signature;
  else
    // need to prepend one:
    return TQString::fromLatin1("\n-- \n") + signature;
}


void Signature::setUrl( const TQString & url, bool isExecutable )
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

void Signature::readConfig( const KConfigBase * config )
{
  TQString sigType = config->readEntry( sigTypeKey );
  if ( sigType == sigTypeInlineValue ) {
    mType = Inlined;
  } else if ( sigType == sigTypeFileValue ) {
    mType = FromFile;
    mUrl = config->readPathEntry( sigFileKey );
  } else if ( sigType == sigTypeCommandValue ) {
    mType = FromCommand;
    mUrl = config->readPathEntry( sigCommandKey );
  } else {
    mType = Disabled;
  }
  mText = config->readEntry( sigTextKey );
}

void Signature::writeConfig( KConfigBase * config ) const
{
  switch ( mType ) {
  case Inlined:
    config->writeEntry( sigTypeKey, sigTypeInlineValue );
    break;
  case FromFile:
    config->writeEntry( sigTypeKey, sigTypeFileValue );
    config->writePathEntry( sigFileKey, mUrl );
    break;
  case FromCommand:
    config->writeEntry( sigTypeKey, sigTypeCommandValue );
    config->writePathEntry( sigCommandKey, mUrl );
    break;
  case Disabled:
    config->writeEntry( sigTypeKey, sigTypeDisabledValue );
  default: ;
  }
  config->writeEntry( sigTextKey, mText );
}

TQDataStream & KPIM::operator<<( TQDataStream & stream, const KPIM::Signature & sig ) {
  return stream << static_cast<Q_UINT8>(sig.mType)
		<< sig.mUrl
		<< sig.mText;
}

TQDataStream & KPIM::operator>>( TQDataStream & stream, KPIM::Signature & sig ) {
    Q_UINT8 s;
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
    mEmailAliases.empty() &&
    mOrganization.isEmpty() && mReplyToAddr.isEmpty() && mBcc.isEmpty() &&
    mVCardFile.isEmpty() &&
    mFcc.isEmpty() && mDrafts.isEmpty() && mTemplates.isEmpty() &&
    mPGPEncryptionKey.isEmpty() && mPGPSigningKey.isEmpty() &&
    mSMIMEEncryptionKey.isEmpty() && mSMIMESigningKey.isEmpty() &&
    mTransport.isEmpty() && mDictionary.isEmpty() &&
    mPreferredCryptoMessageFormat == Kleo::AutoFormat &&
    mSignature.type() == Signature::Disabled &&
    mXFace.isEmpty();
}

bool Identity::operator==( const Identity & other ) const {
  bool same = mUoid == other.mUoid &&
      mIdentity == other.mIdentity && mFullName == other.mFullName &&
      mEmailAddr == other.mEmailAddr && mOrganization == other.mOrganization &&
      mEmailAliases == other.mEmailAliases &&
      mReplyToAddr == other.mReplyToAddr && mBcc == other.mBcc &&
      mVCardFile == other.mVCardFile &&
      mFcc == other.mFcc &&
      mPGPEncryptionKey == other.mPGPEncryptionKey &&
      mPGPSigningKey == other.mPGPSigningKey &&
      mSMIMEEncryptionKey == other.mSMIMEEncryptionKey &&
      mSMIMESigningKey == other.mSMIMESigningKey &&
      mPreferredCryptoMessageFormat == other.mPreferredCryptoMessageFormat &&
      mDrafts == other.mDrafts && mTemplates == other.mTemplates &&
      mTransport == other.mTransport &&
      mDictionary == other.mDictionary && mSignature == other.mSignature &&
      mXFace == other.mXFace && mXFaceEnabled == other.mXFaceEnabled;

#if 0
  if ( same )
    return true;
  if ( mUoid != other.mUoid ) kdDebug() << "mUoid differs : " << mUoid << " != " << other.mUoid << endl;
  if ( mIdentity != other.mIdentity ) kdDebug() << "mIdentity differs : " << mIdentity << " != " << other.mIdentity << endl;
  if ( mFullName != other.mFullName ) kdDebug() << "mFullName differs : " << mFullName << " != " << other.mFullName << endl;
  if ( mEmailAddr != other.mEmailAddr ) kdDebug() << "mEmailAddr differs : " << mEmailAddr << " != " << other.mEmailAddr << endl;
  if ( mEmailAliases != other.mEmailAliases ) kdDebug() << "mEmailAliases differs : " << mEmailAliases.join(";") << " != " << other.mEmailAliases.join(";") << endl;
  if ( mOrganization != other.mOrganization ) kdDebug() << "mOrganization differs : " << mOrganization << " != " << other.mOrganization << endl;
  if ( mReplyToAddr != other.mReplyToAddr ) kdDebug() << "mReplyToAddr differs : " << mReplyToAddr << " != " << other.mReplyToAddr << endl;
  if ( mBcc != other.mBcc ) kdDebug() << "mBcc differs : " << mBcc << " != " << other.mBcc << endl;
  if ( mVCardFile != other.mVCardFile ) kdDebug() << "mVCardFile differs : " << mVCardFile << " != " << other.mVCardFile << endl;
  if ( mFcc != other.mFcc ) kdDebug() << "mFcc differs : " << mFcc << " != " << other.mFcc << endl;
  if ( mPGPEncryptionKey != other.mPGPEncryptionKey ) kdDebug() << "mPGPEncryptionKey differs : " << mPGPEncryptionKey << " != " << other.mPGPEncryptionKey << endl;
  if ( mPGPSigningKey != other.mPGPSigningKey ) kdDebug() << "mPGPSigningKey differs : " << mPGPSigningKey << " != " << other.mPGPSigningKey << endl;
  if ( mSMIMEEncryptionKey != other.mSMIMEEncryptionKey ) kdDebug() << "mSMIMEEncryptionKey differs : '" << mSMIMEEncryptionKey << "' != '" << other.mSMIMEEncryptionKey << "'" << endl;
  if ( mSMIMESigningKey != other.mSMIMESigningKey ) kdDebug() << "mSMIMESigningKey differs : " << mSMIMESigningKey << " != " << other.mSMIMESigningKey << endl;
  if ( mPreferredCryptoMessageFormat != other.mPreferredCryptoMessageFormat ) kdDebug() << "mPreferredCryptoMessageFormat differs : " << mPreferredCryptoMessageFormat << " != " << other.mPreferredCryptoMessageFormat << endl;
  if ( mDrafts != other.mDrafts ) kdDebug() << "mDrafts differs : " << mDrafts << " != " << other.mDrafts << endl;
  if ( mTemplates != other.mTemplates ) kdDebug() << "mTemplates differs : " << mTemplates << " != " << other.mTemplates << endl;
  if ( mTransport != other.mTransport ) kdDebug() << "mTransport differs : " << mTransport << " != " << other.mTransport << endl;
  if ( mDictionary != other.mDictionary ) kdDebug() << "mDictionary differs : " << mDictionary << " != " << other.mDictionary << endl;
  if ( ! ( mSignature == other.mSignature ) ) kdDebug() << "mSignature differs" << endl;
#endif
  return same;
}

Identity::Identity( const TQString & id, const TQString & fullName,
			const TQString & emailAddr, const TQString & organization,
			const TQString & replyToAddr )
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
    mIsDefault( false ),
    mPreferredCryptoMessageFormat( Kleo::AutoFormat )
{
}

Identity::~Identity()
{
}


void Identity::readConfig( const KConfigBase * config )
{
  mUoid = config->readUnsignedNumEntry("uoid",0);

  mIdentity = config->readEntry("Identity");
  mFullName = config->readEntry("Name");
  mEmailAddr = config->readEntry("Email Address");
  mEmailAliases = config->readListEntry("Email Aliases");
  mVCardFile = config->readPathEntry("VCardFile");
  mOrganization = config->readEntry("Organization");
  mPGPSigningKey = config->readEntry("PGP Signing Key").latin1();
  mPGPEncryptionKey = config->readEntry("PGP Encryption Key").latin1();
  mSMIMESigningKey = config->readEntry("SMIME Signing Key").latin1();
  mSMIMEEncryptionKey = config->readEntry("SMIME Encryption Key").latin1();
  mPreferredCryptoMessageFormat = Kleo::stringToCryptoMessageFormat( config->readEntry("Preferred Crypto Message Format", "none" ) );
  mReplyToAddr = config->readEntry( "Reply-To Address" );
  mBcc = config->readEntry( "Bcc" );
  mFcc = config->readEntry( "Fcc", "sent-mail" );
  if( mFcc.isEmpty() )
    mFcc = "sent-mail";
  mDrafts = config->readEntry( "Drafts", "drafts" );
  if( mDrafts.isEmpty() )
    mDrafts = "drafts";
  mTemplates = config->readEntry( "Templates", "templates" );
  if( mTemplates.isEmpty() )
    mTemplates = "templates";
  mTransport = config->readEntry( "Transport" );
  mDictionary = config->readEntry( "Dictionary" );
  mXFace = config->readEntry( "X-Face" );
  mXFaceEnabled = config->readBoolEntry( "X-FaceEnabled", false );

  mSignature.readConfig( config );
  kdDebug(5006) << "Identity::readConfig(): UOID = " << mUoid
	    << " for identity named \"" << mIdentity << "\"" << endl;
}


void Identity::writeConfig( KConfigBase * config ) const
{
  config->writeEntry("uoid", mUoid);

  config->writeEntry("Identity", mIdentity);
  config->writeEntry("Name", mFullName);
  config->writeEntry("Organization", mOrganization);
  config->writeEntry("PGP Signing Key", mPGPSigningKey.data());
  config->writeEntry("PGP Encryption Key", mPGPEncryptionKey.data());
  config->writeEntry("SMIME Signing Key", mSMIMESigningKey.data());
  config->writeEntry("SMIME Encryption Key", mSMIMEEncryptionKey.data());
  config->writeEntry("Preferred Crypto Message Format", Kleo::cryptoMessageFormatToString( mPreferredCryptoMessageFormat ) );
  config->writeEntry("Email Address", mEmailAddr);
  config->writeEntry("Email Aliases", mEmailAliases);
  config->writeEntry("Reply-To Address", mReplyToAddr);
  config->writeEntry("Bcc", mBcc);
  config->writePathEntry("VCardFile", mVCardFile);
  config->writeEntry("Transport", mTransport);
  config->writeEntry("Fcc", mFcc);
  config->writeEntry("Drafts", mDrafts);
  config->writeEntry("Templates", mTemplates);
  config->writeEntry( "Dictionary", mDictionary );
  config->writeEntry( "X-Face", mXFace );
  config->writeEntry( "X-FaceEnabled", mXFaceEnabled );

  mSignature.writeConfig( config );
}

TQDataStream & KPIM::operator<<( TQDataStream & stream, const KPIM::Identity & i ) {
  return stream << static_cast<Q_UINT32>(i.uoid())
		<< i.identityName()
		<< i.fullName()
		<< i.organization()
		<< i.pgpSigningKey()
		<< i.pgpEncryptionKey()
		<< i.smimeSigningKey()
		<< i.smimeEncryptionKey()
		<< i.primaryEmailAddress()
		<< i.emailAliases()
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
		<< TQString( Kleo::cryptoMessageFormatToString( i.mPreferredCryptoMessageFormat ) );
}

TQDataStream & KPIM::operator>>( TQDataStream & stream, KPIM::Identity & i ) {
  Q_UINT32 uoid;
  TQString format;
  stream        >> uoid
		>> i.mIdentity
		>> i.mFullName
		>> i.mOrganization
		>> i.mPGPSigningKey
		>> i.mPGPEncryptionKey
		>> i.mSMIMESigningKey
		>> i.mSMIMEEncryptionKey
		>> i.mEmailAddr
		>> i.mEmailAliases
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
  i.mPreferredCryptoMessageFormat = Kleo::stringToCryptoMessageFormat( format.latin1() );

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

void Identity::setIdentityName( const TQString & name ) {
  mIdentity = name;
}

void Identity::setFullName(const TQString &str)
{
  mFullName = str;
}


//-----------------------------------------------------------------------------
void Identity::setOrganization(const TQString &str)
{
  mOrganization = str;
}

void Identity::setPGPSigningKey(const TQCString &str)
{
  mPGPSigningKey = str;
  if ( mPGPSigningKey.isNull() )
    mPGPSigningKey = "";
}

void Identity::setPGPEncryptionKey(const TQCString &str)
{
  mPGPEncryptionKey = str;
  if ( mPGPEncryptionKey.isNull() )
    mPGPEncryptionKey = "";
}

void Identity::setSMIMESigningKey(const TQCString &str)
{
  mSMIMESigningKey = str;
  if ( mSMIMESigningKey.isNull() )
    mSMIMESigningKey = "";
}

void Identity::setSMIMEEncryptionKey(const TQCString &str)
{
  mSMIMEEncryptionKey = str;
  if ( mSMIMEEncryptionKey.isNull() )
    mSMIMEEncryptionKey = "";
}

//-----------------------------------------------------------------------------
void Identity::setPrimaryEmailAddress( const TQString & str )
{
  mEmailAddr = str;
}

void Identity::setEmailAliases( const TQStringList & list )
{
  mEmailAliases = list;
}

bool Identity::matchesEmailAddress( const TQString & addr ) const
{
  const TQString lower = addr.lower();
  if ( lower == mEmailAddr.lower() )
    return true;
  for ( TQStringList::const_iterator it = mEmailAliases.begin(), end = mEmailAliases.end() ; it != end ; ++it )
    if ( (*it).lower() == lower )
      return true;
  return false;
}

//-----------------------------------------------------------------------------
void Identity::setVCardFile(const TQString &str)
{
  mVCardFile = str;
}


//-----------------------------------------------------------------------------
TQString Identity::fullEmailAddr(void) const
{
  if (mFullName.isEmpty()) return mEmailAddr;

  const TQString specials("()<>@,.;:[]");

  TQString result;

  // add DQUOTE's if necessary:
  bool needsQuotes=false;
  for (unsigned int i=0; i < mFullName.length(); i++) {
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
void Identity::setReplyToAddr(const TQString& str)
{
  mReplyToAddr = str;
}


//-----------------------------------------------------------------------------
void Identity::setSignatureFile(const TQString &str)
{
  mSignature.setUrl( str, signatureIsCommand() );
}


//-----------------------------------------------------------------------------
void Identity::setSignatureInlineText(const TQString &str )
{
  mSignature.setText( str );
}


//-----------------------------------------------------------------------------
void Identity::setTransport( const TQString &str )
{
  mTransport = str;
  if ( mTransport.isNull() )
    mTransport = "";
}

//-----------------------------------------------------------------------------
void Identity::setFcc( const TQString &str )
{
  mFcc = str;
  if ( mFcc.isNull() )
    mFcc = "";
}

//-----------------------------------------------------------------------------
void Identity::setDrafts( const TQString &str )
{
  mDrafts = str;
  if ( mDrafts.isNull() )
    mDrafts = "";
}

//-----------------------------------------------------------------------------
void Identity::setTemplates( const TQString &str )
{
  mTemplates = str;
  if ( mTemplates.isNull() )
    mTemplates = "";
}

//-----------------------------------------------------------------------------
void Identity::setDictionary( const TQString &str )
{
  mDictionary = str;
  if ( mDictionary.isNull() )
    mDictionary = "";
}


//-----------------------------------------------------------------------------
void Identity::setXFace( const TQString &str )
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
TQString Identity::signatureText( bool * ok ) const
{
  bool internalOK = false;
  TQString signatureText = mSignature.withSeparator( &internalOK );
  if ( internalOK ) {
    if ( ok ) *ok=true;
    return signatureText;
  }

  // OK, here comes the funny part. The call to
  // Signature::withSeparator() failed, so we should probably fix the
  // cause:
  if ( ok ) *ok = false;
  return TQString::null;

#if 0 // ### FIXME: error handling
  if (mSignatureFile.endsWith("|"))
  {
  }
  else
  {
  }
#endif

  return TQString::null;
}
