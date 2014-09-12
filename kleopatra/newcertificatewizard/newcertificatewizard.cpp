/* -*- mode: c++; c-basic-offset:4 -*-
    newcertificatewizard/newcertificatewizard.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "newcertificatewizard.h"

#include "ui_chooseprotocolpage.h"
#include "ui_enterdetailspage.h"
#include "ui_overviewpage.h"
#include "ui_keycreationpage.h"
#include "ui_resultpage.h"

#include "ui_advancedsettingsdialog.h"

#include <models/keycache.h>

#include <commands/exportsecretkeycommand.h>
#include <commands/exportopenpgpcertstoservercommand.h>
#include <commands/exportcertificatecommand.h>

#include <utils/formatting.h>
#include <utils/validation.h>
#include <utils/filedialog.h>

#include <kleo/stl_util.h>
#include <kleo/dn.h>
#include <kleo/oidmap.h>
#include <kleo/keygenerationjob.h>
#include <kleo/cryptobackendfactory.h>
#include <kleo/cryptobackend.h>

#include <gpgme++/global.h>
#include <gpgme++/keygenerationresult.h>
#include <gpgme.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <QDebug>
#include <QTemporaryDir>
#include <KMessageBox>
#include <QIcon>

#include <QRegExpValidator>
#include <QLineEdit>
#include <QMetaProperty>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QDesktopServices>

#include <boost/range.hpp>

#include <algorithm>
#include <KSharedConfig>
#include <KLocale>

using namespace Kleo;
using namespace Kleo::NewCertificateUi;
using namespace Kleo::Commands;
using namespace GpgME;
using namespace boost;

static const char RSA_KEYSIZES_ENTRY[] = "RSAKeySizes";
static const char DSA_KEYSIZES_ENTRY[] = "DSAKeySizes";
static const char ELG_KEYSIZES_ENTRY[] = "ELGKeySizes";

static const char RSA_KEYSIZE_LABELS_ENTRY[] = "RSAKeySizeLabels";
static const char DSA_KEYSIZE_LABELS_ENTRY[] = "DSAKeySizeLabels";
static const char ELG_KEYSIZE_LABELS_ENTRY[] = "ELGKeySizeLabels";

static const char PGP_KEY_TYPE_ENTRY[] = "PGPKeyType";
static const char CMS_KEY_TYPE_ENTRY[] = "CMSKeyType";

static void set_tab_order( const QList<QWidget*> & wl ) {
    kdtools::for_each_adjacent_pair( wl, &QWidget::setTabOrder );
}

enum KeyAlgo { RSA, DSA, ELG };

static bool is_algo( gpgme_pubkey_algo_t algo, KeyAlgo what ) {
    switch ( algo ) {
    case GPGME_PK_RSA:
    case GPGME_PK_RSA_E:
    case GPGME_PK_RSA_S:
        return what == RSA;
    case GPGME_PK_ELG_E:
    case GPGME_PK_ELG:
        return what == ELG;
    case GPGME_PK_DSA:
        return what == DSA;
    default:
        break;
    }
    return false;
}

static bool is_rsa( unsigned int algo ) {
    return is_algo( static_cast<gpgme_pubkey_algo_t>( algo ), RSA );
}

static bool is_dsa( unsigned int algo ) {
    return is_algo( static_cast<gpgme_pubkey_algo_t>( algo ), DSA );
}

static bool is_elg( unsigned int algo ) {
    return is_algo( static_cast<gpgme_pubkey_algo_t>( algo ), ELG );
}

static void force_set_checked( QAbstractButton * b, bool on ) {
    // work around Qt bug (tested: 4.1.4, 4.2.3, 4.3.4)
    const bool autoExclusive = b->autoExclusive();
    b->setAutoExclusive( false );
    b->setChecked( b->isEnabled() && on );
    b->setAutoExclusive( autoExclusive );
}

static void set_keysize( QComboBox * cb, unsigned int strength ) {
    if ( !cb )
        return;
    const int idx = cb->findData( static_cast<int>( strength ) );
    if ( idx < 0 )
        qWarning() << "keysize " << strength << " not allowed";
    cb->setCurrentIndex( idx );
}

static unsigned int get_keysize( const QComboBox * cb ) {
    if ( !cb )
        return 0;
    const int idx = cb->currentIndex();
    if ( idx < 0 )
        return 0;
    return cb->itemData( idx ).toInt();
}

namespace Kleo {
namespace NewCertificateUi {
    class WizardPage : public QWizardPage {
        Q_OBJECT
    protected:
        explicit WizardPage( QWidget * parent=0 )
            : QWizardPage( parent ) {}

        NewCertificateWizard * wizard() const {
            assert( static_cast<NewCertificateWizard*>( QWizardPage::wizard() ) == qobject_cast<NewCertificateWizard*>( QWizardPage::wizard() ) );
            return static_cast<NewCertificateWizard*>( QWizardPage::wizard() );
        }

        QAbstractButton * button( QWizard::WizardButton button ) const {
            return QWizardPage::wizard() ? QWizardPage::wizard()->button( button ) : 0 ;
        }

        bool isButtonVisible( QWizard::WizardButton button ) const {
            if ( const QAbstractButton * const b = this->button( button ) )
                return b->isVisible();
            else
                return false;
        }

        QDir tmpDir() const;

    protected Q_SLOTS:
        void setButtonVisible( QWizard::WizardButton button, bool visible ) {
            if ( QAbstractButton * const b = this->button( button ) )
                b->setVisible( visible );
        }

    protected:
#define FIELD(type, name) type name() const { return field( QLatin1String(#name) ).value<type>(); }
        FIELD( bool, pgp )
        FIELD( bool, signingAllowed )
        FIELD( bool, encryptionAllowed )
        FIELD( bool, certificationAllowed )
        FIELD( bool, authenticationAllowed )

        FIELD( QString, name )
        FIELD( QString, email )
        FIELD( QString, comment )
        FIELD( QString, dn )

        FIELD( int, keyType )
        FIELD( int, keyStrength )

        FIELD( int, subkeyType )
        FIELD( int, subkeyStrength )

        FIELD( QDate, expiryDate )

        FIELD( QStringList, additionalUserIDs )
        FIELD( QStringList, additionalEMailAddresses )
        FIELD( QStringList, dnsNames )
        FIELD( QStringList, uris )

        FIELD( QString, url )
        FIELD( QString, error )
        FIELD( QString, result )
        FIELD( QString, fingerprint )
#undef FIELD
    };
} // namespace NewCertificateUi
} // namespace Kleo

using namespace Kleo::NewCertificateUi;

namespace {

    class AdvancedSettingsDialog : public QDialog {
        Q_OBJECT
        Q_PROPERTY( QStringList additionalUserIDs READ additionalUserIDs WRITE setAdditionalUserIDs )
        Q_PROPERTY( QStringList additionalEMailAddresses READ additionalEMailAddresses WRITE setAdditionalEMailAddresses )
        Q_PROPERTY( QStringList dnsNames READ dnsNames WRITE setDnsNames )
        Q_PROPERTY( QStringList uris READ uris WRITE setUris )
        Q_PROPERTY( uint keyStrength READ keyStrength WRITE setKeyStrength )
        Q_PROPERTY( uint keyType READ keyType WRITE setKeyType )
        Q_PROPERTY( uint subkeyStrength READ subkeyStrength WRITE setSubkeyStrength )
        Q_PROPERTY( uint subkeyType READ subkeyType WRITE setSubkeyType )
        Q_PROPERTY( bool signingAllowed READ signingAllowed WRITE setSigningAllowed )
        Q_PROPERTY( bool encryptionAllowed READ encryptionAllowed WRITE setEncryptionAllowed )
        Q_PROPERTY( bool certificationAllowed READ certificationAllowed WRITE setCertificationAllowed )
        Q_PROPERTY( bool authenticationAllowed READ authenticationAllowed WRITE setAuthenticationAllowed )
        Q_PROPERTY( QDate expiryDate READ expiryDate WRITE setExpiryDate )
    public:
        explicit AdvancedSettingsDialog( QWidget * parent=0 )
            : QDialog( parent ),
              protocol( UnknownProtocol ),
              pgpDefaultAlgorithm( GPGME_PK_ELG_E ),
              cmsDefaultAlgorithm( GPGME_PK_RSA ),
              keyTypeImmutable( false ),
              ui()
        {
            ui.setupUi( this );
            const QDate today = QDate::currentDate();
            ui.expiryDE->setMinimumDate( today );
            ui.expiryDE->setDate( today.addYears( 2 ) );
            ui.emailLW->setDefaultValue( i18n("new email") );
            ui.dnsLW->setDefaultValue( i18n("new dns name") );
            ui.uriLW->setDefaultValue( i18n("new uri") );

            fillKeySizeComboBoxen();
        }

        void setProtocol( GpgME::Protocol proto ) {
            if ( protocol == proto )
                return;
            protocol = proto;
            loadDefaultKeyType();
        }

        void setAdditionalUserIDs( const QStringList & items ) { ui.uidLW->setItems( items ); }
        QStringList additionalUserIDs() const { return ui.uidLW->items();   }

        void setAdditionalEMailAddresses( const QStringList & items ) { ui.emailLW->setItems( items ); }
        QStringList additionalEMailAddresses() const { return ui.emailLW->items(); }

        void setDnsNames( const QStringList & items ) { ui.dnsLW->setItems( items ); }
        QStringList dnsNames() const { return ui.dnsLW->items();   }

        void setUris( const QStringList & items ) { ui.uriLW->setItems( items ); }
        QStringList uris() const { return ui.uriLW->items();   }


        void setKeyStrength( unsigned int strength ) {
            set_keysize( ui.rsaKeyStrengthCB, strength );
            set_keysize( ui.dsaKeyStrengthCB, strength );
        }
        unsigned int keyStrength() const {
            return
                ui.dsaRB->isChecked() ? get_keysize( ui.dsaKeyStrengthCB ) :
                ui.rsaRB->isChecked() ? get_keysize( ui.rsaKeyStrengthCB ) : 0 ;
        }

        void setKeyType( unsigned int algo ) {
            QRadioButton * const rb =
                is_rsa( algo ) ? ui.rsaRB :
                is_dsa( algo ) ? ui.dsaRB : 0 ;
            if ( rb )
                rb->setChecked( true );
        }
        unsigned int keyType() const {
            return
                ui.dsaRB->isChecked() ? GPGME_PK_DSA :
                ui.rsaRB->isChecked() ? GPGME_PK_RSA :
                0 ;
        }

        void setSubkeyType( unsigned int algo ) { ui.elgCB->setChecked( is_elg( algo ) ); }
        unsigned int subkeyType() const { return ui.elgCB->isChecked() ? GPGME_PK_ELG_E : 0 ; }

        void setSubkeyStrength( unsigned int strength ) {
            set_keysize( ui.elgKeyStrengthCB, strength );
        }
        unsigned int subkeyStrength() const {
            return get_keysize( ui.elgKeyStrengthCB );
        }

        void setSigningAllowed( bool on ) { ui.signingCB->setChecked( on ); }
        bool signingAllowed() const { return ui.signingCB->isChecked(); }

        void setEncryptionAllowed( bool on ) { ui.encryptionCB->setChecked( on ); }
        bool encryptionAllowed() const { return ui.encryptionCB->isChecked(); }

        void setCertificationAllowed( bool on ) { ui.certificationCB->setChecked( on ); }
        bool certificationAllowed() const { return ui.certificationCB->isChecked(); }

        void setAuthenticationAllowed( bool on ) { ui.authenticationCB->setChecked( on ); }
        bool authenticationAllowed() const { return ui.authenticationCB->isChecked(); }

        void setExpiryDate( const QDate & date ) { if ( date.isValid() ) ui.expiryDE->setDate( date ); else ui.expiryCB->setChecked( false ); }
        QDate expiryDate() const { return ui.expiryCB->isChecked() ? ui.expiryDE->date() : QDate() ; }

    Q_SIGNALS:
        void changed();

    private Q_SLOTS:
        void slotKeyMaterialSelectionChanged() {
            const unsigned int algo = keyType();
            const unsigned int sk_algo = subkeyType();
            if ( protocol == OpenPGP ) {
                if ( !keyTypeImmutable ) {
                    ui.elgCB->setEnabled( is_dsa( algo ) );
                    if ( sender() == ui.dsaRB || sender() == ui.rsaRB )
                        ui.elgCB->setChecked( is_dsa( algo ) );
                }
                if ( is_rsa( algo ) ) {
                    ui.encryptionCB->setEnabled( true );
                    ui.encryptionCB->setChecked( true );
                    ui.signingCB->setEnabled( true );
                    ui.signingCB->setChecked( true );
                    ui.authenticationCB->setEnabled( true );
                } else if ( is_dsa( algo ) ) {
                    ui.encryptionCB->setEnabled( false );
                    if ( is_elg( sk_algo ) )
                        ui.encryptionCB->setChecked( true );
                    else
                        ui.encryptionCB->setChecked( false );
                }
            } else {
                //assert( is_rsa( keyType() ) ); // it can happen through misconfiguration by the admin that no key type is selectable at all
            }
        }

        void slotSigningAllowedToggled( bool on ) {
            if ( !on && protocol == CMS && !encryptionAllowed() )
                setEncryptionAllowed( true );
        }
        void slotEncryptionAllowedToggled( bool on ) {
            if ( !on && protocol == CMS && !signingAllowed() )
                setSigningAllowed( true );
        }

    private:
        void fillKeySizeComboBoxen();
        void loadDefaultKeyType();
        void updateWidgetVisibility();

    private:
        GpgME::Protocol protocol;
        unsigned int pgpDefaultAlgorithm;
        unsigned int cmsDefaultAlgorithm;
        bool keyTypeImmutable;
        Ui_AdvancedSettingsDialog ui;
    };

    class ChooseProtocolPage : public WizardPage {
        Q_OBJECT
    public:
        explicit ChooseProtocolPage( QWidget * p=0 )
            : WizardPage( p ),
              initialized( false ),
              ui()
        {
            ui.setupUi( this );
            registerField( QLatin1String("pgp"), ui.pgpCLB );
        }

        void setProtocol( Protocol proto ) {
            if ( proto == OpenPGP )
                ui.pgpCLB->setChecked( true );
            else if ( proto == CMS )
                ui.x509CLB->setChecked( true );
            else {
                force_set_checked( ui.pgpCLB,  false );
                force_set_checked( ui.x509CLB, false );
            }
        }

        Protocol protocol() const {
            return
                ui.pgpCLB->isChecked()  ? OpenPGP :
                ui.x509CLB->isChecked() ? CMS : UnknownProtocol ;
        }

        /* reimp */ void initializePage() {
            if ( !initialized ) {
                connect( ui.pgpCLB,  SIGNAL(clicked()), wizard(), SLOT(next()), Qt::QueuedConnection );
                connect( ui.x509CLB, SIGNAL(clicked()), wizard(), SLOT(next()), Qt::QueuedConnection );
            }
            initialized = true;
        }

        /* reimp */ bool isComplete() const {
            return protocol() != UnknownProtocol ;
        }

    private:
        bool initialized : 1;
        Ui_ChooseProtocolPage ui;
    };

    struct Line {
        QString attr;
        QString label;
        QString regex;
        QLineEdit * edit;
    };

    class EnterDetailsPage : public WizardPage {
        Q_OBJECT
    public:
        explicit EnterDetailsPage( QWidget * p=0 )
            : WizardPage( p ), dialog( this ), ui()
        {
            ui.setupUi( this );

            // set errorLB to have a fixed height of two lines:
            ui.errorLB->setText( QLatin1String("2<br>1") );
            ui.errorLB->setFixedHeight( ui.errorLB->minimumSizeHint().height() );
            ui.errorLB->clear();

            connect( ui.resultLE, SIGNAL(textChanged(QString)),
                     SIGNAL(completeChanged()) );
            // The email doesn't necessarily show up in ui.resultLE:
            connect( ui.emailLE, SIGNAL(textChanged(QString)),
                     SIGNAL(completeChanged()) );
            connect( ui.addEmailToDnCB, SIGNAL(toggled(bool)),
                     SLOT(slotUpdateResultLabel()) );
            registerDialogPropertiesAsFields();
            registerField( QLatin1String("dn"), ui.resultLE );
            registerField( QLatin1String("name"), ui.nameLE );
            registerField( QLatin1String("email"), ui.emailLE );
            registerField( QLatin1String("comment"), ui.commentLE );
            updateForm();
        }

        /* reimp */ bool isComplete() const;
        /* reimp */ void initializePage() {
            updateForm();
            dialog.setProtocol( pgp() ? OpenPGP : CMS );
        }
        /* reimp */ void cleanupPage() {
            saveValues();
        }

    private:
        void updateForm();
        void clearForm();
        void saveValues();
        void registerDialogPropertiesAsFields();

    private:
        QString pgpUserID() const;
        QString cmsDN() const;

    private Q_SLOTS:
        void slotAdvancedSettingsClicked();
        void slotUpdateResultLabel() {
            ui.resultLE->setText( pgp() ? pgpUserID() : cmsDN() );
        }

    private:
        QVector<Line> lineList;
        QList<QWidget*> dynamicWidgets;
        QMap<QString,QString> savedValues;
        AdvancedSettingsDialog dialog;
        Ui_EnterDetailsPage ui;
    };

    class OverviewPage : public WizardPage {
        Q_OBJECT
    public:
        explicit OverviewPage( QWidget * p=0 )
            : WizardPage( p ), ui()
        {
            ui.setupUi( this );
            setCommitPage( true );
            setButtonText( QWizard::CommitButton, i18nc("@action", "Create Key") );
        }

        /* reimp */ void initializePage() {
            slotShowDetails();
        }

    private Q_SLOTS:
        void slotShowDetails() {
            ui.textBrowser->setHtml( i18nFormatGnupgKeyParms( ui.showAllDetailsCB->isChecked() ) );
        }

    private:
        QStringList i18nKeyUsages() const;
        QStringList i18nSubkeyUsages() const;
        QStringList i18nCombinedKeyUsages() const;
        QString i18nFormatGnupgKeyParms( bool details ) const;

    private:
        Ui_OverviewPage ui;
    };

    class KeyCreationPage : public WizardPage {
        Q_OBJECT
    public:
        explicit KeyCreationPage( QWidget * p=0 )
            : WizardPage( p ),
              ui()
        {
            ui.setupUi( this );
        }

        /* reimp */ bool isComplete() const {
            return !job;
        }

        /* reimp */ void initializePage() {
            startJob();
        }

    private:
        void startJob() {
            const CryptoBackend::Protocol * const proto = CryptoBackendFactory::instance()->protocol( pgp() ? OpenPGP : CMS );
            if ( !proto )
                return;
            KeyGenerationJob * const j = proto->keyGenerationJob();
            if ( !j )
                return;
            connect( j, SIGNAL(result(GpgME::KeyGenerationResult,QByteArray,QString)),
                     this, SLOT(slotResult(GpgME::KeyGenerationResult,QByteArray,QString)) );
            if ( const Error err = j->start( createGnupgKeyParms() ) )
                setField( QLatin1String("error"), i18n("Could not start certificate creation: %1",
                                        QString::fromLocal8Bit( err.asString() ) ) );
            else
                job = j;
        }
        QStringList keyUsages() const;
        QStringList subkeyUsages() const;
        QString createGnupgKeyParms() const;

    private Q_SLOTS:
        void slotResult( const GpgME::KeyGenerationResult & result, const QByteArray & request, const QString & auditLog )
        {
            Q_UNUSED( auditLog );
            if ( result.error().code() ) {
                setField( QLatin1String("error"), result.error().isCanceled()
                          ? i18n("Operation canceled.")
                          : i18n("Could not create certificate: %1",
                                 QString::fromLocal8Bit( result.error().asString() ) ) );
                setField( QLatin1String("url"), QString() );
                setField( QLatin1String("result"), QString() );
            } else if ( pgp() ) {
                setField( QLatin1String("error"), QString() );
                setField( QLatin1String("url"), QString() );
                setField( QLatin1String("result"), i18n("Certificate created successfully.\n"
                                         "Fingerprint: %1", QLatin1String(result.fingerprint()) ) );
            } else {
                QFile file( tmpDir().absoluteFilePath( QLatin1String("request.p10") ) );

                if ( !file.open( QIODevice::WriteOnly ) ) {
                    setField( QLatin1String("error"), i18n("Could not write output file %1: %2",
                                            file.fileName(), file.errorString() ) );
                    setField( QLatin1String("url"), QString() );
                    setField( QLatin1String("result"), QString() );
                } else {
                    file.write( request );
                    setField( QLatin1String("error"), QString() );
                    setField( QLatin1String("url"), QUrl::fromLocalFile( file.fileName() ).toString() );
                    setField( QLatin1String("result"), i18n("Certificate created successfully.") );
                }
            }
            setField( QLatin1String("fingerprint"), QString::fromLatin1( result.fingerprint() ) );
            job = 0;
            emit completeChanged();
            QMetaObject::invokeMethod( wizard(), "next", Qt::QueuedConnection );
        }

    private:
        QPointer<KeyGenerationJob> job;
        Ui_KeyCreationPage ui;
    };

    class ResultPage : public WizardPage {
        Q_OBJECT
    public:
        explicit ResultPage( QWidget * p=0 )
            : WizardPage( p ),
              initialized( false ),
              successfullyCreatedSigningCertificate( false ),
              successfullyCreatedEncryptionCertificate( false ),
              ui()
        {
            ui.setupUi( this );
            ui.dragQueen->setPixmap( QIcon::fromTheme( QLatin1String("kleopatra") ).pixmap( 64, 64 ) );
            registerField( QLatin1String("error"),  ui.errorTB,   "plainText" );
            registerField( QLatin1String("result"), ui.resultTB,  "plainText" );
            registerField( QLatin1String("url"),    ui.dragQueen, "url" );
            // hidden field, since QWizard can't deal with non-widget-backed fields...
            QLineEdit * le = new QLineEdit( this );
            le->hide();
            registerField( QLatin1String("fingerprint"), le );
        }

        /* reimp */ void initializePage() {
            const bool error = isError();

            if ( error ) {
                setTitle( i18nc("@title","Key Creation Failed") );
                setSubTitle( i18n("Key pair creation failed. Please find details about the failure below.") );
            } else {
                setTitle( i18nc("@title","Key Pair Successfully Created") );
                setSubTitle( i18n("Your new key pair was created successfully. Please find details on the result and some suggested next steps below.") );
            }

            ui.resultTB                 ->setVisible( !error );
            ui.errorTB                  ->setVisible(  error );
            ui.dragQueen                ->setVisible( !error && !pgp() );
            ui.restartWizardPB          ->setVisible(  error );
            ui.nextStepsGB              ->setVisible( !error );
            ui.saveRequestToFilePB      ->setVisible( !pgp() );
            ui.makeBackupPB             ->setVisible(  pgp() );
            ui.createRevocationRequestPB->setVisible(  pgp() && false ); // not implemented

#ifdef KDEPIM_MOBILE_UI
            ui.sendCertificateByEMailPB ->setVisible(  false );
            ui.sendRequestByEMailPB     ->setVisible(  false );
            ui.uploadToKeyserverPB      ->setVisible(  false );
#else
            ui.sendCertificateByEMailPB ->setVisible(  pgp() );
            ui.sendRequestByEMailPB     ->setVisible( !pgp() );
            ui.uploadToKeyserverPB      ->setVisible(  pgp() );
#endif

            if ( !error && !pgp() ) {
                if ( signingAllowed() && !encryptionAllowed() )
                    successfullyCreatedSigningCertificate = true;
                else if ( !signingAllowed() && encryptionAllowed() )
                    successfullyCreatedEncryptionCertificate = true;
                else
                    successfullyCreatedEncryptionCertificate = successfullyCreatedSigningCertificate = true;
            }

            ui.createSigningCertificatePB->setVisible( successfullyCreatedEncryptionCertificate && !successfullyCreatedSigningCertificate );
            ui.createEncryptionCertificatePB->setVisible( successfullyCreatedSigningCertificate && !successfullyCreatedEncryptionCertificate );

            setButtonVisible( QWizard::CancelButton, error );

            if ( !initialized )
                connect( ui.restartWizardPB, SIGNAL(clicked()),
                         wizard(), SLOT(restart()) );
            initialized = true;
        }

        /* reimp */ void cleanupPage() {
            setButtonVisible( QWizard::CancelButton, true );
        }

        bool isError() const {
            return !ui.errorTB->toPlainText().isEmpty();
        }

        /* reimp */ bool isComplete() const {
            return !isError();
        }

    private:
        Key key() const {
            return KeyCache::instance()->findByFingerprint( fingerprint().toLatin1().constData() );
        }

    private Q_SLOTS:
        void slotSaveRequestToFile() {
            QString fileName = FileDialog::getSaveFileName( this, i18nc("@title", "Save Request"),
                                                            QLatin1String("imp"), i18n("PKCS#10 Requests (*.p10)") );
            if ( fileName.isEmpty() )
                return;
            if ( !fileName.endsWith( QLatin1String( ".p10" ), Qt::CaseInsensitive ) )
                fileName += QLatin1String(".p10");
            QFile src( QUrl( url() ).toLocalFile() );
            if ( !src.copy( fileName ) )
                KMessageBox::error( this,
                                    xi18nc("@info",
                                          "Could not copy temporary file <filename>%1</filename> "
                                          "to file <filename>%2</filename>: <message>%3</message>",
                                          src.fileName(), fileName, src.errorString() ),
                                    i18nc("@title", "Error Saving Request") );
            else
                KMessageBox::information( this,
                                          xi18nc("@info",
                                                "<para>Successfully wrote request to <filename>%1</filename>.</para>"
                                                "<para>You should now send the request to the Certification Authority (CA).</para>",
                                                fileName ),
                                          i18nc("@title", "Request Saved" ) );
        }

        void slotSendRequestByEMail() {
            if ( pgp() )
                return;
            const KConfigGroup config( KSharedConfig::openConfig(), "CertificateCreationWizard" );
            invokeMailer( config.readEntry( "CAEmailAddress" ), // to
                          i18n("Please process this certificate."), // subject
                          i18n("Please process this certificate and inform the sender about the location to fetch the resulting certificate.\n\nThanks,\n"), // body
                          QUrl( url() ).toLocalFile() ); // attachment
        }

        void slotSendCertificateByEMail() {
            if ( !pgp() || exportCertificateCommand )
                return;
            ExportCertificateCommand * cmd = new ExportCertificateCommand( key() );
            connect(cmd, &ExportCertificateCommand::finished, this, &ResultPage::slotSendCertificateByEMailContinuation);
            cmd->setOpenPGPFileName( tmpDir().absoluteFilePath( fingerprint() + QLatin1String(".asc") ) );
            cmd->start();
            exportCertificateCommand = cmd;
        }

        void slotSendCertificateByEMailContinuation() {
            if ( !exportCertificateCommand )
                return;
            // ### better error handling?
            const QString fileName = exportCertificateCommand->openPGPFileName();
            qDebug() << "fileName" << fileName;
            exportCertificateCommand = 0;
            if ( fileName.isEmpty() )
                return;
            invokeMailer( QString(), // to
                          i18n("My new OpenPGP certificate"), // subject
                          i18n("Please find attached my new OpenPGP certificate."), // body
                          fileName );
        }

        QByteArray ol_quote( QByteArray str ) {
#ifdef Q_OS_WIN
            return "\"\"" + str.replace( '"', "\\\"" ) + "\"\"";
            //return '"' + str.replace( '"', "\\\"" ) + '"';
#else
            return str;
#endif
        }

        void invokeMailer( const QString & to, const QString & subject, QString body, const QString & attachment ) {
            qDebug() << "to:" << to << "subject:" << subject
                     << "body:" << body << "attachment:" << attachment;
            // KToolInvocation::invokeMailer is broken on Windows, and openUrl works fine on Unix, too.

            // RFC 2368 says body's linebreaks need to be encoded as
            // "%0D%0A", so normalize body to CRLF:
            body.replace( QLatin1Char( '\n' ), QLatin1String( "\r\n" ) ).remove( QLatin1String( "\r\r" ) );

            QByteArray encoded = "mailto:?to=" + QUrl::toPercentEncoding( to )
                + "&subject=" + QUrl::toPercentEncoding( subject )
                + "&body=" + QUrl::toPercentEncoding( body ) ;
            if ( !attachment.isEmpty() )
                encoded += "&attach=" + ol_quote( QUrl::toPercentEncoding( QFileInfo( attachment ).absoluteFilePath() ) );
            qDebug() << "openUrl" << QUrl::fromEncoded( encoded );
            QDesktopServices::openUrl( QUrl::fromEncoded( encoded ) );
            KMessageBox::information( this,
                                      xi18nc("@info",
                                            "<para><application>Kleopatra</application> tried to send a mail via your default mail client.</para>"
                                            "<para>Some mail clients are known not to support attachments when invoked this way.</para>"
                                            "<para>If your mail client does not have an attachment, then drag the <application>Kleopatra</application> icon and drop it on the message compose window of your mail client.</para>"
                                            "<para>If that does not work, either, save the request to a file, and then attach that.</para>"),
                                      i18nc("@title", "Sending Mail"),
                                      QLatin1String("newcertificatewizard-mailto-troubles") );
        }

        void slotUploadCertificateToDirectoryServer() {
            if ( pgp() )
                ( new ExportOpenPGPCertsToServerCommand( key() ) )->start();
        }

        void slotBackupCertificate() {
            if ( pgp() )
                ( new ExportSecretKeyCommand( key() ) )->start();
        }

        void slotCreateRevocationRequest() {

        }

        void slotCreateSigningCertificate() {
            if ( successfullyCreatedSigningCertificate )
                return;
            toggleSignEncryptAndRestart();
        }

        void slotCreateEncryptionCertificate() {
            if ( successfullyCreatedEncryptionCertificate )
                return;
            toggleSignEncryptAndRestart();
        }

    private:
        void toggleSignEncryptAndRestart() {
            if ( !wizard() )
                return;
            if ( KMessageBox::warningContinueCancel(
                     this,
                     i18nc("@info",
                           "This operation will delete the certification request. "
                           "Please make sure that you have sent or saved it before proceeding." ),
                     i18nc("@title", "Certification Request About To Be Deleted") ) != KMessageBox::Continue )
                return;
            const bool sign = signingAllowed();
            const bool encr = encryptionAllowed();
            setField( QLatin1String("signingAllowed"),    !sign );
            setField( QLatin1String("encryptionAllowed"), !encr );
            // restart and skip to Overview Page:
            wizard()->restart();
            for ( int i = wizard()->currentId() ; i < NewCertificateWizard::OverviewPageId ; ++i )
                wizard()->next();
        }

    private:
        bool initialized : 1;
        bool successfullyCreatedSigningCertificate : 1;
        bool successfullyCreatedEncryptionCertificate : 1;
        QPointer<ExportCertificateCommand> exportCertificateCommand;
        Ui_ResultPage ui;
    };
}

class NewCertificateWizard::Private {
    friend class ::Kleo::NewCertificateWizard;
    friend class ::Kleo::NewCertificateUi::WizardPage;
    NewCertificateWizard * const q;
public:
    explicit Private( NewCertificateWizard * qq )
        : q( qq ),
          tmp( QDir::temp().absoluteFilePath( QLatin1String("kleo-") ) ),
          ui( q )
    {
        q->setWindowTitle( i18nc("@title", "Certificate Creation Wizard") );
    }

private:
    QTemporaryDir tmp;
    struct Ui {
        ChooseProtocolPage chooseProtocolPage;
        EnterDetailsPage enterDetailsPage;
        OverviewPage overviewPage;
        KeyCreationPage keyCreationPage;
        ResultPage resultPage;

        explicit Ui( NewCertificateWizard * q )
            : chooseProtocolPage( q ),
              enterDetailsPage( q ),
              overviewPage( q ),
              keyCreationPage( q ),
              resultPage( q )
        {
            KDAB_SET_OBJECT_NAME( chooseProtocolPage );
            KDAB_SET_OBJECT_NAME( enterDetailsPage );
            KDAB_SET_OBJECT_NAME( overviewPage );
            KDAB_SET_OBJECT_NAME( keyCreationPage );
            KDAB_SET_OBJECT_NAME( resultPage );

            q->setOptions( DisabledBackButtonOnLastPage );

            q->setPage( ChooseProtocolPageId, &chooseProtocolPage );
            q->setPage( EnterDetailsPageId,   &enterDetailsPage   );
            q->setPage( OverviewPageId,       &overviewPage       );
            q->setPage( KeyCreationPageId,    &keyCreationPage    );
            q->setPage( ResultPageId,         &resultPage         );

            q->setStartId( ChooseProtocolPageId );
        }

    } ui;

};

NewCertificateWizard::NewCertificateWizard( QWidget * p )
    : QWizard( p ), d( new Private( this ) )
{

}

NewCertificateWizard::~NewCertificateWizard() {}

void NewCertificateWizard::setProtocol( Protocol proto ) {
    d->ui.chooseProtocolPage.setProtocol( proto );
    setStartId( proto == UnknownProtocol ? ChooseProtocolPageId : EnterDetailsPageId );
}

Protocol NewCertificateWizard::protocol() const {
    return d->ui.chooseProtocolPage.protocol();
}

static QString pgpLabel( const QString & attr ) {
    if ( attr == QLatin1String("NAME") )
        return i18n("Name");
    if ( attr == QLatin1String("COMMENT") )
        return i18n("Comment");
    if ( attr == QLatin1String("EMAIL") )
        return i18n("EMail");
    return QString();
}

static QString attributeLabel( const QString & attr, bool pgp ) {
  if ( attr.isEmpty() )
    return QString();
  const QString label = pgp ? pgpLabel( attr ) : Kleo::DNAttributeMapper::instance()->name2label( attr ) ;
  if ( !label.isEmpty() )
      if ( pgp )
          return label;
      else
          return i18nc("Format string for the labels in the \"Your Personal Data\" page",
                       "%1 (%2)", label, attr );
  else
    return attr;
}

#if 0
//Not used anywhere
static QString attributeLabelWithColor( const QString & attr, bool pgp ) {
    const QString result = attributeLabel( attr, pgp );
    if ( result.isEmpty() )
        return QString();
    else
        return result + ':';
}
#endif

static QString attributeFromKey( QString key ) {
  return key.remove( QLatin1Char('!') );
}

static const char * oidForAttributeName( const QString & attr ) {
  QByteArray attrUtf8 = attr.toUtf8();
  for ( unsigned int i = 0 ; i < numOidMaps ; ++i )
    if ( qstricmp( attrUtf8, oidmap[i].name ) == 0 )
      return oidmap[i].oid;
  return 0;
}

QDir WizardPage::tmpDir() const {
    return wizard() ? QDir( wizard()->d->tmp.path() ) : QDir::home() ;
}

void EnterDetailsPage::registerDialogPropertiesAsFields() {

    const QMetaObject * const mo = dialog.metaObject();
    for ( unsigned int i = mo->propertyOffset(), end = i + mo->propertyCount() ; i != end ; ++i ) {
        const QMetaProperty mp = mo->property( i );
        if ( mp.isValid() )
            registerField( QLatin1String(mp.name()), &dialog, mp.name(), SIGNAL(accepted()) );
    }

}

void EnterDetailsPage::saveValues() {
    Q_FOREACH( const Line & line, lineList )
        savedValues[ attributeFromKey( line.attr ) ] = line.edit->text().trimmed();
}

void EnterDetailsPage::clearForm() {
    qDeleteAll( dynamicWidgets );
    dynamicWidgets.clear();
    lineList.clear();

    ui.nameLE->hide();
    ui.nameLE->clear();
    ui.nameLB->hide();
    ui.nameRequiredLB->hide();

    ui.emailLE->hide();
    ui.emailLE->clear();
    ui.emailLB->hide();
    ui.emailRequiredLB->hide();

    ui.commentLE->hide();
    ui.commentLE->clear();
    ui.commentLB->hide();
    ui.commentRequiredLB->hide();

    ui.addEmailToDnCB->hide();
}

static int row_index_of( QWidget * w, QGridLayout * l ) {
    const int idx = l->indexOf( w );
    int r, c, rs, cs;
    l->getItemPosition( idx, &r, &c, &rs, &cs );
    return r;
}

static QLineEdit * adjust_row( QGridLayout * l, int row, const QString & label, const QString & preset, QValidator * validator, bool readonly, bool required ) {
    assert( l );
    assert( row >= 0 );
    assert( row < l->rowCount() );

    QLabel * lb = qobject_cast<QLabel*>( l->itemAtPosition( row, 0 )->widget() );
    assert( lb );
    QLineEdit * le = qobject_cast<QLineEdit*>( l->itemAtPosition( row, 1 )->widget() );
    assert( le );
    lb->setBuddy( le ); // For better accessibility
    QLabel * reqLB = qobject_cast<QLabel*>( l->itemAtPosition( row, 2 )->widget() );
    assert( reqLB );

    lb->setText( i18nc("interpunctation for labels", "%1:", label ) );
    le->setText( preset );
    reqLB->setText( required ? i18n("(required)") : i18n("(optional)") );
    delete le->validator();
    if ( validator ) {
        if ( !validator->parent() )
            validator->setParent( le );
        le->setValidator( validator );
    }

    le->setReadOnly( readonly && le->hasAcceptableInput() );

    lb->show();
    le->show();
    reqLB->show();

    return le;
}

static int add_row( QGridLayout * l, QList<QWidget*> * wl ) {
    assert( l );
    assert( wl );
    const int row = l->rowCount();
    QWidget *w1, *w2, *w3;
    l->addWidget( w1 = new QLabel( l->parentWidget() ),    row, 0 );
    l->addWidget( w2 = new QLineEdit( l->parentWidget() ), row, 1 );
    l->addWidget( w3 = new QLabel( l->parentWidget() ),    row, 2 );
    wl->push_back( w1 );
    wl->push_back( w2 );
    wl->push_back( w3 );
    return row;
}

void EnterDetailsPage::updateForm() {

    clearForm();

    const KConfigGroup config( KSharedConfig::openConfig(), "CertificateCreationWizard" );

    QStringList attrOrder = config.readEntry( pgp() ? "OpenPGPAttributeOrder" : "DNAttributeOrder", QStringList() );
    if ( attrOrder.empty() ) {
        if ( pgp() )
            attrOrder << QLatin1String("NAME!") << QLatin1String("EMAIL!") << QLatin1String("COMMENT");
        else
            attrOrder << QLatin1String("CN!") << QLatin1String("L") << QLatin1String("OU") << QLatin1String("O!") << QLatin1String("C!") << QLatin1String("EMAIL!");
    }

    QList<QWidget*> widgets;
    widgets.push_back( ui.nameLE );
    widgets.push_back( ui.emailLE );
    widgets.push_back( ui.commentLE );

    QMap<int,Line> lines;

    Q_FOREACH( const QString & rawKey, attrOrder ) {
        const QString key = rawKey.trimmed().toUpper();
        const QString attr = attributeFromKey( key );
        if ( attr.isEmpty() )
            continue;
        const QString preset = savedValues.value( attr, config.readEntry( attr, QString() ) );
        const bool required = key.endsWith( QLatin1Char('!') );
        const bool readonly = config.isEntryImmutable( attr );
        const QString label = config.readEntry( attr + QLatin1String("_label"),
                                                attributeLabel( attr, pgp() ) );
        const QString regex = config.readEntry( attr + QLatin1String("_regex") );

        int row;
        bool known = true;
        QValidator * validator = 0;
        if ( attr == QLatin1String("EMAIL") ) {
            row = row_index_of( ui.emailLE, ui.gridLayout );
            validator = regex.isEmpty() ? Validation::email() : Validation::email( QRegExp( regex ) ) ;
            if ( !pgp() )
                ui.addEmailToDnCB->show();
        } else if ( attr == QLatin1String("NAME") || attr == QLatin1String("CN") ) {
            if ( ( pgp() && attr == QLatin1String("CN") ) || ( !pgp() && attr == QLatin1String("NAME") ) )
                continue;
            if ( pgp() )
                validator = regex.isEmpty() ? Validation::pgpName() : Validation::pgpName( QRegExp( regex ) ) ;
            row = row_index_of( ui.nameLE, ui.gridLayout );
        } else if ( attr == QLatin1String("COMMENT") ) {
            if ( !pgp() )
                continue;
            validator = regex.isEmpty() ? Validation::pgpComment() : Validation::pgpComment( QRegExp( regex ) ) ;
            row = row_index_of( ui.commentLE, ui.gridLayout );
        } else {
            known = false;
            row = add_row( ui.gridLayout, &dynamicWidgets );
        }
        if ( !validator && !regex.isEmpty() )
            validator = new QRegExpValidator( QRegExp( regex ), 0 );

        QLineEdit * le = adjust_row( ui.gridLayout, row, label, preset, validator, readonly, required );

        const Line line = { key, label, regex, le };
        lines[row] = line;

        if ( !known )
            widgets.push_back( le );

        // don't connect twice:
        disconnect( le, SIGNAL(textChanged(QString)),
                    this, SLOT(slotUpdateResultLabel()) );
        connect( le, SIGNAL(textChanged(QString)),
                 this, SLOT(slotUpdateResultLabel()) );
    }

    // create lineList in visual order, so requirementsAreMet()
    // complains from top to bottom:
    lineList = kdtools::copy< QVector<Line> >( lines );

    widgets.push_back( ui.resultLE );
    widgets.push_back( ui.addEmailToDnCB );
    widgets.push_back( ui.advancedPB );

    set_tab_order( widgets );
}

QString EnterDetailsPage::cmsDN() const {
    DN dn;
    for ( QVector<Line>::const_iterator it = lineList.begin(), end = lineList.end() ; it != end ; ++it ) {
        const QString text = it->edit->text().trimmed();
        if ( text.isEmpty() )
            continue;
        QString attr = attributeFromKey( it->attr );
        if ( attr == QLatin1String("EMAIL") && !ui.addEmailToDnCB->isChecked() )
            continue;
        if ( const char * const oid = oidForAttributeName( attr ) )
            attr = QString::fromUtf8( oid );
        dn.append( DN::Attribute( attr, text ) );
    }
    return dn.dn();
}

QString EnterDetailsPage::pgpUserID() const {
    return Formatting::prettyNameAndEMail( OpenPGP, QString(),
                                           ui.nameLE->text().trimmed(),
                                           ui.emailLE->text().trimmed(),
                                           ui.commentLE->text().trimmed() );
}

static bool has_intermediate_input( const QLineEdit * le ) {
    QString text = le->text();
    int pos = le->cursorPosition();
    const QValidator * const v = le->validator();
    return v && v->validate( text, pos ) == QValidator::Intermediate ;
}

static bool requirementsAreMet( const QVector<Line> & list, QString & error ) {
  Q_FOREACH( const Line & line, list ) {
    const QLineEdit * le = line.edit;
    if ( !le )
      continue;
    const QString key = line.attr;
    qDebug() << "requirementsAreMet(): checking \"" << key << "\" against \"" << le->text() << "\":";
    if ( le->text().trimmed().isEmpty() ) {
        if ( key.endsWith(QLatin1Char('!')) ) {
            if ( line.regex.isEmpty() )
                error = xi18nc("@info","<interface>%1</interface> is required, but empty.", line.label );
            else
                error = xi18nc("@info","<interface>%1</interface> is required, but empty.<nl/>"
                              "Local Admin rule: <icode>%2</icode>", line.label, line.regex );
            return false;
        }
    } else if ( has_intermediate_input( le ) ) {
        if ( line.regex.isEmpty() )
            error = xi18nc("@info","<interface>%1</interface> is incomplete.", line.label );
        else
            error = xi18nc("@info","<interface>%1</interface> is incomplete.<nl/>"
                          "Local Admin rule: <icode>%2</icode>", line.label, line.regex );
        return false;
    } else if ( !le->hasAcceptableInput() ) {
        if ( line.regex.isEmpty() )
            error = xi18nc("@info","<interface>%1</interface> is invalid.", line.label );
        else
            error = xi18nc("@info","<interface>%1</interface> is invalid.<nl/>"
                          "Local Admin rule: <icode>%2</icode>", line.label, line.regex );
        return false;
    }
    qDebug() << "ok" << endl;
  }
  return true;
}

bool EnterDetailsPage::isComplete() const {
    QString error;
    const bool ok = requirementsAreMet( lineList, error );
    ui.errorLB->setText( error );
    return ok;
}

void EnterDetailsPage::slotAdvancedSettingsClicked() {
    dialog.exec();
}

QStringList KeyCreationPage::keyUsages() const {
    QStringList usages;
    if ( signingAllowed() )
        usages << QLatin1String("sign");
    if ( encryptionAllowed() && !is_dsa( keyType() ) )
        usages << QLatin1String("encrypt");
    if ( 0 ) // not needed in pgp (implied) and not supported in cms
    if ( certificationAllowed() )
        usages << QLatin1String("certify");
    if ( authenticationAllowed() )
        usages << QLatin1String("auth");
    return usages;
}

QStringList OverviewPage::i18nKeyUsages() const {
    QStringList usages;
    if ( signingAllowed() )
        usages << i18n("Sign");
    if ( encryptionAllowed() && !is_dsa( keyType() ) )
        usages << i18n("Encrypt");
    if ( 0 ) // not needed in pgp (implied) and not supported in cms
    if ( certificationAllowed() )
        usages << i18n("Certify");
    if ( authenticationAllowed() )
        usages << i18n("Authenticate");
    return usages;
}

QStringList KeyCreationPage::subkeyUsages() const {
    QStringList usages;
    if ( encryptionAllowed() && is_dsa( keyType() ) ) {
        assert( subkeyType() );
        assert( is_elg( subkeyType() ) );
        usages << QLatin1String("encrypt");
    }
    return usages;
}

QStringList OverviewPage::i18nSubkeyUsages() const {
    QStringList usages;
    if ( encryptionAllowed() && is_dsa( keyType() ) ) {
        assert( subkeyType() );
        assert( is_elg( subkeyType() ) );
        usages << i18n("Encrypt");
    }
    return usages;
}

QStringList OverviewPage::i18nCombinedKeyUsages() const {
    return i18nSubkeyUsages() + i18nKeyUsages();
}

namespace {
    template <typename T=QString>
    struct Row {
        QString key;
        T value;

        Row( const QString & k, const T & v ) : key( k ), value( v ) {}
    };
    template <typename T>
    QTextStream & operator<<( QTextStream & s, const Row<T> & row ) {
        if ( row.key.isEmpty() )
            return s;
        else
            return s << "<tr><td>" << row.key << "</td><td>" << row.value << "</td></tr>";
    }
}

QString OverviewPage::i18nFormatGnupgKeyParms( bool details ) const {
    QString result;
    QTextStream s( &result );
    s             << "<table>";
    if ( pgp() )
        s         << Row<        >( i18n("Name:"),              name() );
    s             << Row<        >( i18n("Email Address:"),     email() );
    if ( pgp() ) {
        if ( !comment().isEmpty() )
            s     << Row<        >( i18n("Comment:"),           comment() );
    } else {
        s         << Row<        >( i18n("Subject-DN:"),        DN( dn() ).dn( QLatin1String(",<br>") ) );
    }
    if ( details ) {
        s         << Row<        >( i18n("Key Type:"),          QLatin1String(gpgme_pubkey_algo_name( static_cast<gpgme_pubkey_algo_t>( keyType() ) )) );
        if ( const unsigned int strength = keyStrength() )
            s     << Row<        >( i18n("Key Strength:"),      i18np("1 bit", "%1 bits", strength ) );
        else
            s     << Row<        >( i18n("Key Strength:"),      i18n("default") );
        s         << Row<        >( i18n("Certificate Usage:"), i18nCombinedKeyUsages().join(i18nc("separator for key usages",",&nbsp;")) );
        if ( const unsigned int subkey = subkeyType() ) {
            s     << Row<        >( i18n("Subkey Type:"),       QLatin1String(gpgme_pubkey_algo_name( static_cast<gpgme_pubkey_algo_t>( subkey ) )) );
            if ( const unsigned int strength = subkeyStrength() )
                s << Row<        >( i18n("Subkey Strength:"),   i18np("1 bit", "%1 bits", strength ) );
            else
                s << Row<        >( i18n("Subkey Strength:"),   i18n("default") );
            s     << Row<        >( i18n("Subkey Usage:"),      i18nSubkeyUsages().join(i18nc("separator for key usages",",&nbsp;")) );
        }
    }
    if ( pgp() && details && expiryDate().isValid() )
        s         << Row<        >( i18n("Valid Until:"),       KLocale::global()->formatDate( expiryDate() ) );
    if ( !pgp() && details ) {
        Q_FOREACH( const QString & email, additionalEMailAddresses() )
            s     << Row<        >( i18n("Add. Email Address:"),email );
        Q_FOREACH( const QString & dns,   dnsNames() )
            s     << Row<        >( i18n("DNS Name:"),          dns );
        Q_FOREACH( const QString & uri,   uris() )
            s     << Row<        >( i18n("URI:"),               uri );
    }
    return result;
}

static QString encode_dns( const QString & dns ) {
    return QString::fromLatin1( QUrl::toAce( dns ) );
}

static QString encode_email( const QString & email ) {
    const int at = email.lastIndexOf( QLatin1Char('@') );
    if ( at < 0 )
        return email;
    return email.left( at + 1 ) + encode_dns( email.mid( at + 1 ) );
}

QString KeyCreationPage::createGnupgKeyParms() const {
    QString result;
    QTextStream s( &result );
    s     << "<GnupgKeyParms format=\"internal\">"         << endl;
    if ( pgp() )
        s << "%ask-passphrase"                             << endl;
    s     << "key-type:      " << gpgme_pubkey_algo_name( static_cast<gpgme_pubkey_algo_t>( keyType() ) ) << endl;
    if ( const unsigned int strength = keyStrength() )
        s << "key-length:    " << strength                 << endl;
    s     << "key-usage:     " << keyUsages().join(QLatin1String(" "))    << endl;
    if ( const unsigned int subkey = subkeyType() ) {
        s << "subkey-type:   " << gpgme_pubkey_algo_name( static_cast<gpgme_pubkey_algo_t>( subkey ) ) << endl;
        if ( const unsigned int strength = subkeyStrength() )
            s << "subkey-length: " << strength             << endl;
        s << "subkey-usage:  " << subkeyUsages().join(QLatin1String(" ")) << endl;
    }
    if ( pgp() && expiryDate().isValid() )
        s << "expire-date:   " << expiryDate().toString( Qt::ISODate ) << endl;
    s     << "name-email:    " << encode_email( email() )  << endl;
    if ( pgp() ) {
        s << "name-real:     " << name()                   << endl;
        if ( !comment().isEmpty() )
            s << "name-comment:  " << comment()            << endl;
    } else {
        s << "name-dn:       " << dn()                     << endl;
        Q_FOREACH( const QString & email, additionalEMailAddresses() )
            s << "name-email:    " << encode_email( email )<< endl;
        Q_FOREACH( const QString & dns,   dnsNames() )
            s << "name-dns:      " << encode_dns( dns )    << endl;
        Q_FOREACH( const QString & uri,   uris() )
            s << "name-uri:      " << uri                  << endl;
    }
    s     << "</GnupgKeyParms>"                            << endl;
    qDebug() << '\n' << result;
    return result;
}

static void fill_combobox( QComboBox & cb, const QList<int> & sizes, const QStringList & labels ) {
    cb.clear();
    for ( int i = 0, end = sizes.size() ; i != end ; ++i ) {
        cb.addItem( i < labels.size() && !labels[i].trimmed().isEmpty()
                    ? sizes[i] < 0
                      ? i18ncp( "%2: some admin-supplied text, %1: key size in bits", "%2 (1 bit; default)", "%2 (%1 bits; default)", -sizes[i], labels[i].trimmed() )
                      : i18ncp( "%2: some admin-supplied text, %1: key size in bits", "%2 (1 bit)", "%2 (%1 bits)", sizes[i], labels[i].trimmed() )
                    : sizes[i] < 0
                      ? i18ncp( "%1: key size in bits", "1 bit (default)", "%1 bits (default)", -sizes[i] )
                      : i18ncp( "%1: key size in bits", "1 bit", "%1 bits", sizes[i] ),
                    std::abs( sizes[i] ) );
        if ( sizes[i] < 0 )
            cb.setCurrentIndex( cb.count() - 1 );
    }
}

void AdvancedSettingsDialog::fillKeySizeComboBoxen() {

    const KConfigGroup config( KSharedConfig::openConfig(), "CertificateCreationWizard" );

    const QList<int> rsaKeySizes = config.readEntry( RSA_KEYSIZES_ENTRY, QList<int>() << 1536 << -2048 << 3072 << 4096 );
    const QList<int> dsaKeySizes = config.readEntry( DSA_KEYSIZES_ENTRY, QList<int>() << -2048 );
    const QList<int> elgKeySizes = config.readEntry( ELG_KEYSIZES_ENTRY, QList<int>() << 1536 << -2048 << 3072 << 4096 );

    const QStringList rsaKeySizeLabels = config.readEntry( RSA_KEYSIZE_LABELS_ENTRY, QStringList() );
    const QStringList dsaKeySizeLabels = config.readEntry( DSA_KEYSIZE_LABELS_ENTRY, QStringList() );
    const QStringList elgKeySizeLabels = config.readEntry( ELG_KEYSIZE_LABELS_ENTRY, QStringList() );

    fill_combobox( *ui.rsaKeyStrengthCB, rsaKeySizes, rsaKeySizeLabels );
    fill_combobox( *ui.dsaKeyStrengthCB, dsaKeySizes, dsaKeySizeLabels );
    fill_combobox( *ui.elgKeyStrengthCB, elgKeySizes, elgKeySizeLabels );

}

void AdvancedSettingsDialog::loadDefaultKeyType() {

    if ( protocol != CMS && protocol != OpenPGP )
        return;

    const KConfigGroup config( KSharedConfig::openConfig(), "CertificateCreationWizard" );

    const QString entry = protocol == CMS ? QLatin1String(CMS_KEY_TYPE_ENTRY) : QLatin1String(PGP_KEY_TYPE_ENTRY) ;
    const QString keyType = config.readEntry( entry ).trimmed().toUpper();

    if ( protocol == OpenPGP && keyType == QLatin1String("DSA") ) {
        setKeyType( GPGME_PK_DSA );
        setSubkeyType( 0 );
    } else if ( protocol == OpenPGP && keyType == QLatin1String("DSA+ELG") ) {
        setKeyType( GPGME_PK_DSA );
        setSubkeyType( GPGME_PK_ELG_E );
    } else {
        if ( !keyType.isEmpty() && keyType != QLatin1String("RSA") )
            qWarning() << "invalid value \"" << qPrintable( keyType )
                       << "\" for entry \"[CertificateCreationWizard]"
                       << qPrintable( entry ) << "\"";
        setKeyType( GPGME_PK_RSA );
        setSubkeyType( 0 );
    }

    keyTypeImmutable = config.isEntryImmutable( entry );
    updateWidgetVisibility();
}

void AdvancedSettingsDialog::updateWidgetVisibility() {
    // Personal Details Page
    if ( protocol == OpenPGP ) {  // ### hide until multi-uid is implemented
        if ( ui.tabWidget->indexOf( ui.personalTab ) != -1 )
            ui.tabWidget->removeTab( ui.tabWidget->indexOf( ui.personalTab ) );
    } else {
        if ( ui.tabWidget->indexOf( ui.personalTab ) == -1 )
            ui.tabWidget->addTab( ui.personalTab, tr2i18n( "Personal Details", 0 ) );
    }
    ui.uidGB->setVisible( protocol == OpenPGP );
    ui.uidGB->setEnabled( false );
    ui.uidGB->setToolTip( i18nc("@info:tooltip","Adding more than one User ID is not yet implemented.") );
    ui.emailGB->setVisible( protocol == CMS );
    ui.dnsGB->setVisible( protocol == CMS );
    ui.uriGB->setVisible( protocol == CMS );
    // Technical Details Page
    if ( keyTypeImmutable ) {
        ui.rsaRB->setEnabled( false );
        ui.dsaRB->setEnabled( false );
        ui.elgCB->setEnabled( false );
    } else {
        ui.rsaRB->setEnabled( true );
        ui.dsaRB->setEnabled( protocol == OpenPGP );
        ui.elgCB->setEnabled( protocol == OpenPGP );
    }
    ui.certificationCB->setVisible( protocol == OpenPGP ); // gpgsm limitation?
    ui.authenticationCB->setVisible( protocol == OpenPGP );
    if ( protocol == OpenPGP ) { // pgp keys must have certify capability
        ui.certificationCB->setChecked( true );
        ui.certificationCB->setEnabled( false );
    }
    if ( protocol == CMS ) {
        ui.encryptionCB->setEnabled( true );
    }
    ui.expiryDE->setVisible( protocol == OpenPGP );
    ui.expiryCB->setVisible( protocol == OpenPGP );
    slotKeyMaterialSelectionChanged();
}

#include "newcertificatewizard.moc"
