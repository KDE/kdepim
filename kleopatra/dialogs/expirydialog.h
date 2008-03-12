#ifndef __KLEOPATRA_DIALOGS_EXIPRYDIALOG_H__
#define __KLEOPATRA_DIALOGS_EXIPRYDIALOG_H__

#include <QDialog>

#include <utils/pimpl_ptr.h>

class QDate;

namespace Kleo {
namespace Dialogs {

    class ExpiryDialog : public QDialog {
        Q_OBJECT
        Q_PROPERTY( QDate dateOfExpiry READ dateOfExpiry WRITE setDateOfExpiry )
    public:
        explicit ExpiryDialog( QWidget * parent=0, Qt::WindowFlags f=0 );
        ~ExpiryDialog();

        void setDateOfExpiry( const QDate & date );
        QDate dateOfExpiry() const;

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
        Q_PRIVATE_SLOT( d, void slotInAmountChanged() )
        Q_PRIVATE_SLOT( d, void slotInUnitChanged() )
        Q_PRIVATE_SLOT( d, void slotOnDateChanged() )
    };

}
}

#endif /* __KLEOPATRA_DIALOGS_EXIPRYDIALOG_H__ */
