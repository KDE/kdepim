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

#include <utils/formatting.h>
#include <utils/stl_util.h>

#include <kleo/dn.h>
#include <kleo/oidmap.h>
#include <kleo/keygenerationjob.h>
#include <kleo/cryptobackendfactory.h>
#include <kleo/cryptobackend.h>

#include <gpgme++/global.h>
#include <gpgme++/keygenerationresult.h>
#include <gpgme.h>

#include <KConfigGroup>
#include <KGlobal>
#include <KLocale>
#include <KDebug>
#include <KTempDir>
#include <KMessageBox>
#include <KToolInvocation>

#include <QRegExpValidator>
#include <QLineEdit>
#include <QMetaProperty>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QFileDialog>

#include <boost/range.hpp>

#include <algorithm>

using namespace Kleo;
using namespace Kleo::NewCertificateUi;
using namespace Kleo::Commands;
using namespace GpgME;
using namespace boost;

static void set_tab_order( const QList<QWidget*> & wl ) {
    kdtools::for_each_adjacent_pair( wl, &QWidget::setTabOrder );
}

static const unsigned int key_strengths[] = {
    0, 1024, 1532, 2048, 3072, 4096,
};
static const unsigned int num_key_strengths = sizeof key_strengths / sizeof *key_strengths;

static unsigned int index2strength( unsigned int index ) {
    if ( index < num_key_strengths )
        return key_strengths[index];
    else
        return 0;
}

static int strength2index( unsigned int strength ) {
    const unsigned int * const it = 
        std::lower_bound( begin( key_strengths ), end( key_strengths ), strength );
    if ( it == end( key_strengths ) )
        return key_strengths[num_key_strengths-1];
    else
        return *it;
}

static unsigned int version2strength( int version ) {
    if ( version == 1 )
        return 1024;
    if ( version == 2 )
        return 2048;
    return 0;
}

static int strength2version( unsigned int strength ) {
    if ( strength == 0 )
        return 0;
    if ( strength <= 1024 )
        return 1;
    if ( strength <= 2048 )
        return 2;
    return 0;
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

namespace {

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
        
    protected Q_SLOTS:
        void setButtonVisible( QWizard::WizardButton button, bool visible ) {
            if ( QAbstractButton * const b = this->button( button ) )
                b->setVisible( visible );
        }

    protected:
#define FIELD(type, name) type name() const { return field( #name ).value<type>(); }
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
            : QDialog( parent ), protocol( UnknownProtocol ), ui()
        {
            ui.setupUi( this );
            const QDate today = QDate::currentDate();
            ui.expiryDE->setMinimumDate( today );
            ui.expiryDE->setDate( today.addYears( 2 ) );
            ui.emailLW->setDefaultValue( i18n("new email") );
            ui.dnsLW->setDefaultValue( i18n("new dns name") );
            ui.uriLW->setDefaultValue( i18n("new uri") );
        }

        void setProtocol( GpgME::Protocol proto ) {
            if ( protocol == proto )
                return;
            protocol = proto;
            updateWidgetVisibility();
        }

        void updateWidgetVisibility() {
            // Personal Details Page
            ui.uidGB->setVisible( protocol == OpenPGP );
            ui.emailGB->setVisible( protocol == CMS );
            ui.dnsGB->setVisible( protocol == CMS );
            ui.uriGB->setVisible( protocol == CMS );
            // Technical Details Page
            ui.dsaRB->setEnabled( protocol == OpenPGP );
            ui.elgCB->setEnabled( protocol == OpenPGP );
            if ( protocol == CMS ) {
                ui.rsaRB->setChecked( true ); // gpgsm can generate only rsa atm
                ui.elgCB->setChecked( false );
            } else {
                ui.dsaRB->setChecked( true ); // default for OpenPGP
                ui.elgCB->setChecked( true );
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

        void setAdditionalUserIDs( const QStringList & items ) { ui.uidLW->setItems( items ); }
        QStringList additionalUserIDs() const { return ui.uidLW->items();   }

        void setAdditionalEMailAddresses( const QStringList & items ) { ui.emailLW->setItems( items ); }
        QStringList additionalEMailAddresses() const { return ui.emailLW->items(); }

        void setDnsNames( const QStringList & items ) { ui.dnsLW->setItems( items ); }
        QStringList dnsNames() const { return ui.dnsLW->items();   }

        void setUris( const QStringList & items ) { ui.uriLW->setItems( items ); }
        QStringList uris() const { return ui.uriLW->items();   }


        void setKeyStrength( unsigned int strength ) {
            ui.rsaKeyStrengthCB->setCurrentIndex( strength2index( strength ) );
            ui.dsaVersionCB->setCurrentIndex( strength2version( strength ) );
        }
        unsigned int keyStrength() const {
            return
                ui.dsaRB->isChecked() ? version2strength( ui.dsaVersionCB->currentIndex() ) :
                ui.rsaRB->isChecked() ? index2strength( ui.rsaKeyStrengthCB->currentIndex() ) : 0 ;
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
            ui.elgKeyStrengthCB->setCurrentIndex( strength2index( strength ) );
        }
        unsigned int subkeyStrength() const {
            return index2strength( ui.elgKeyStrengthCB->currentIndex() );
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
                ui.elgCB->setEnabled( is_dsa( algo ) );
                if ( sender() == ui.dsaRB || sender() == ui.rsaRB )
                    ui.elgCB->setChecked( is_dsa( algo ) );
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
                assert( is_rsa( keyType() ) );
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
        GpgME::Protocol protocol;
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
            registerField( "pgp", ui.pgpCLB );
        }

        /* reimp */ void initializePage() {
            if ( !initialized ) {
                connect( ui.pgpCLB,  SIGNAL(clicked()), wizard(), SLOT(next()), Qt::QueuedConnection );
                connect( ui.x509CLB, SIGNAL(clicked()), wizard(), SLOT(next()), Qt::QueuedConnection );
            }
            initialized = true;
        }

        /* reimp */ bool isComplete() const {
            return ui.pgpCLB->isChecked() || ui.x509CLB->isChecked() ;
        }

    private:
        bool initialized : 1;
        Ui_ChooseProtocolPage ui;
    };

    class EnterDetailsPage : public WizardPage {
        Q_OBJECT
    public:
        explicit EnterDetailsPage( QWidget * p=0 )
            : WizardPage( p ), dialog( this ), ui()
        {
            ui.setupUi( this );
            connect( ui.resultLE, SIGNAL(textChanged(QString)),
                     SIGNAL(completeChanged()) );
            connect( ui.addEmailToDnCB, SIGNAL(toggled(bool)),
                     SLOT(slotUpdateResultLabel()) );
            registerDialogPropertiesAsFields();
            registerField( "dn", ui.resultLE );
            registerField( "name", ui.nameLE );
            registerField( "email", ui.emailLE );
            registerField( "comment", ui.commentLE );
            updateForm();
        }

        /* reimp */ bool isComplete() const;
        /* reimp */ void initializePage() {
            updateForm();
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
        QVector< QPair<QString,QLineEdit*> > attributePairList;
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

    private:
        Ui_OverviewPage ui;
    };

    class KeyCreationPage : public WizardPage {
        Q_OBJECT
    public:
        explicit KeyCreationPage( QWidget * p=0 )
            : WizardPage( p ),
              tmp( QDir::temp().absoluteFilePath( "kleo-" ) ),
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
                setField( "error", i18n("Could not start certificate creation: %1",
                                        QString::fromLocal8Bit( err.asString() ) ) );
            else
                job = j;
        }
        QStringList keyUsages() const;
        QStringList subkeyUsages() const;
        QString createGnupgKeyParms() const;

    private Q_SLOTS:
        void slotResult( const GpgME::KeyGenerationResult & result, const QByteArray & request, const QString & auditLog ) {
            if ( result.error().code() ) {
                setField( "error", result.error().isCanceled()
                          ? i18n("Operation canceled.")
                          : i18n("Could not create certificate: %1",
                                 QString::fromLocal8Bit( result.error().asString() ) ) );
                setField( "url", QString() );
                setField( "result", QString() );
            } else if ( pgp() ) {
                setField( "error", QString() );
                setField( "url", QString() );
                setField( "result", i18n("Certificate created successfully.\n"
                                         "Fingerprint: %1", result.fingerprint() ) );
            } else {
                QFile file( QDir( tmp.name() ).absoluteFilePath( "request.p10" ) );

                if ( !file.open( QIODevice::WriteOnly ) ) {
                    setField( "error", i18n("Could not write output file %1: %2",
                                            file.fileName(), file.errorString() ) );
                    setField( "url", QString() );
                    setField( "result", QString() );
                } else {
                    file.write( request );
                    setField( "error", QString() );
                    setField( "url", QUrl::fromLocalFile( file.fileName() ).toString() );
                    setField( "result", i18n("Certificate created successfully.") );
                }
            }
            setField( "fingerprint", QString::fromLatin1( result.fingerprint() ) );
            job = 0;
            emit completeChanged();
            QMetaObject::invokeMethod( wizard(), "next", Qt::QueuedConnection );
        }

    private:
        KTempDir tmp;
        QPointer<KeyGenerationJob> job;
        Ui_KeyCreationPage ui;
    };

    class ResultPage : public WizardPage {
        Q_OBJECT
    public:
        explicit ResultPage( QWidget * p=0 )
            : WizardPage( p ), initialized( false ), ui()
        {
            ui.setupUi( this );
            ui.dragQueen->setPixmap( KIcon( "kleopatra" ).pixmap( 64, 64 ) );
            registerField( "error",  ui.errorTB,   "plainText" );
            registerField( "result", ui.resultTB,  "plainText" );
            registerField( "url",    ui.dragQueen, "url" );
            // hidden field, since QWizard can't deal with non-widget-backed fields...
            QLineEdit * le = new QLineEdit( this );
            le->hide();
            registerField( "fingerprint", le );
        }

        /* reimp */ void initializePage() {
            const bool error = isError();
            ui.resultTB                 ->setVisible( !error );
            ui.errorTB                  ->setVisible(  error );
            ui.dragQueen                ->setVisible( !error && !pgp() );
            ui.restartWizardPB          ->setVisible(  error );
            ui.nextStepsGB              ->setVisible( !error );
            ui.saveRequestToFilePB      ->setVisible( !pgp() );
            ui.uploadToKeyserverPB      ->setVisible(  pgp() );
            ui.makeBackupPB             ->setVisible(  pgp() );
            ui.createRevocationRequestPB->setVisible(  pgp() && false ); // not implemented
            ui.sendRequestByEMailPB     ->setVisible( !pgp() );
            ui.sendCertificateByEMailPB ->setVisible(  pgp() );

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
            QString fileName = QFileDialog::getSaveFileName( this, i18nc("@title", "Save Request") );
            if ( fileName.isEmpty() )
                return;
            if ( !fileName.endsWith( ".p10", Qt::CaseInsensitive ) )
                fileName += ".p10";
            QFile src( QUrl( url() ).toLocalFile() );
            if ( !src.copy( fileName ) )
                KMessageBox::error( this,
                                    i18nc("@info",
                                          "Could not copy temporary file <filename>%1</filename> "
                                          "to file <filename>%2</filename>: <message>%3</message>",
                                          src.fileName(), fileName, src.errorString() ),
                                    i18nc("@title", "Error Saving Request") );
            else
                KMessageBox::information( this,
                                          i18nc("@info",
                                                "<para>Successfully wrote request to <filename>%1</filename>.</para>"
                                                "<para>You should now send the request to the Certification Authority (CA).</para>" ),
                                          i18nc("@title", "Request Saved" ) );
        }

        void slotSendRequestByEMail() {
            if ( pgp() )
                return;
            const KConfigGroup config( KGlobal::config(), "CertificateCreationWizard" );
            KToolInvocation::invokeMailer( config.readEntry( "CAEmailAddress" ), QString() /*cc*/, QString() /*bcc*/,
                                           i18n("Please process this certificate."),
                                           i18n("Please process this certificate and inform the sender about the location to fetch the resulting certificate.\n\nThanks,\n"),
                                           QString(), // unused
                                           QStringList( url() ) );
        }

        void slotSendCertificateByEMail() {

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

    private:
        bool initialized;
        Ui_ResultPage ui;
    };
}

class NewCertificateWizard::Private {
    friend class ::Kleo::NewCertificateWizard;
    NewCertificateWizard * const q;
public:
    explicit Private( NewCertificateWizard * qq )
        : q( qq ),
          ui( q )
    {

    }

private:
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

static QString pgpLabel( const QString & attr ) {
    if ( attr == "NAME" )
        return i18n("Name");
    if ( attr == "COMMENT" )
        return i18n("Comment");
    if ( attr == "EMAIL" )
        return i18n("EMail");
    return QString();
}

static QString attributeLabel( const QString & attr, bool pgp ) {
  if ( attr.isEmpty() )
    return QString();
  const QString label = pgp ? pgpLabel( attr ) : Kleo::DNAttributeMapper::instance()->name2label( attr ) ;
  if ( !label.isEmpty() )
      if ( pgp )
          return label + ':';
      else
          return i18nc("Format string for the labels in the \"Your Personal Data\" page",
                       "%1 (%2):", label, attr );
  else
    return attr + ':';
}

static QString attributeFromKey( QString key ) {
  return key.remove( '!' );
}

static const char * oidForAttributeName( const QString & attr ) {
  QByteArray attrUtf8 = attr.toUtf8();
  for ( unsigned int i = 0 ; i < numOidMaps ; ++i )
    if ( qstricmp( attrUtf8, oidmap[i].name ) == 0 )
      return oidmap[i].oid;
  return 0;
}

void EnterDetailsPage::registerDialogPropertiesAsFields() {

    const QMetaObject * const mo = dialog.metaObject();
    for ( unsigned int i = mo->propertyOffset(), end = i + mo->propertyCount() ; i != end ; ++i ) {
        const QMetaProperty mp = mo->property( i );
        if ( mp.isValid() )
            registerField( mp.name(), &dialog, mp.name(), SIGNAL(accepted()) );
    }

}

void EnterDetailsPage::saveValues() {
    for ( QVector< QPair<QString,QLineEdit*> >::const_iterator it = attributePairList.begin(), end = attributePairList.end() ; it != end ; ++it )
        savedValues[ attributeFromKey(it->first) ] = it->second->text().trimmed();
}

void EnterDetailsPage::clearForm() {
    qDeleteAll( dynamicWidgets );
    dynamicWidgets.clear();
    attributePairList.clear();

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

static QLineEdit * adjust_row( QGridLayout * l, int row, const QString & label, const QString & preset, const QString & regex, bool readonly, bool required ) {
    assert( l );
    assert( row >= 0 );
    assert( row < l->rowCount() );

    QLabel * lb = qobject_cast<QLabel*>( l->itemAtPosition( row, 0 )->widget() );
    assert( lb );
    QLineEdit * le = qobject_cast<QLineEdit*>( l->itemAtPosition( row, 1 )->widget() );
    assert( le );
    QLabel * reqLB = qobject_cast<QLabel*>( l->itemAtPosition( row, 2 )->widget() );
    assert( reqLB );

    lb->setText( label );
    le->setText( preset );
    reqLB->setText( required ? i18n("(required)") : i18n("(optional)") );
    if ( !required && regex.isEmpty() )
        delete le->validator();
    else
        le->setValidator( new QRegExpValidator( QRegExp( regex.isEmpty() ? QLatin1String( "[^\\s].*" ) : regex ), le ) );

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

    const KConfigGroup config( KGlobal::config(), "CertificateCreationWizard" );

    QStringList attrOrder = config.readEntry( pgp() ? "OpenPGPAttributeOrder" : "DNAttributeOrder", QStringList() );
    if ( attrOrder.empty() )
        if ( pgp() )
            attrOrder << "NAME!" << "EMAIL!" << "COMMENT";
        else
            attrOrder << "CN!" << "L" << "OU" << "O!" << "C!" << "EMAIL!";

    QList<QWidget*> widgets;
    widgets.push_back( ui.nameLE );
    widgets.push_back( ui.emailLE );
    widgets.push_back( ui.commentLE );

    Q_FOREACH( const QString & rawKey, attrOrder ) {
        const QString key = rawKey.trimmed().toUpper();
        const QString attr = attributeFromKey( key );
        if ( attr.isEmpty() )
            continue;
        const QString preset = savedValues.value( attr, config.readEntry( attr, QString() ) );
        const bool required = key.endsWith( QLatin1Char('!') );
        const bool readonly = config.isEntryImmutable( attr );
        const QString label = config.readEntry( attr + "_label",
                                                attributeLabel( attr, pgp() ) );
        const QString regex = config.readEntry( attr + "_regex" );

        int row;
        bool known = true;
        if ( attr == "EMAIL" ) {
            row = row_index_of( ui.emailLE, ui.gridLayout );
            if ( !pgp() )
                ui.addEmailToDnCB->show();
        } else if ( attr == "NAME" || attr == "CN" ) {
            if ( pgp() && attr == "CN" || !pgp() && attr == "NAME" )
                continue;
            row = row_index_of( ui.nameLE, ui.gridLayout );
        } else if ( attr == "COMMENT" ) {
            if ( !pgp() )
                continue;
            row = row_index_of( ui.commentLE, ui.gridLayout );
        } else {
            known = false;
            row = add_row( ui.gridLayout, &dynamicWidgets );
        }
        QLineEdit * le = adjust_row( ui.gridLayout, row, label, preset, regex, readonly, required );

        attributePairList.append( qMakePair( key, le ) );

        if ( !known )
            widgets.push_back( le );

        // don't connect twice:
        le->disconnect( this );
        connect( le, SIGNAL(textChanged(QString)),
                 this, SLOT(slotUpdateResultLabel()) );
    }

    widgets.push_back( ui.resultLE );
    widgets.push_back( ui.addEmailToDnCB );
    widgets.push_back( ui.advancedPB );

    set_tab_order( widgets );
}

QString EnterDetailsPage::cmsDN() const {
    DN dn;
    for ( QVector< QPair<QString,QLineEdit*> >::const_iterator it = attributePairList.begin(), end = attributePairList.end() ; it != end ; ++it ) {
        const QString text = it->second->text().trimmed();
        if ( text.isEmpty() )
            continue;
        QString attr = attributeFromKey( it->first );
        if ( attr == "EMAIL" && !ui.addEmailToDnCB->isChecked() )
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

static bool requirementsAreMet( const QVector< QPair<QString,QLineEdit*> > & list ) {
    for ( QVector< QPair<QString,QLineEdit*> >::const_iterator it = list.begin() ;
          it != list.end() ; ++it ) {
    const QLineEdit * le = (*it).second;
    if ( !le )
      continue;
    const QString key = (*it).first;
    kDebug() << "requirementsAreMet(): checking \"" << key << "\" against \"" << le->text() << "\":";
    if ( key.endsWith('!') && !le->hasAcceptableInput() ) {
      kDebug() << "required field has non-acceptable input!";
      return false;
    }
    kDebug() << "ok" << endl;
  }
  return true;
}

bool EnterDetailsPage::isComplete() const {
    return requirementsAreMet( attributePairList );
}

void EnterDetailsPage::slotAdvancedSettingsClicked() {
    dialog.setProtocol( pgp() ? OpenPGP : CMS );
    dialog.exec();
}

QStringList KeyCreationPage::keyUsages() const {
    QStringList usages;
    if ( signingAllowed() )
        usages << "sign";
    if ( encryptionAllowed() && !is_dsa( keyType() ) )
        usages << "encrypt";
    if ( 0 ) // not needed in pgp (implied) and not supported in cms
    if ( certificationAllowed() )
        usages << "certify";
    if ( authenticationAllowed() )
        usages << "auth";
    return usages;
}

QStringList KeyCreationPage::subkeyUsages() const {
    QStringList usages;
    if ( encryptionAllowed() && is_dsa( keyType() ) ) {
        assert( subkeyType() );
        assert( is_elg( subkeyType() ) );
        usages << "encrypt";
    }
    return usages;
}

static QString encode_dns( const QString & dns ) {
    return QString::fromLatin1( QUrl::toAce( dns ) );
}

static QString encode_email( const QString & email ) {
    const int at = email.lastIndexOf( '@' );
    if ( at < 0 )
        return email;
    return email.left( at + 1 ) + encode_dns( email.mid( at + 1 ) );
}

QString KeyCreationPage::createGnupgKeyParms() const {
    QString result;
    QTextStream s( &result );
    s     << "<GnupgKeyParms format=\"internal\">"         << endl
          << "key-type:      " << gpgme_pubkey_algo_name( static_cast<gpgme_pubkey_algo_t>( keyType() ) ) << endl;
    if ( const unsigned int strength = keyStrength() )
        s << "key-length:    " << strength                 << endl;
    s     << "key-usage:     " << keyUsages().join(" ")    << endl;
    if ( const unsigned int subkey = subkeyType() ) {
        s << "subkey-type:   " << gpgme_pubkey_algo_name( static_cast<gpgme_pubkey_algo_t>( subkey ) ) << endl;
        if ( const unsigned int strength = subkeyStrength() )
            s << "subkey-length: " << strength             << endl;
        s << "subkey-usage:  " << subkeyUsages().join(" ") << endl;
    }
    if ( pgp() && expiryDate().isValid() )
        s << "expire-date:   " << expiryDate().toString( Qt::ISODate ) << endl;
    s     << "name-email:    " << encode_email( email() )  << endl;
    if ( pgp() )
        s << "name-real:     " << name()                   << endl
          << "name-comment:  " << comment()                << endl;
    else {
        s << "name-dn:       " << dn()                     << endl;
        Q_FOREACH( const QString & email, additionalEMailAddresses() )
            s << "name-email:    " << encode_email( email )<< endl;
        Q_FOREACH( const QString & dns,   dnsNames() )
            s << "name-dns:      " << encode_dns( dns )    << endl;
        Q_FOREACH( const QString & uri,   uris() )
            s << "name-uri:      " << uri                  << endl;
    }
    if ( pgp() )
        s << "%ask-passphrase"                             << endl;
    s     << "</GnupgKeyParms>"                            << endl;
    kDebug() << '\n' << result;
    return result;
}

#include "moc_newcertificatewizard.cpp"
#include "newcertificatewizard.moc"
