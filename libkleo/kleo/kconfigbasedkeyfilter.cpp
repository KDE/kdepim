/*
    kconfigbasedkeyfilter.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
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


#include "kconfigbasedkeyfilter.h"

#include <kconfigbase.h>
#include <kconfiggroup.h>
#include <klocale.h>

#include <boost/mem_fn.hpp>
#include <algorithm>

using namespace Kleo;
using namespace GpgME;
using namespace boost;

//
//
// FontDescription - intuitive font property resolving
//                   (QFont::resolve doesn't work for us)
//
//
struct KeyFilter::FontDescription::Private {
    bool bold, italic, strikeOut, fullFont;
    QFont font;
};

KeyFilter::FontDescription::FontDescription()
    : d( new Private )
{
    d->bold = d->italic = d->strikeOut = d->fullFont = false;
}

KeyFilter::FontDescription::FontDescription( const FontDescription & other )
    : d( new Private( *other.d ) )
{

}

KeyFilter::FontDescription::~FontDescription() {
    delete d;
}

KeyFilter::FontDescription KeyFilter::FontDescription::create( bool b, bool i, bool s ) {
    FontDescription fd;
    fd.d->bold = b;
    fd.d->italic = i;
    fd.d->strikeOut = s;
    return fd;
}

KeyFilter::FontDescription KeyFilter::FontDescription::create( const QFont & f, bool b, bool i, bool s ) {
    FontDescription fd;
    fd.d->fullFont = true;
    fd.d->font = f;
    fd.d->bold = b;
    fd.d->italic = i;
    fd.d->strikeOut = s;
    return fd;
}

QFont KeyFilter::FontDescription::font( const QFont & base ) const {
    QFont font;
    if ( d->fullFont ) {
        font = d->font;
        font.setPointSize( base.pointSize() );
    } else {
        font = base;
    }
    if ( d->bold )
        font.setBold( true );
    if ( d->italic )
        font.setItalic( true );
    if ( d->strikeOut )
        font.setStrikeOut( true );
    return font;
}

KeyFilter::FontDescription KeyFilter::FontDescription::resolve( const FontDescription & other ) const {
    FontDescription fd;
    fd.d->fullFont = this->d->fullFont || other.d->fullFont ;
    if ( fd.d->fullFont )
        fd.d->font = this->d->fullFont ? this->d->font : other.d->font ;
    fd.d->bold = this->d->bold || other.d->bold ;
    fd.d->italic = this->d->italic || other.d->italic ;
    fd.d->strikeOut = this->d->strikeOut || other.d->strikeOut ;
    return fd;
}




static const struct {
  const char * name;
  Key::OwnerTrust trust;
  UserID::Validity validity;
} ownerTrustAndValidityMap[] = {
  { "unknown",   Key::Unknown,   UserID::Unknown   },
  { "undefined", Key::Undefined, UserID::Undefined },
  { "never",     Key::Never,     UserID::Never     },
  { "marginal",  Key::Marginal,  UserID::Marginal  },
  { "full",      Key::Full,      UserID::Full      },
  { "ultimate",  Key::Ultimate,  UserID::Ultimate  },
};

static Key::OwnerTrust map2OwnerTrust( const QString & s ) {
  for ( unsigned int i = 0 ; i < sizeof ownerTrustAndValidityMap / sizeof *ownerTrustAndValidityMap ; ++i )
    if ( s.toLower() == QLatin1String(ownerTrustAndValidityMap[i].name) )
      return ownerTrustAndValidityMap[i].trust;
  return ownerTrustAndValidityMap[0].trust;
}

static UserID::Validity map2Validity( const QString & s ) {
  for ( unsigned int i = 0 ; i < sizeof ownerTrustAndValidityMap / sizeof *ownerTrustAndValidityMap ; ++i )
    if ( s.toLower() == QLatin1String(ownerTrustAndValidityMap[i].name) )
      return ownerTrustAndValidityMap[i].validity;
  return ownerTrustAndValidityMap[0].validity;
}


KeyFilterImplBase::KeyFilterImplBase()
  : KeyFilter(),
    mMatchContexts( AnyMatchContext ),
    mSpecificity( 0 ),
    mItalic( false ),
    mBold( false ),
    mStrikeOut( false ),
    mUseFullFont( false ),
    mRevoked( DoesNotMatter ),
    mExpired( DoesNotMatter ),
    mDisabled( DoesNotMatter ),
    mRoot( DoesNotMatter ),
    mCanEncrypt( DoesNotMatter ),
    mCanSign( DoesNotMatter ),
    mCanCertify( DoesNotMatter ),
    mCanAuthenticate( DoesNotMatter ),
    mQualified( DoesNotMatter ),
    mCardKey( DoesNotMatter ),
    mHasSecret( DoesNotMatter ),
    mIsOpenPGP( DoesNotMatter ),
    mWasValidated( DoesNotMatter ),
    mOwnerTrust( LevelDoesNotMatter ),
    mOwnerTrustReferenceLevel( Key::Unknown ),
    mValidity( LevelDoesNotMatter ),
    mValidityReferenceLevel( UserID::Unknown )
{

}

KeyFilterImplBase::~KeyFilterImplBase() {}

KConfigBasedKeyFilter::KConfigBasedKeyFilter( const KConfigGroup & config )
  : KeyFilterImplBase()
{
  mFgColor = config.readEntry<QColor>( "foreground-color", QColor() );
  mBgColor = config.readEntry<QColor>( "background-color", QColor() );
  mName = config.readEntry( "Name", config.name() );
  mIcon = config.readEntry( "icon" );
  mId = config.readEntry( "id", config.name() );
  if ( config.hasKey( "font" ) ) {
    mUseFullFont = true;
    mFont = config.readEntry( "font" );
  } else {
    mUseFullFont = false;
    mItalic = config.readEntry( "font-italic", false );
    mBold = config.readEntry( "font-bold", false );
  }
  mStrikeOut = config.readEntry( "font-strikeout", false );
#ifdef SET
#undef SET
#endif
#define SET(member,key) \
  if ( config.hasKey( key ) ) { \
    member = config.readEntry( key, false ) ? Set : NotSet ; \
    ++mSpecificity; \
  }
  SET( mRevoked, "is-revoked" );
  SET( mExpired, "is-expired" );
  SET( mDisabled, "is-disabled" );
  SET( mRoot, "is-root-certificate" );
  SET( mCanEncrypt, "can-encrypt" );
  SET( mCanSign, "can-sign" );
  SET( mCanCertify, "can-certify" );
  SET( mCanAuthenticate, "can-authenticate" );
  SET( mQualified, "is-qualified" );
  SET( mCardKey, "is-cardkey" );
  SET( mHasSecret, "has-secret-key" );
  SET( mIsOpenPGP, "is-openpgp-key" );
  SET( mWasValidated, "was-validated" );
#undef SET
  static const struct {
    const char * prefix;
    LevelState state;
  } prefixMap[] = {
    { "is-", Is },
    { "is-not-", IsNot },
    { "is-at-least-", IsAtLeast },
    { "is-at-most-", IsAtMost },
  };
  for ( unsigned int i = 0 ; i < sizeof prefixMap / sizeof *prefixMap ; ++i ) {
    const QString key = QLatin1String( prefixMap[i].prefix ) + QLatin1String("ownertrust");
    if ( config.hasKey( key ) ) {
      mOwnerTrust = prefixMap[i].state;
      mOwnerTrustReferenceLevel = map2OwnerTrust( config.readEntry( key, QString() ) );
      ++mSpecificity;
      break;
    }
  }
  for ( unsigned int i = 0 ; i < sizeof prefixMap / sizeof *prefixMap ; ++i ) {
    const QString key = QLatin1String( prefixMap[i].prefix ) + QLatin1String("validity");
    if ( config.hasKey( key ) ) {
      mValidity = prefixMap[i].state;
      mValidityReferenceLevel = map2Validity( config.readEntry( key, QString() ) );
      ++mSpecificity;
      break;
    }
  }
  static const struct {
      const char * key;
      MatchContext context;
  } matchMap[] = {
      { "any", AnyMatchContext },
      { "appearance", Appearance },
      { "filtering", Filtering },
  };
  const QStringList contexts = config.readEntry( "match-contexts", "any" ).toLower().split( QRegExp( QLatin1String("[^a-zA-Z0-9_-!]+") ), QString::SkipEmptyParts );
  mMatchContexts = NoMatchContext;
  Q_FOREACH( const QString & ctx, contexts ) {
      bool found = false;
      for ( unsigned int i = 0 ; i < sizeof matchMap / sizeof *matchMap ; ++i )
          if ( ctx == QLatin1String(matchMap[i].key) ) {
              mMatchContexts |= matchMap[i].context;
              found = true;
              break;
          } else if ( ctx.startsWith( QLatin1Char('!') ) && ctx.mid( 1 ) == QLatin1String(matchMap[i].key) ) {
              mMatchContexts &= ~matchMap[i].context;
              found = true;
              break;
          }
      if ( !found )   
          qWarning( "KConfigBasedKeyFilter: found unknown match context '%s' in group '%s'",
                    qPrintable( ctx ), qPrintable( config.name() ) );
  }
  if ( mMatchContexts == NoMatchContext ) {
      qWarning( "KConfigBasedKeyFilter: match context in group '%s' evaluates to NoMatchContext, "
                "replaced by AnyMatchContext", qPrintable( config.name() ) );
      mMatchContexts = AnyMatchContext;
  }
}

static bool is_card_key( const Key & key ) {
    const std::vector<Subkey> sks = key.subkeys();
    return std::find_if( sks.begin(), sks.end(),
                         mem_fn( &Subkey::isCardKey ) ) != sks.end() ;
}

bool KeyFilterImplBase::matches( const Key & key, MatchContexts contexts ) const {
  if ( !( mMatchContexts & contexts ) )
    return false;
#ifdef MATCH
#undef MATCH
#endif
#define MATCH(member,method) \
  if ( member != DoesNotMatter && key.method() != bool( member == Set ) ) \
    return false
#define IS_MATCH(what) MATCH( m##what, is##what )
#define CAN_MATCH(what) MATCH( mCan##what, can##what )
  IS_MATCH( Revoked );
  IS_MATCH( Expired );
  IS_MATCH( Disabled );
  IS_MATCH( Root );
  CAN_MATCH( Encrypt );
  CAN_MATCH( Sign );
  CAN_MATCH( Certify );
  CAN_MATCH( Authenticate );
  IS_MATCH( Qualified );
  if ( mCardKey != DoesNotMatter )
      if ( ( mCardKey == Set    && !is_card_key( key ) ) ||
           ( mCardKey == NotSet &&  is_card_key( key ) )   )
          return false;
  MATCH( mHasSecret, hasSecret );
#undef MATCH
  if ( mIsOpenPGP != DoesNotMatter &&
       bool( key.protocol() == GpgME::OpenPGP ) != bool( mIsOpenPGP == Set ) )
    return false;
  if ( mWasValidated != DoesNotMatter &&
       bool( key.keyListMode() & GpgME::Validate ) != bool( mWasValidated == Set ) )
    return false;
  switch ( mOwnerTrust ) {
  default:
  case LevelDoesNotMatter:
    break;
  case Is:
    if ( key.ownerTrust() != mOwnerTrustReferenceLevel )
      return false;
    break;
  case IsNot:
    if ( key.ownerTrust() == mOwnerTrustReferenceLevel )
      return false;
    break;
  case IsAtLeast:
    if ( (int)key.ownerTrust() < (int)mOwnerTrustReferenceLevel )
      return false;
    break;
  case IsAtMost:
    if ( (int)key.ownerTrust() > (int)mOwnerTrustReferenceLevel )
      return false;
    break;
  }
  const UserID uid = key.userID(0);
  switch ( mValidity ) {
  default:
  case LevelDoesNotMatter:
    break;
  case Is:
    if ( uid.validity() != mValidityReferenceLevel )
      return false;
    break;
  case IsNot:
    if ( uid.validity() == mValidityReferenceLevel )
      return false;
    break;
  case IsAtLeast:
    if ( (int)uid.validity() < (int)mValidityReferenceLevel )
      return false;
    break;
  case IsAtMost:
    if ( (int)uid.validity() > (int)mValidityReferenceLevel )
      return false;
    break;
  }
  return true;
}

KeyFilter::FontDescription KeyFilterImplBase::fontDesription() const {
  if ( mUseFullFont )
    return FontDescription::create( mFont, mBold, mItalic, mStrikeOut );
  else
    return FontDescription::create( mBold, mItalic, mStrikeOut );
}
