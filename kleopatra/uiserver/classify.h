#ifndef __KLEOPATRA_UISERVER_CLASSIFY_H__
#define __KLEOPATRA_UISERVER_CLASSIFY_H__

class QString;

namespace Kleo {

    namespace Class {
        enum {
            NoClass = 0,

            // protocol:
            CMS          = 0x01,
            OpenPGP      = 0x02,

            AnyProtocol  = OpenPGP|CMS,
            ProtocolMask = AnyProtocol,

            // format:
            Binary     = 0x04,
            Ascii      = 0x08,

            AnyFormat  = Binary|Ascii,
            FormatMask = AnyFormat,
        
            // type:
            DetachedSignature  = 0x010,
            OpaqueSignature    = 0x020,

            AnySignature       = DetachedSignature|OpaqueSignature,

            CipherText         = 0x040,

            AnyMessageType     = DetachedSignature|OpaqueSignature|CipherText,

            Certificate        = 0x080,
            ExportedPSM        = 0x100,

            AnyCertStoreType   = Certificate|ExportedPSM,

            CertificateRequest = 0x200,

            AnyType            = AnyMessageType|AnyCertStoreType|CertificateRequest,
            TypeMask           = AnyType
        };
    };

    unsigned int classify( const QString & filename );

#define make_convenience( What, Mask )                                  \
    inline bool is##What( const QString & filename ) {                  \
        return ( classify( filename ) & Class::Mask ) == Class::What ;  \
    }                                                                   \
    inline bool mayBe##What( const QString & filename ) {               \
        return classify( filename ) & Class::What ;                     \
    }

    make_convenience( CMS,     ProtocolMask )
    make_convenience( OpenPGP, ProtocolMask )

    make_convenience( Binary, FormatMask )
    make_convenience( Ascii,  FormatMask )

    make_convenience( DetachedSignature, TypeMask )
    make_convenience( OpaqueSignature,   TypeMask )
    make_convenience( CipherText,        TypeMask )
#undef make_convenience

}

#endif /* __KLEOPATRA_UISERVER_CLASSIFY_H__ */
