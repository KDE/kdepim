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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kconfigbasedkeyfilter.h"

#include <kconfigbase.h>
#include <klocale.h>

static const struct {
  const char * name;
  GpgME::Key::OwnerTrust trust;
  GpgME::UserID::Validity validity;
} ownerTrustAndValidityMap[] = {
  { "unknown",   GpgME::Key::Unknown,   GpgME::UserID::Unknown   },
  { "undefined", GpgME::Key::Undefined, GpgME::UserID::Undefined },
  { "never",     GpgME::Key::Never,     GpgME::UserID::Never     },
  { "marginal",  GpgME::Key::Marginal,  GpgME::UserID::Marginal  },
  { "full",      GpgME::Key::Full,      GpgME::UserID::Full      },
  { "ultimate",  GpgME::Key::Ultimate,  GpgME::UserID::Ultimate  },
};

static GpgME::Key::OwnerTrust map2OwnerTrust( const QString & s ) {
  for ( unsigned int i = 0 ; i < sizeof ownerTrustAndValidityMap / sizeof *ownerTrustAndValidityMap ; ++i )
    if ( s.lower() == ownerTrustAndValidityMap[i].name )
      return ownerTrustAndValidityMap[i].trust;
  return ownerTrustAndValidityMap[0].trust;
}

static GpgME::UserID::Validity map2Validity( const QString & s ) {
  for ( unsigned int i = 0 ; i < sizeof ownerTrustAndValidityMap / sizeof *ownerTrustAndValidityMap ; ++i )
    if ( s.lower() == ownerTrustAndValidityMap[i].name )
      return ownerTrustAndValidityMap[i].validity;
  return ownerTrustAndValidityMap[0].validity;
}


Kleo::KConfigBasedKeyFilter::KConfigBasedKeyFilter( const KConfigBase & config )
  : KeyFilter(),
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
    mHasSecret( DoesNotMatter ),
    mIsOpenPGP( DoesNotMatter ),
    mWasValidated( DoesNotMatter ),
    mOwnerTrust( LevelDoesNotMatter ),
    mOwnerTrustReferenceLevel( GpgME::Key::Unknown ),
    mValidity( LevelDoesNotMatter ),
    mValidityReferenceLevel( GpgME::UserID::Unknown )
{
  mFgColor = config.readColorEntry( "foreground-color" );
  mBgColor = config.readColorEntry( "background-color" );
  mName = config.readEntry( "name", i18n("<unnamed>") );
  mIcon = config.readEntry( "icon" );
  if ( config.hasKey( "font" ) ) {
    mUseFullFont = true;
    mFont = config.readFontEntry( "font" );
  } else {
    mItalic = config.readBoolEntry( "font-italic", false );
    mBold = config.readBoolEntry( "font-bold", false );
  }
  mStrikeOut = config.readBoolEntry( "font-strikeout", false );
#ifdef SET
#undef SET
#endif
#define SET(member,key) \
  if ( config.hasKey( key ) ) { \
    member = config.readBoolEntry( key ) ? Set : NotSet ; \
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
    const QString key = QString( prefixMap[i].prefix ) + "ownertrust";
    if ( config.hasKey( key ) ) {
      mOwnerTrust = prefixMap[i].state;
      mOwnerTrustReferenceLevel = map2OwnerTrust( config.readEntry( key ) );
      ++mSpecificity;
      break;
    }
  }
  for ( unsigned int i = 0 ; i < sizeof prefixMap / sizeof *prefixMap ; ++i ) {
    const QString key = QString( prefixMap[i].prefix ) + "validity";
    if ( config.hasKey( key ) ) {
      mValidity = prefixMap[i].state;
      mValidityReferenceLevel = map2Validity( config.readEntry( key ) );
      ++mSpecificity;
      break;
    }
  }
}

Kleo::KConfigBasedKeyFilter::~KConfigBasedKeyFilter() {

}

bool Kleo::KConfigBasedKeyFilter::matches( const GpgME::Key & key ) const {
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
  MATCH( mHasSecret, isSecret );
#undef MATCH
  if ( mIsOpenPGP != DoesNotMatter &&
       bool( key.protocol() == GpgME::Context::OpenPGP ) != bool( mIsOpenPGP == Set ) )
    return false;
  if ( mWasValidated != DoesNotMatter &&
       bool( key.keyListMode() & GpgME::Context::Validate ) != bool( mWasValidated == Set ) )
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
  const GpgME::UserID uid = key.userID(0);
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

static inline QFont resizedFont( QFont font, int pointSize, bool strike ) {
  font.setPointSize( pointSize );
  if ( strike )
    font.setStrikeOut( true );
  return font;
}

static inline QFont adapt( QFont font, bool it, bool b, bool strike ) {
  if ( it )
    font.setItalic( true );
  if ( b )
    font.setBold( true );
  if ( strike )
    font.setStrikeOut( true );
  return font;
}

QFont Kleo::KConfigBasedKeyFilter::font( const QFont & f ) const {
  if ( mUseFullFont )
    return resizedFont( mFont, f.pointSize(), mStrikeOut );
  else
    return adapt( f, mItalic, mBold, mStrikeOut );
}
