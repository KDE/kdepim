/*
    kleo/enum.h

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#ifndef __KLEO_ENUM_H__
#define __KLEO_ENUM_H__

#include <kdepimmacros.h>

class QString;
class QStringList;

namespace Kleo {

  enum CryptoMessageFormat {
    InlineOpenPGPFormat = 1,
    OpenPGPMIMEFormat = 2,
    SMIMEFormat = 4,
    SMIMEOpaqueFormat = 8,
    AnyOpenPGP = InlineOpenPGPFormat|OpenPGPMIMEFormat,
    AnySMIME = SMIMEOpaqueFormat|SMIMEFormat,
    AutoFormat = AnyOpenPGP|AnySMIME
  };

  KDE_EXPORT QString cryptoMessageFormatToLabel( CryptoMessageFormat f );

  KDE_EXPORT const char * cryptoMessageFormatToString( CryptoMessageFormat f );
  KDE_EXPORT QStringList cryptoMessageFormatsToStringList( unsigned int f );
  KDE_EXPORT CryptoMessageFormat stringToCryptoMessageFormat( const QString & s );
  KDE_EXPORT unsigned int stringListToCryptoMessageFormats( const QStringList & sl );

  enum Action {
    Conflict, DoIt, DontDoIt, Ask, AskOpportunistic, Impossible
  };

  enum EncryptionPreference {
    UnknownPreference = 0,
    NeverEncrypt = 1,
    AlwaysEncrypt = 2,
    AlwaysEncryptIfPossible = 3,
    AlwaysAskForEncryption = 4,
    AskWheneverPossible = 5,
    MaxEncryptionPreference = AskWheneverPossible
  };

  KDE_EXPORT QString encryptionPreferenceToLabel( EncryptionPreference pref );
  KDE_EXPORT const char* encryptionPreferenceToString( EncryptionPreference pref );
  KDE_EXPORT EncryptionPreference stringToEncryptionPreference( const QString& str );

  enum SigningPreference {
    UnknownSigningPreference = 0,
    NeverSign = 1,
    AlwaysSign = 2,
    AlwaysSignIfPossible = 3,
    AlwaysAskForSigning = 4,
    AskSigningWheneverPossible = 5,
    MaxSigningPreference = AskSigningWheneverPossible
  };

  KDE_EXPORT QString signingPreferenceToLabel( SigningPreference pref ) KDE_EXPORT;
  KDE_EXPORT const char* signingPreferenceToString( SigningPreference pref ) KDE_EXPORT;
  KDE_EXPORT SigningPreference stringToSigningPreference( const QString& str );
}

#endif // __KLEO_CRYPTOBACKEND_H__
