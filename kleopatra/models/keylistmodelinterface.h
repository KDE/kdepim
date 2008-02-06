#ifndef __KLEOPATRA_MODELS_KEYLISTMODELINTERFACE_H__
#define __KLEOPATRA_MODELS_KEYLISTMODELINTERFACE_H__

#include <vector>

namespace GpgME {
    class Key;
}

class QModelIndex;
template <typename T> class QList;

namespace Kleo {

    class KeyListModelInterface {
    public:
        virtual ~KeyListModelInterface() {}

        virtual GpgME::Key key( const QModelIndex & idx ) const = 0;
        virtual std::vector<GpgME::Key> keys( const QList<QModelIndex> & idxs ) const = 0;

        virtual QModelIndex index( const GpgME::Key & key ) const = 0;
        virtual QList<QModelIndex> indexes( const std::vector<GpgME::Key> & keys ) const = 0;
    };

}

#endif /* __KLEOPATRA_MODELS_KEYLISTMODELINTERFACE_H__ */
