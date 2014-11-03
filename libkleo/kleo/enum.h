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

#ifndef __KLEO_ENUM_H__
#define __KLEO_ENUM_H__

#include "kleo_export.h"

class QString;
class QStringList;

namespace Kleo
{

enum CryptoMessageFormat {
    InlineOpenPGPFormat = 1,
    OpenPGPMIMEFormat = 2,
    SMIMEFormat = 4,
    SMIMEOpaqueFormat = 8,
    AnyOpenPGP = InlineOpenPGPFormat | OpenPGPMIMEFormat,
    AnySMIME = SMIMEOpaqueFormat | SMIMEFormat,
    AutoFormat = AnyOpenPGP | AnySMIME
};

KLEO_EXPORT QString cryptoMessageFormatToLabel(CryptoMessageFormat f);

KLEO_EXPORT const char *cryptoMessageFormatToString(CryptoMessageFormat f);
KLEO_EXPORT QStringList cryptoMessageFormatsToStringList(unsigned int f);
KLEO_EXPORT CryptoMessageFormat stringToCryptoMessageFormat(const QString &s);
KLEO_EXPORT unsigned int stringListToCryptoMessageFormats(const QStringList &sl);

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

KLEO_EXPORT QString encryptionPreferenceToLabel(EncryptionPreference pref);
KLEO_EXPORT const char *encryptionPreferenceToString(EncryptionPreference pref);
KLEO_EXPORT EncryptionPreference stringToEncryptionPreference(const QString &str);

enum SigningPreference {
    UnknownSigningPreference = 0,
    NeverSign = 1,
    AlwaysSign = 2,
    AlwaysSignIfPossible = 3,
    AlwaysAskForSigning = 4,
    AskSigningWheneverPossible = 5,
    MaxSigningPreference = AskSigningWheneverPossible
};

KLEO_EXPORT QString signingPreferenceToLabel(SigningPreference pref);
KLEO_EXPORT const char *signingPreferenceToString(SigningPreference pref);
KLEO_EXPORT SigningPreference stringToSigningPreference(const QString &str);
}

#endif // __KLEO_CRYPTOBACKEND_H__
