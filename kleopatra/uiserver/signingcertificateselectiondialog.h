#ifndef __KLEOPATRA_SIGNINGCERTIFICATESELECTIONDIALOG_H__
#define __KLEOPATRA_SIGNINGCERTIFICATESELECTIONDIALOG_H__


#include <KDialog>
    
#include <gpgme++/key.h>

#include <utils/pimpl_ptr.h>

template <typename K, typename U> class QMap;

namespace Kleo {

    class SigningCertificateSelectionDialog : public KDialog {
        Q_OBJECT
            public:
        explicit SigningCertificateSelectionDialog( QWidget * parent=0, Qt::WFlags f=0 );
        ~SigningCertificateSelectionDialog();
        
        void setSelectedCertificates( const QMap<GpgME::Protocol, GpgME::Key>& certificates );
        QMap<GpgME::Protocol, GpgME::Key> selectedCertificates() const;

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
    };
}

#endif // __KLEOPATRA_SIGNINGCERTIFICATESELECTIONDIALOG_H__
 
