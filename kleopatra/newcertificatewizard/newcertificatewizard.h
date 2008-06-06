#ifndef __KLEOPATRA_NEWCERTIFICATEWIZARD_NEWCERTIFICATEWIZARD_H__
#define __KLEOPATRA_NEWCERTIFICATEWIZARD_NEWCERTIFICATEWIZARD_H__

#include <QWizard>

#include <utils/pimpl_ptr.h>

namespace Kleo {

    class NewCertificateWizard : public QWizard {
        Q_OBJECT
    public:
        explicit NewCertificateWizard( QWidget * parent=0 );
        ~NewCertificateWizard();

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
    };

}

#endif /* __KLEOPATRA_NEWCERTIFICATEWIZARD_NEWCERTIFICATEWIZARD_H__ */
