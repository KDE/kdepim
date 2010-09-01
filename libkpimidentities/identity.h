/* -*- mode: C++; c-file-style: "gnu" -*-
 * User identity information
 *
 * Author: Stefan Taferner <taferner@kde.org>
 * This code is under GPL
 */
#ifndef kpim_identity_h
#define kpim_identity_h

#include <kleo/enum.h>

#include <kdepimmacros.h>

#include <tqstring.h>
#include <tqcstring.h>
#include <tqstringlist.h>

class KProcess;
namespace KPIM {
  class Identity;
  class Signature;
}
class KConfigBase;
class IdentityList;
class TQDataStream;

namespace KPIM {

/**
 * @short abstraction of a signature (aka "footer").
 * @author Marc Mutz <mutz@kde.org>
 */
class KDE_EXPORT Signature {
  friend class Identity;

  friend TQDataStream & KPIM::operator<<( TQDataStream & stream, const Signature & sig );
  friend TQDataStream & KPIM::operator>>( TQDataStream & stream, Signature & sig );

public:
  /** Type of signature (ie. way to obtain the signature text) */
  enum Type { Disabled = 0, Inlined = 1, FromFile = 2, FromCommand = 3 };

  /** Used for comparison */
  bool operator==( const Signature & other ) const;

  /** Constructor for disabled signature */
  Signature();
  /** Constructor for inline text */
  Signature( const TQString & text );
  /** Constructor for text from a file or from output of a command */
  Signature( const TQString & url, bool isExecutable );

  /** @return the raw signature text as entered resp. read from file. */
  TQString rawText( bool * ok=0 ) const;

  /** @return the signature text with a "-- " separator added, if
      necessary. */
  TQString withSeparator( bool * ok=0 ) const;

  /** Set the signature text and mark this signature as being of
      "inline text" type. */
  void setText( const TQString & text ) { mText = text; }
  TQString text() const { return mText; }

  /** Set the signature URL and mark this signature as being of
      "from file" resp. "from output of command" type. */
  void setUrl( const TQString & url, bool isExecutable=false );
  TQString url() const { return mUrl; }

  /// @return the type of signature (ie. way to obtain the signature text)
  Type type() const { return mType; }
  void setType( Type type ) { mType = type; }

protected:
  void writeConfig( KConfigBase * config ) const;
  void readConfig( const KConfigBase * config );

private:
  TQString textFromFile( bool * ok ) const;
  TQString textFromCommand( bool * ok ) const;

private:
  TQString mUrl;
  TQString mText;
  Type    mType;
};

/** User identity information */
class KDE_EXPORT Identity
{
  // only the identity manager should be able to construct and
  // destruct us, but then we get into problems with using
  // TQValueList<Identity> and especially qHeapSort().
  friend class IdentityManager;

  friend TQDataStream & operator<<( TQDataStream & stream, const KPIM::Identity & ident );
  friend TQDataStream & operator>>( TQDataStream & stream, KPIM::Identity & ident );

public:
  typedef TQValueList<Identity> List;

  /** used for comparison */
  bool operator==( const Identity & other ) const;

  bool operator!=( const Identity & other ) const {
    return !operator==( other );
  }

  /** used for sorting */
  bool operator<( const Identity & other ) const {
    if ( isDefault() ) return true;
    if ( other.isDefault() ) return false;
    return identityName() < other.identityName();
  }
  bool operator>( const Identity & other ) const {
    if ( isDefault() ) return false;
    if ( other.isDefault() ) return true;
    return identityName() > other.identityName();
  }
  bool operator<=( const Identity & other ) const {
    return !operator>( other );
  }
  bool operator>=( const Identity & other ) const {
    return !operator<( other );
  }

  /** Constructor */
  explicit Identity( const TQString & id=TQString::null,
		     const TQString & realName=TQString::null,
		     const TQString & emailAddr=TQString::null,
		     const TQString & organization=TQString::null,
		     const TQString & replyToAddress=TQString::null );

  /** Destructor */
  ~Identity();

protected:
  /** Read configuration from config. Group must be preset (or use
      KConfigGroup). Called from IdentityManager. */
  void readConfig( const KConfigBase * );

  /** Write configuration to config. Group must be preset (or use
      KConfigGroup). Called from IdentityManager. */
  void writeConfig( KConfigBase * ) const;

public:
  /** Tests if there are enough values set to allow mailing */
  bool mailingAllowed() const;

  /** Identity/nickname for this collection */
  TQString identityName() const { return mIdentity; }
  void setIdentityName( const TQString & name );

  /** @return whether this identity is the default identity */
  bool isDefault() const { return mIsDefault; }

  /// Unique Object Identifier for this identity
  uint uoid() const { return mUoid; }

protected:
  /** Set whether this identity is the default identity. Since this
      affects all other identites, too (most notably, the old default
      identity), only the IdentityManager can change this.
      You should use
      <pre>
      kmkernel->identityManager()->setAsDefault( name_of_default )
      </pre>
      instead.
  **/
  void setIsDefault( bool flag );

  void setUoid( uint aUoid ) { mUoid = aUoid; }

public:
  /** Full name of the user */
  TQString fullName() const { return mFullName; }
  void setFullName(const TQString&);

  /** The user's organization (optional) */
  TQString organization() const { return mOrganization; }
  void setOrganization(const TQString&);

  KDE_DEPRECATED TQCString pgpIdentity() const { return pgpEncryptionKey(); }
  KDE_DEPRECATED void setPgpIdentity( const TQCString & key ) {
    setPGPEncryptionKey( key );
    setPGPSigningKey( key );
  }

  /** The user's OpenPGP encryption key */
  TQCString pgpEncryptionKey() const { return mPGPEncryptionKey; }
  void setPGPEncryptionKey( const TQCString & key );

  /** The user's OpenPGP signing key */
  TQCString pgpSigningKey() const { return mPGPSigningKey; }
  void setPGPSigningKey( const TQCString & key );

  /** The user's S/MIME encryption key */
  TQCString smimeEncryptionKey() const { return mSMIMEEncryptionKey; }
  void setSMIMEEncryptionKey( const TQCString & key );

  /** The user's S/MIME signing key */
  TQCString smimeSigningKey() const { return mSMIMESigningKey; }
  void setSMIMESigningKey( const TQCString & key );

  Kleo::CryptoMessageFormat preferredCryptoMessageFormat() const { return mPreferredCryptoMessageFormat; }
  void setPreferredCryptoMessageFormat( Kleo::CryptoMessageFormat format ) { mPreferredCryptoMessageFormat = format; }

  /** email address (without the user name - only name\@host) */
  KDE_DEPRECATED TQString emailAddr() const { return primaryEmailAddress(); }
  KDE_DEPRECATED void setEmailAddr( const TQString & email ) { setPrimaryEmailAddress( email ); }

  /** primary email address (without the user name - only name\@host).
      The primary email address is used for all outgoing mail. */
  TQString primaryEmailAddress() const { return mEmailAddr; }
  void setPrimaryEmailAddress( const TQString & email );

  /** email address aliases */
  const TQStringList & emailAliases() const { return mEmailAliases; }
  void setEmailAliases( const TQStringList & );

  bool matchesEmailAddress( const TQString & addr ) const;

  /** vCard to attach to outgoing emails */
  TQString vCardFile() const { return mVCardFile; }
  void setVCardFile(const TQString&);

  /** email address in the format "username <name@host>" suitable
    for the "From:" field of email messages. */
  TQString fullEmailAddr() const;

  /** email address for the ReplyTo: field */
  TQString replyToAddr() const { return mReplyToAddr; }
  void setReplyToAddr(const TQString&);

  /** email addresses for the BCC: field */
  TQString bcc() const { return mBcc; }
  void setBcc(const TQString& aBcc) { mBcc = aBcc; }

  void setSignature( const Signature & sig ) { mSignature = sig; }
  Signature & signature() /* _not_ const! */ { return mSignature; }
  const Signature & signature() const { return mSignature; }

protected:
  /** @return true if the signature is read from the output of a command */
  bool signatureIsCommand() const { return mSignature.type() == Signature::FromCommand; }
  /** @return true if the signature is read from a text file */
  bool signatureIsPlainFile() const { return mSignature.type() == Signature::FromFile; }
  /** @return true if the signature was specified directly */
  bool signatureIsInline() const { return mSignature.type() == Signature::Inlined; }

  /** name of the signature file (with path) */
  TQString signatureFile() const { return mSignature.url(); }
  void setSignatureFile(const TQString&);

  /** inline signature */
  TQString signatureInlineText() const { return mSignature.text();}
  void setSignatureInlineText(const TQString&);

  /** Inline or signature from a file */
  bool useSignatureFile() const { return signatureIsPlainFile() || signatureIsCommand(); }

public:
  /** Returns the signature. This method also takes care of special
    signature files that are shell scripts and handles them
    correct. So use this method to rectreive the contents of the
    signature file. If @p prompt is false, no errors will be displayed
    (useful for retries). */
  TQString signatureText( bool * ok=0) const;

  /** The transport that is set for this identity. Used to link a
      transport with an identity. */
  TQString transport() const { return mTransport; }
  void setTransport(const TQString&);

  /** The folder where sent messages from this identity will be
      stored by default. */
  TQString fcc() const { return mFcc; }
  void setFcc(const TQString&);

  /** The folder where draft messages from this identity will be
      stored by default. */
  TQString drafts() const { return mDrafts; }
  void setDrafts(const TQString&);

  /** The folder where template messages from this identity will be
      stored by default. */
  TQString templates() const { return mTemplates; }
  void setTemplates( const TQString& );

  /** dictionary which should be used for spell checking */
  TQString dictionary() const { return mDictionary; }
  void setDictionary( const TQString& );

  /** a X-Face header for this identity */
  TQString xface() const { return mXFace; }
  void setXFace( const TQString& );
  bool isXFaceEnabled() const { return mXFaceEnabled; }
  void setXFaceEnabled( const bool );

  static const Identity& null();
  bool isNull() const;
protected:
  // if you add new members, make sure they have an operator= (or the
  // compiler can synthesize one) and amend Identity::operator==,
  // isNull(), readConfig() and writeConfig() as well as operator<<
  // and operator>> accordingly:
  uint mUoid;
  TQString mIdentity, mFullName, mEmailAddr, mOrganization;
  TQStringList mEmailAliases;
  TQString mReplyToAddr;
  TQString mBcc;
  TQString mVCardFile;
  TQCString mPGPEncryptionKey, mPGPSigningKey, mSMIMEEncryptionKey, mSMIMESigningKey;
  TQString mFcc, mDrafts, mTemplates, mTransport;
  TQString mDictionary;
  TQString mXFace;
  bool mXFaceEnabled;
  Signature mSignature;
  bool      mIsDefault;
  Kleo::CryptoMessageFormat mPreferredCryptoMessageFormat;
};

KDE_EXPORT TQDataStream & operator<<( TQDataStream & stream, const KPIM::Signature & sig );
KDE_EXPORT TQDataStream & operator>>( TQDataStream & stream, KPIM::Signature & sig );

KDE_EXPORT TQDataStream & operator<<( TQDataStream & stream, const KPIM::Identity & ident );
KDE_EXPORT TQDataStream & operator>>( TQDataStream & stream, KPIM::Identity & ident );

} // namespace KPIM

#endif /*kpim_identity_h*/
