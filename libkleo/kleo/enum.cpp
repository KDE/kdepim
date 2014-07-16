/*
    kleo/enum.cpp

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

#include "enum.h"

#include <klocale.h>

#include <QString>
#include <QStringList>

static const struct {
  Kleo::CryptoMessageFormat format;
  const char * displayName;
  const char * configName;
} cryptoMessageFormats[] = {
  { Kleo::InlineOpenPGPFormat,
    I18N_NOOP("Inline OpenPGP (deprecated)"),
    "inline openpgp" },
  { Kleo::OpenPGPMIMEFormat,
    I18N_NOOP("OpenPGP/MIME"),
    "openpgp/mime" },
  { Kleo::SMIMEFormat,
    I18N_NOOP("S/MIME"),
    "s/mime" },
  { Kleo::SMIMEOpaqueFormat,
    I18N_NOOP("S/MIME Opaque"),
    "s/mime opaque" },
};
static const unsigned int numCryptoMessageFormats
  = sizeof cryptoMessageFormats / sizeof *cryptoMessageFormats ;

const char * Kleo::cryptoMessageFormatToString( Kleo::CryptoMessageFormat f ) {
  if ( f == AutoFormat )
    return "auto";
  for ( unsigned int i = 0 ; i < numCryptoMessageFormats ; ++i )
    if ( f == cryptoMessageFormats[i].format )
      return cryptoMessageFormats[i].configName;
  return 0;
}

QStringList Kleo::cryptoMessageFormatsToStringList( unsigned int f ) {
  QStringList result;
  for ( unsigned int i = 0 ; i < numCryptoMessageFormats ; ++i )
    if ( f & cryptoMessageFormats[i].format )
      result.push_back( QLatin1String(cryptoMessageFormats[i].configName) );
  return result;
}

QString Kleo::cryptoMessageFormatToLabel( Kleo::CryptoMessageFormat f ) {
  if ( f == AutoFormat )
    return i18n("Any");
  for ( unsigned int i = 0 ; i < numCryptoMessageFormats ; ++i )
    if ( f == cryptoMessageFormats[i].format )
      return i18n( cryptoMessageFormats[i].displayName );
  return QString();
}

Kleo::CryptoMessageFormat Kleo::stringToCryptoMessageFormat( const QString & s ) {
  const QString t = s.toLower();
  for ( unsigned int i = 0 ; i < numCryptoMessageFormats ; ++i )
    if ( t == QLatin1String(cryptoMessageFormats[i].configName) )
      return cryptoMessageFormats[i].format;
  return AutoFormat;
}

unsigned int Kleo::stringListToCryptoMessageFormats( const QStringList & sl ) {
  unsigned int result = 0;
  for ( QStringList::const_iterator it = sl.begin() ; it != sl.end() ; ++it )
    result |= stringToCryptoMessageFormat( *it );
  return result;
}

// For the config values used below, see also kaddressbook/editors/cryptowidget.cpp

const char* Kleo::encryptionPreferenceToString( EncryptionPreference pref )
{
  switch( pref ) {
  case UnknownPreference:
    return 0;
  case NeverEncrypt:
    return "never";
  case AlwaysEncrypt:
    return "always";
  case AlwaysEncryptIfPossible:
    return "alwaysIfPossible";
  case AlwaysAskForEncryption:
    return "askAlways";
  case AskWheneverPossible:
    return "askWhenPossible";
  }
  return 0; // keep the compiler happy
}

Kleo::EncryptionPreference Kleo::stringToEncryptionPreference( const QString& str )
{
  if ( str == QLatin1String("never") )
    return NeverEncrypt;
  if ( str == QLatin1String("always") )
    return AlwaysEncrypt;
  if ( str == QLatin1String("alwaysIfPossible") )
    return AlwaysEncryptIfPossible;
  if ( str == QLatin1String("askAlways"))
    return AlwaysAskForEncryption;
  if ( str == QLatin1String("askWhenPossible") )
    return AskWheneverPossible;
  return UnknownPreference;
}

QString Kleo::encryptionPreferenceToLabel( EncryptionPreference pref )
{
  switch( pref ) {
  case NeverEncrypt:
    return i18n( "Never Encrypt" );
  case AlwaysEncrypt:
    return i18n( "Always Encrypt" );
  case AlwaysEncryptIfPossible:
    return i18n( "Always Encrypt If Possible" );
  case AlwaysAskForEncryption:
    return i18n( "Ask" );
  case AskWheneverPossible:
    return i18n( "Ask Whenever Possible" );
  default:
    return xi18nc( "no specific preference", "<placeholder>none</placeholder>" );
  }
}

const char* Kleo::signingPreferenceToString( SigningPreference pref )
{
  switch( pref ) {
  case UnknownSigningPreference:
    return 0;
  case NeverSign:
    return "never";
  case AlwaysSign:
    return "always";
  case AlwaysSignIfPossible:
    return "alwaysIfPossible";
  case AlwaysAskForSigning:
    return "askAlways";
  case AskSigningWheneverPossible:
    return "askWhenPossible";
  }
  return 0; // keep the compiler happy
}

Kleo::SigningPreference Kleo::stringToSigningPreference( const QString& str )
{
  if ( str == QLatin1String("never") )
    return NeverSign;
  if ( str == QLatin1String("always") )
    return AlwaysSign;
  if ( str == QLatin1String("alwaysIfPossible") )
    return AlwaysSignIfPossible;
  if ( str == QLatin1String("askAlways") )
    return AlwaysAskForSigning;
  if ( str == QLatin1String("askWhenPossible") )
    return AskSigningWheneverPossible;
  return UnknownSigningPreference;
}

QString Kleo::signingPreferenceToLabel( SigningPreference pref )
{
  switch( pref ) {
  case NeverSign:
    return i18n( "Never Sign" );
  case AlwaysSign:
    return i18n( "Always Sign" );
  case AlwaysSignIfPossible:
    return i18n( "Always Sign If Possible" );
  case AlwaysAskForSigning:
    return i18n( "Ask" );
  case AskSigningWheneverPossible:
    return i18n( "Ask Whenever Possible" );
  default:
    return i18nc( "no specific preference", "<none>" );
  }
}
