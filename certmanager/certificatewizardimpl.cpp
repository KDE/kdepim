#include "certificatewizardimpl.h"

// kdenetwork
#include <cryptplugwrapper.h>

// KDE
#include <kmessagebox.h>
#include <klocale.h>
#include <kapplication.h>

// Qt
#include <qlineedit.h>
#include <qtextedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

#include <iostream>

using namespace std;

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
    setNextEnabled( personalDataPage, false );
    nameED->setFocus();
    
    connect( this, SIGNAL( helpClicked() ),
	     this, SLOT( slotHelpClicked() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
CertificateWizardImpl::~CertificateWizardImpl()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
  This slot is called when the user changes the text in the email
  address field.
*/
void CertificateWizardImpl::slotEmailAddressChanged( const QString& text )
{
    setNextEnabled( personalDataPage, text.contains( '@' ) );
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
    certParms += "name-dn: ";
    bool bFirst = true;
    if( !nameED->text().isEmpty() ) {
        if( !bFirst )
            certParms += ',';
        certParms += "CN=";
        certParms += nameED->text();
        bFirst = false;
    }
    if( !locationED->text().isEmpty() ) {
        if( !bFirst )
            certParms += ',';
        certParms += "L=";
        certParms += locationED->text();
        bFirst = false;
    }
    if( !departmentED->text().isEmpty() ) {
        if( !bFirst )
            certParms += ',';
        certParms += "OU=";
        certParms += departmentED->text();
        bFirst = false;
    }
    if( !organizationED->text().isEmpty() ) {
        if( !bFirst )
            certParms += ',';
        certParms += "O=";
        certParms += organizationED->text();
        bFirst = false;
    }
    if( !countryED->text().isEmpty() ) {
        if( !bFirst )
            certParms += ',';
        certParms += "C=";
        certParms += countryED->text();
        bFirst = false;
    }
    if ( brokenCA->isChecked() ){
	// a broken CA is described as a CA that requires an Email tag
	// for X509v1 certificate requests. As the cryptplug stuff
	// implements X509v3, we need to pass the number encoded already
	// and also convert the QLineEdit 16 bit output to 8 bit first. 
	// Then convert it to hex and put it tinto the certParms string:
	QString emailstr=QString(emailED->text().ascii());
	const unsigned short *chars = emailstr.ucs2();
	QString hexString;
	for(unsigned int i = 0; i < emailstr.length(); i++){
		hexString.append("\\");
		hexString.append(QString::number(chars[i], 16));
	}
	if( !bFirst )
		certParms += ',';

        /* Actually this encoding is not correct because the gpgsm API
         says that an RFC-2253 encoding is to be used.  Due to wrong
         encoding created by libksba the actual encoding is not RFC
         conform but a hack which assumes that the argument is a kind
         of string and libksba decides for itself what kind of ASN.1
         type to use (printable-string or utf8-string).  Fortunately
         we can easily discriminate between that bad encoding and
         regular encoding based on the fact that the regular encoding
         does not start with a byte that can represent a printable
         character - well, at least for email addresses this is true.

         The other compatible and regular choice we have, is to encode
         it in a regular way; i.e as defined by rfc-2253 with regular
         quoting (note, the SHOULD in the RFC is only relevant for the
         DER -> RFC-2253 encoding direction).  I'd vote for that
         because UCS-2 converting to UTF-8 is pretty simple and the
         quoting rules can basically be ignored and simple
         backslashquoting used for all characters.  An Email address of
         <foo@bar> would then be encoded as:

           1.2.840.113549.1.9.1=\66\6F\6F\40\62\61\72

         I am not firm with Qt classes, so I won't do that but use the
         irregular way. -- wk@gnupg.org */

        certParms += QString("1.2.840.113549.1.9.1=%1").arg(hexString);
        bFirst = false;
    }
    certParms += '\n';
    certParms += QString( "name-email: %1\n" ).arg( emailED->text() );
    certParms += "</GnupgKeyParms>\n";
    
	cout << certParms.utf8() << endl;
    
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

void CertificateWizardImpl::slotHelpClicked()
{
  kapp->invokeHelp( "newcert" );
}

#include "certificatewizardimpl.moc"
