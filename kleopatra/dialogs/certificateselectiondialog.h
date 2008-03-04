#ifndef __KLEOPATRA_DIALOGS_CERTIFICATESELECTIONDIALOG_H__
#define __KLEOPATRA_DIALOGS_CERTIFICATESELECTIONDIALOG_H__

#include <QDialog>

#include <utils/pimpl_ptr.h>

#include <vector>

namespace GpgME {
    class Key;
}

namespace boost {
    template <typename T> class shared_ptr;
}

namespace Kleo {

    class KeyFilter;

namespace Dialogs {

    class CertificateSelectionDialog : public QDialog {
        Q_OBJECT
        Q_FLAGS( Options )
    public:
        enum Option {
            SingleSelection = 0x00,
            MultiSelection  = 0x01,

            SignOnly        = 0x02,
            EncryptOnly     = 0x04,
            AnyCertificate  = 0x06,

            OpenPGPFormat   = 0x08,
            CMSFormat       = 0x10,
            AnyFormat       = 0x18,

            Certificates    = 0x00,
            SecretKeys      = 0x20,

            OptionMask
        };
        Q_DECLARE_FLAGS( Options, Option )

        explicit CertificateSelectionDialog( QWidget * parent=0, Qt::WindowFlags f=0 );
        ~CertificateSelectionDialog();

        void setCustomLabelText( const QString & text );
        QString customLabelText() const;

        void setOptions( Options options );
        Options options() const;

        void selectCertificates( const std::vector<GpgME::Key> & certs );
        void selectCertificate( const GpgME::Key & key );


        std::vector<GpgME::Key> selectedCertificates() const;
        GpgME::Key selectedCertificate() const;

    public Q_SLOTS:
        void setStringFilter( const QString & text );
        void setKeyFilter( const boost::shared_ptr<Kleo::KeyFilter> & filter );

    protected:
        void hideEvent( QHideEvent * );

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
        Q_PRIVATE_SLOT( d, void refresh() )
        Q_PRIVATE_SLOT( d, void slotRefreshed() )
    };

}
}

Q_DECLARE_OPERATORS_FOR_FLAGS( Kleo::Dialogs::CertificateSelectionDialog::Options )

#endif /* __KLEOPATRA_DIALOGS_CERTIFICATESELECTIONDIALOG_H__ */
