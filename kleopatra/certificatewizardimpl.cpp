#include "certificatewizardimpl.h"
#include "cryptplugwrapper.h"
#include <qlineedit.h>
#include <qtextedit.h>
#include <kmessagebox.h>
#include <klocale.h>

extern CryptPlugWrapper* pWrapper;

/*
 *  Constructs a CertificateWizardImpl which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The wizard will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal wizard.
 */
CertificateWizardImpl::CertificateWizardImpl( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : CertificateWizard( parent, name, modal, fl )
{
    // don't allow to go to last page until a key has been generated
    setNextEnabled( generatePage, false );
    nameED->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
CertificateWizardImpl::~CertificateWizardImpl()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 * protected slot
 */
void CertificateWizardImpl::slotGenerateCertificate()
{
    // Ask gpgme to generate a key and return it
    char* generatedKey;
    int keyLength;

    QString certParms;
    certParms += "<GnupgKeyParms format=\"internal\">\n";
    certParms += "Key-Type: RSA\n";
    certParms += "Key-Length: 1024\n"; // PENDING(NN) Might want to make this user-configurable
    certParms += "Key-Usage: Sign, Encrypt\n"; // PENDING(NN) Might want to make this user-configurable
    certParms += QString( "name-dn: C=%1,O=%2,OU=%3,CN=%4\n" ).arg( countryED->text() ).arg( organizationED->text() ).arg( departmentED->text() ).arg( nameED->text() );
    certParms += QString( "name-email: %1\n" ).arg( emailED->text() );
    certParms += "</GnupgKeyParms>\n";

    QApplication::setOverrideCursor( Qt::waitCursor );
    if( !pWrapper->requestDecentralCertificate( certParms.utf8(),
                                                  &generatedKey,
                                                  &keyLength ) ) {
          setNextEnabled( generatePage, false );
          setFinishEnabled( finishPage, false );
          KMessageBox::error( this,
                              i18n( "Could not generate certificate" ),
                              i18n( "Certificate Manager Error" ) );
    } else {
        // next will stay enabled until the user clicks Generate
        // Certificate again
        setNextEnabled( generatePage, true );
        setFinishEnabled( finishPage, true );
        qDebug( "data length was %d", keyLength );
        _keyData.duplicate( generatedKey, keyLength );
        free( generatedKey );
        certificateTE->setText( certParms );
    }
    QApplication::restoreOverrideCursor();
}


