#ifndef __KLEOPATRA_NEWCERTIFICATEWIZARD_LISTWIDGET_H__
#define __KLEOPATRA_NEWCERTIFICATEWIZARD_LISTWIDGET_H__

#include <QWidget>

#include <utils/pimpl_ptr.h>

namespace Kleo {
namespace NewCertificateUi {

    class ListWidget : public QWidget {
        Q_OBJECT
        Q_PROPERTY( QStringList items READ items WRITE setItems USER true NOTIFY itemsChanged )
    public:
        explicit ListWidget( QWidget * parent=0 );
        ~ListWidget();

        QStringList items() const;

    public Q_SLOTS:
        void setItems( const QStringList & items );

    Q_SIGNALS:
        void itemsChanged();

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
        Q_PRIVATE_SLOT( d, void slotAdd() )
        Q_PRIVATE_SLOT( d, void slotRemove() )
        Q_PRIVATE_SLOT( d, void slotUp() )
        Q_PRIVATE_SLOT( d, void slotDown() )
        Q_PRIVATE_SLOT( d, void slotSelectionChanged() )
    };

}
}

#endif /* __KLEOPATRA_NEWCERTIFICATEWIZARD_LISTWIDGET_H__ */
