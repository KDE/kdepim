#include "signingcertificateselectiondialog.h"

#include "ui_signingcertificateselectionwidget.h"
#include "models/keycache.h"
#include "utils/formatting.h"

#include <QByteArray>
#include <QMap>

#include <boost/bind.hpp>
#include <cassert>

using namespace Kleo;

class SigningCertificateSelectionDialog::Private {
    friend class ::SigningCertificateSelectionDialog;
    SigningCertificateSelectionDialog * const q;
public:
    explicit Private( SigningCertificateSelectionDialog * qq );
    ~Private();
    std::vector<GpgME::Key> candidates( GpgME::Protocol prot ) const;
    void addCandidates( GpgME::Protocol prot, QComboBox* combo );

private:
    Ui::SigningCertificateSelectionWidget ui;
};


SigningCertificateSelectionDialog::Private::Private( SigningCertificateSelectionDialog * qq )
  : q( qq )
{
    q->setWindowTitle( i18n( "Select Signing Certificates" ) );
    QWidget* main = new QWidget( q );
    ui.setupUi( main );
    q->setMainWidget( main ); 
    addCandidates( GpgME::CMS, ui.cmsCombo );
    addCandidates( GpgME::OpenPGP, ui.pgpCombo );
    ui.rememberCO->setChecked( true );
}

SigningCertificateSelectionDialog::Private::~Private() {}



SigningCertificateSelectionDialog::SigningCertificateSelectionDialog( QWidget * parent, Qt::WFlags f )
  : KDialog( parent, f ), d( new Private( this ) )
{
}

SigningCertificateSelectionDialog::~SigningCertificateSelectionDialog() {}


void SigningCertificateSelectionDialog::setSelectedCertificates( const QMap<GpgME::Protocol, GpgME::Key>& certificates )
{
}

std::vector<GpgME::Key> SigningCertificateSelectionDialog::Private::candidates( GpgME::Protocol prot ) const
{
    assert( prot != GpgME::UnknownProtocol );
    std::vector<GpgME::Key> keys = KeyCache::instance()->keys();
    std::vector<GpgME::Key>::iterator end = keys.end();

    end = std::remove_if( keys.begin(), end, bind( &GpgME::Key::protocol, _1 ) != prot );
    end = std::remove_if( keys.begin(), end, !bind( &GpgME::Key::hasSecret, _1 ) );
    end = std::remove_if( keys.begin(), end, !bind( &GpgME::Key::canSign, _1 ) );
    keys.erase( end, keys.end() );
    return keys;
}

void SigningCertificateSelectionDialog::Private::addCandidates( GpgME::Protocol prot, QComboBox* combo )
{
    const std::vector<GpgME::Key> keys = candidates( prot );
    foreach ( const GpgME::Key& i, keys )
    {
        combo->addItem( Formatting::formatForComboBox( i ), 
                        QByteArray( i.keyID() ) );
    }
}


QMap<GpgME::Protocol, GpgME::Key> SigningCertificateSelectionDialog::selectedCertificates() const
{
    QMap<GpgME::Protocol, GpgME::Key> res;
    
    const QByteArray pgpid = d->ui.pgpCombo->itemData( d->ui.pgpCombo->currentIndex() ).toByteArray();
    res.insert( GpgME::OpenPGP, KeyCache::instance()->findByKeyIDOrFingerprint( pgpid.constData() ) );
    const QByteArray cmsid = d->ui.cmsCombo->itemData( d->ui.cmsCombo->currentIndex() ).toByteArray();
    res.insert( GpgME::CMS, KeyCache::instance()->findByKeyIDOrFingerprint( cmsid.constData() ) );
    return res;
}
