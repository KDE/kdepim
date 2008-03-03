#ifndef __KLEOPATRA_UTILS_HEADERVIEW_H__
#define __KLEOPATRA_UTILS_HEADERVIEW_H__

#include <QHeaderView>

#include <utils/pimpl_ptr.h>

#include <vector>

namespace Kleo {

    class HeaderView : public QHeaderView {
        Q_OBJECT
    public:
        explicit HeaderView( Qt::Orientation o, QWidget * parent=0 );
        ~HeaderView();

        void setSectionSizes( const std::vector<int> & sizes );
        std::vector<int> sectionSizes() const;

        void setSectionResizeMode( unsigned int logicalIndex, ResizeMode mode );
        ResizeMode sectionResizeMode( unsigned int logicalIndex ) const;

        /* reimp */ void setModel( QAbstractItemModel * model );
        /* reimp */ void setRootIndex( const QModelIndex & idx );

    private:
        //@{
        /*! Defined, but not implemented, to catch at least some usage errors */
        void setResizeMode( int, ResizeMode );
        ResizeMode resizeMode() const;
        //@}

    protected:
        /* reimp */ void updateGeometries();
        /* reimp */ void mousePressEvent( QMouseEvent * e );
        /* reimp */ void mouseReleaseEvent( QMouseEvent * e );

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
    };

}

#endif /* __KLEOPATRA_UTILS_HEADERVIEW_H__ */
