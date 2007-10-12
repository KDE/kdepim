/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/classify.cpp

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

#include "classify.h"

#include <QString>
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QtAlgorithms>

#include <boost/range.hpp>

#ifdef __GNUC__
# include <ext/algorithm>
#endif

#include <functional>

using namespace boost;
using namespace Kleo::Class;

namespace {

    static const struct _classification {
        char extension[4];
        unsigned int classification;
    } classifications[] = {
        // ordered by extension
        { "asc", OpenPGP|  Ascii  | OpaqueSignature|CipherText|AnyCertStoreType },
        { "crt", CMS    | Binary  | Certificate },
        { "der", CMS    | Binary  | Certificate },
        { "gpg", OpenPGP| Binary  | OpaqueSignature|CipherText|AnyCertStoreType },
        { "p10", CMS    |  Ascii  | CertificateRequest },
        { "p12", CMS    | Binary  | ExportedPSM },
        { "pem", CMS    |  Ascii  | AnyType },
        { "p7c", CMS    | Binary  | Certificate  },
        { "p7m", CMS    | Binary  | CipherText },
        { "p7s", CMS    | Binary  | AnySignature },
        { "sig", OpenPGP|AnyFormat| DetachedSignature },
    };

    static const unsigned int defaultClassification = NoClass;

    template <template <typename U> class Op>
    struct ByExtension {
        typedef bool result_type;

        template <typename T>
        bool operator()( const T & lhs, const T & rhs ) const {
            return Op<int>()( qstricmp( lhs.extension, rhs.extension ), 0 );
        }
        template <typename T>
        bool operator()( const T & lhs, const char * rhs ) const {
            return Op<int>()( qstricmp( lhs.extension, rhs ), 0 );
        }
        template <typename T>
        bool operator()( const char * lhs, const T & rhs ) const {
            return Op<int>()( qstricmp( lhs, rhs.extension ), 0 );
        }
        bool operator()( const char * lhs, const char * rhs ) const {
            return Op<int>()( qstricmp( lhs, rhs ), 0 );
        }
    };

}


unsigned int Kleo::classify( const QString & filename ) {
#ifdef __GNUC__
    assert( __gnu_cxx::is_sorted( begin( classifications ), end( classifications ), ByExtension<std::less>() ) );
#endif

    const QFileInfo fi( filename );

    const _classification * const it = qBinaryFind( begin( classifications ), end( classifications ),
                                                    fi.suffix().toLatin1().constData(),
                                                    ByExtension<std::less>() );
    if ( it == end( classifications ) )
        return defaultClassification;
    else
        return it->classification;
}

static QString chopped( QString s, unsigned int n ) {
    s.chop( n );
    return s;
}

/*!
  \return the data file that corresponds to the signature file \a
  signatureFileName, or QString(), if no such file can be found.
*/
QString Kleo::findSignedData( const QString & signatureFileName ) {
    if ( !isDetachedSignature( signatureFileName ) )
        return QString();
    const QString baseName = chopped( signatureFileName, 4 );
    return QFile::exists( baseName ) ? baseName : QString() ;
}

/*!
  \return all (existing) candiate signature files for \a signedDataFileName

  Note that there can very well be more than one such file, e.g. if
  the same data file was signed by both CMS and OpenPGP certificates.
*/
QStringList Kleo::findSignatures( const QString & signedDataFileName ) {
    QStringList result;
    for ( unsigned int i = 0, end = size( classifications ) ; i < end ; ++i )
        if ( classifications[i].classification & DetachedSignature ) {
            const QString candiate = signedDataFileName + '.' + classifications[i].extension;
            if ( QFile::exists( candiate ) )
                result.push_back( candiate );
        }
    return result;
}
