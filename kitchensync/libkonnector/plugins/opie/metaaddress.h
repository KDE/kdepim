
#ifndef OpieHelper_MetaAddress
#define OpieHelper_MetaAddress

#include <qvaluelist.h>

#include <kabc/addressee.h>

namespace OpieHelper {
    class MetaAddress;
    class MetaAddressReturn {
        friend class MetaAddress;
    public:
        MetaAddressReturn();
        ~MetaAddressReturn();
        MetaAddressReturn( const MetaAddressReturn& ole );
        QValueList<KABC::Addressee> added();
        QValueList<KABC::Addressee> modified();
        QValueList<KABC::Addressee> removed();
        MetaAddressReturn &operator=(const MetaAddressReturn& ole );
    private:
        QValueList<KABC::Addressee> m_add;
        QValueList<KABC::Addressee> m_mod;
        QValueList<KABC::Addressee> m_rem;

    };
    class MetaAddress {
    public:
        MetaAddress();
        ~MetaAddress();
        MetaAddressReturn doMeta( const QValueList<KABC::Addressee>&,
                                  const QValueList<KABC::Addressee>& );

    };
};

#endif
