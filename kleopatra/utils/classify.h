/* -*- mode: c++; c-basic-offset:4 -*-
    utils/classify.h

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

#include <gpgme++/global.h>

#ifndef __KLEOPATRA_UISERVER_CLASSIFY_H__
#define __KLEOPATRA_UISERVER_CLASSIFY_H__

class QString;
class QStringList;

namespace Kleo
{

namespace Class
{
enum {
    NoClass = 0,

    // protocol:
    CMS          = 0x01,
    OpenPGP      = 0x02,

    AnyProtocol  = OpenPGP | CMS,
    ProtocolMask = AnyProtocol,

    // format:
    Binary     = 0x04,
    Ascii      = 0x08,

    AnyFormat  = Binary | Ascii,
    FormatMask = AnyFormat,

    // type:
    DetachedSignature  = 0x010,
    OpaqueSignature    = 0x020,
    ClearsignedMessage = 0x040,

    AnySignature       = DetachedSignature | OpaqueSignature | ClearsignedMessage,

    CipherText         = 0x080,

    AnyMessageType     = AnySignature | CipherText,

    Importable         = 0x100,
    Certificate        = 0x200 | Importable,
    ExportedPSM        = 0x400 | Importable,

    AnyCertStoreType   = Certificate | ExportedPSM,

    CertificateRequest = 0x800,

    CertificateRevocationList = 0x1000,

    AnyType            = AnyMessageType | AnyCertStoreType | CertificateRequest | CertificateRevocationList,
    TypeMask           = AnyType
};
}

unsigned int classify(const QString &filename);
unsigned int classify(const QStringList &fileNames);
unsigned int classifyContent(const QByteArray &data);

QString findSignedData(const QString &signatureFileName);
QStringList findSignatures(const QString &signedDataFileName);
QString outputFileName(const QString &input);

const char *outputFileExtension(unsigned int classification);

QString printableClassification(unsigned int classification);

#define make_convenience( What, Mask )                                  \
    inline bool is##What( const QString & filename ) {                  \
        return ( classify( filename ) & Class::Mask ) == Class::What ;  \
    }                                                                   \
    inline bool is##What( const unsigned int classifcation ) {          \
        return ( classifcation & Class::Mask ) == Class::What ;         \
    }                                                                   \
    inline bool mayBe##What( const QString & filename ) {               \
        return classify( filename ) & Class::What ;                     \
    }                                                                   \
    inline bool mayBe##What( const unsigned int classifcation ) {       \
        return classifcation & Class::What ;                            \
    }

make_convenience(CMS,     ProtocolMask)
make_convenience(OpenPGP, ProtocolMask)

make_convenience(Binary, FormatMask)
make_convenience(Ascii,  FormatMask)

make_convenience(DetachedSignature, TypeMask)
make_convenience(OpaqueSignature,   TypeMask)
make_convenience(CipherText,        TypeMask)
make_convenience(AnyMessageType,    TypeMask)
make_convenience(CertificateRevocationList, TypeMask)
make_convenience(AnyCertStoreType,  TypeMask)
#undef make_convenience

inline GpgME::Protocol findProtocol(const unsigned int classifcation)
{
    if (isOpenPGP(classifcation)) {
        return GpgME::OpenPGP;
    } else if (isCMS(classifcation)) {
        return GpgME::CMS;
    } else {
        return GpgME::UnknownProtocol;
    }
}
inline GpgME::Protocol findProtocol(const QString &filename)
{
    return findProtocol(classify(filename));
}

}

#endif /* __KLEOPATRA_UISERVER_CLASSIFY_H__ */
