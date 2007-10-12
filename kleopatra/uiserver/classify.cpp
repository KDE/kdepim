#include "classify.h"

#include <QString>
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
