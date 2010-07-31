/*
    certificatewizardimpl.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klar�vdalens Datakonsult AB

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "certificatewizardimpl.h"
#include "storedtransferjob.h"

// libkleopatra
#include <kleo/oidmap.h>
#include <kleo/keygenerationjob.h>
#include <kleo/dn.h>
#include <kleo/cryptobackendfactory.h>

#include <ui/progressdialog.h>

// gpgme++
#include <gpgmepp/keygenerationresult.h>

// KDE
#include <kabc/stdaddressbook.h>
#include <kabc/addressee.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kurlrequester.h>
#include <kdcopservicestarter.h>
#include <dcopclient.h>
#include <kio/job.h>
#include <kio/netaccess.h>

// Qt
#include <tqlineedit.h>
#include <tqtextedit.h>
#include <tqpushbutton.h>
#include <tqcheckbox.h>
#include <tqradiobutton.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqcombobox.h>

#include <assert.h>
#include <dcopref.h>

static const unsigned int keyLengths[] = {
  1024, 1532, 2048, 3072, 4096
};
static const unsigned int numKeyLengths = sizeof keyLengths / sizeof *keyLengths;

static TQString attributeLabel( const TQString & attr, bool required ) {
  if ( attr.isEmpty() )
    return TQString::null;
  const TQString label = Kleo::DNAttributeMapper::instance()->name2label( attr );
  if ( !label.isEmpty() )
    if ( required )
      return i18n("Format string for the labels in the \"Your Personal Data\" page - required field",
		  "*%1 (%2):").arg( label, attr );
    else
      return i18n("Format string for the labels in the \"Your Personal Data\" page",
		  "%1 (%2):").arg( label, attr );

  else if ( required )
    return '*' + attr + ':';
  else
    return attr + ':';
}

static TQString attributeFromKey( TQString key ) {
  return key.remove( '!' );
}

static bool availForMod( const TQLineEdit * le ) {
  return le && le->isEnabled();
}

/*
 *  Constructs a CertificateWizardImpl which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The wizard will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal wizard.
 */
CertificateWizardImpl::CertificateWizardImpl( TQWidget* parent,  const char* name, bool modal, WFlags fl )
    : CertificateWizard( parent, name, modal, fl )
{
    // don't allow to go to last page until a key has been generated
    setNextEnabled( generatePage, false );
    // setNextEnabled( personalDataPage, false ); // ## disable again once we have a criteria when to enable again

    createPersonalDataPage();

    // Allow to select remote URLs
    storeUR->setMode( KFile::File );
    storeUR->setFilter( "application/pkcs10" );
    connect( storeUR, TQT_SIGNAL( urlSelected( const TQString& ) ),
             this, TQT_SLOT( slotURLSelected( const TQString& ) ) );

    const KConfigGroup config( KGlobal::config(), "CertificateCreationWizard" );
    caEmailED->setText( config.readEntry( "CAEmailAddress" ) );

    connect( this, TQT_SIGNAL( helpClicked() ),
	     this, TQT_SLOT( slotHelpClicked() ) );
    connect( insertAddressButton, TQT_SIGNAL( clicked() ),
	     this, TQT_SLOT( slotSetValuesFromWhoAmI() ) );

    for ( unsigned int i = 0 ; i < numKeyLengths ; ++i )
      keyLengthCB->insertItem( i18n("%n bit", "%n bits", keyLengths[i] ) );
}

static bool requirementsAreMet( const CertificateWizardImpl::AttrPairList & list ) {
  for ( CertificateWizardImpl::AttrPairList::const_iterator it = list.begin() ;
	it != list.end() ; ++it ) {
    const TQLineEdit * le = (*it).second;
    if ( !le )
      continue;
    const TQString key = (*it).first;
#ifndef NDEBUG
    kdbgstream s = kdDebug();
#else
    kndbgstream s = kdDebug();
#endif
    s << "requirementsAreMet(): checking \"" << key << "\" against \"" << le->text() << "\": ";
    if ( key.endsWith("!") && le->text().stripWhiteSpace().isEmpty() ) {
      s << "required field is empty!" << endl;
      return false;
    }
    s << "ok" << endl;
  }
  return true;
}

/*
  This slot is called when the user changes the text.
 */
void CertificateWizardImpl::slotEnablePersonalDataPageExit() {
  setNextEnabled( personalDataPage, requirementsAreMet( _attrPairList ) );
}


/*
 *  Destroys the object and frees any allocated resources
 */
CertificateWizardImpl::~CertificateWizardImpl()
{
    // no need to delete child widgets, Qt does it all for us
}

static const char * oidForAttributeName( const TQString & attr ) {
  TQCString attrUtf8 = attr.utf8();
  for ( unsigned int i = 0 ; i < numOidMaps ; ++i )
    if ( qstricmp( attrUtf8, oidmap[i].name ) == 0 )
      return oidmap[i].oid;
  return 0;
}

/*
 * protected slot
 */
void CertificateWizardImpl::slotGenerateCertificate()
{
    // Ask gpgme to generate a key and return it
    TQString certParms;
    certParms += "<GnupgKeyParms format=\"internal\">\n";
    certParms += "Key-Type: RSA\n";
    certParms += TQString( "Key-Length: %1\n" ).arg( keyLengths[keyLengthCB->currentItem()] );
    certParms += "Key-Usage: ";
    if ( signOnlyCB->isChecked() )
      certParms += "Sign";
    else if ( encryptOnlyCB->isChecked() )
      certParms += "Encrypt";
    else
      certParms += "Sign, Encrypt";
    certParms += "\n";
    certParms += "name-dn: ";

    TQString email;
    TQStringList rdns;
    for( AttrPairList::const_iterator it = _attrPairList.begin(); it != _attrPairList.end(); ++it ) {
	  const TQString attr = attributeFromKey( (*it).first.upper() );
	  const TQLineEdit * le = (*it).second;
	  if ( !le )
	    continue;

	  const TQString value = le->text().stripWhiteSpace();
	  if ( value.isEmpty() )
	    continue;

	  if ( attr == "EMAIL" ) {
	    // EMAIL is special, since it shouldn't be part of the DN,
	    // except for non-RFC-conformant CAs that require it to be
	    // there.
	    email = value;
	    if ( !brokenCA->isChecked() )
	      continue;
	  }

	  if ( const char * oid = oidForAttributeName( attr ) ) {
		// we need to translate the attribute name for the backend:
              rdns.push_back( TQString::fromUtf8( oid ) + '=' + Kleo::DN::escape( value ) );
	  } else {
              rdns.push_back( attr + '=' + Kleo::DN::escape( value ) );
	  }
    }
    certParms += rdns.join(",");
    if( !email.isEmpty() )
      certParms += "\nname-email: " + email;
    certParms += "\n</GnupgKeyParms>\n";

    kdDebug() << certParms << endl;

    Kleo::KeyGenerationJob * job =
      Kleo::CryptoBackendFactory::instance()->smime()->keyGenerationJob();
    assert( job );

    connect( job, TQT_SIGNAL(result(const GpgME::KeyGenerationResult&,const TQByteArray&)),
	     TQT_SLOT(slotResult(const GpgME::KeyGenerationResult&,const TQByteArray&)) );

    certificateTE->setText( certParms );

    const GpgME::Error err = job->start( certParms );
    if ( err )
      KMessageBox::error( this,
			  i18n( "Could not start certificate generation: %1" )
			  .arg( TQString::fromLocal8Bit( err.asString() ) ),
			  i18n( "Certificate Manager Error" ) );
    else {
      generatePB->setEnabled( false );
      setBackEnabled( generatePage, false );
      (void)new Kleo::ProgressDialog( job, i18n("Generating key"), this );
    }
}


void CertificateWizardImpl::slotResult( const GpgME::KeyGenerationResult & res,
					const TQByteArray & keyData ) {
    //kdDebug() << "keyData.size(): " << keyData.size() << endl;
    _keyData = keyData;

    if ( res.error().isCanceled() || res.error() ) {
          setNextEnabled( generatePage, false );
	  setBackEnabled( generatePage, true );
          setFinishEnabled( finishPage, false );
	  generatePB->setEnabled( true );
	  if ( !res.error().isCanceled() )
	    KMessageBox::error( this,
				i18n( "Could not generate certificate: %1" )
				.arg( TQString::fromLatin1( res.error().asString() ) ),
				i18n( "Certificate Manager Error" ) );
    } else {
        // next will stay enabled until the user clicks Generate
        // Certificate again
        setNextEnabled( generatePage, true );
        setFinishEnabled( finishPage, true );
    }
}

void CertificateWizardImpl::slotHelpClicked()
{
  kapp->invokeHelp( "newcert" );
}

void CertificateWizardImpl::slotSetValuesFromWhoAmI()
{
  const KABC::Addressee a = KABC::StdAddressBook::self( true )->whoAmI();
  if ( a.isEmpty() )
    return;
  const KABC::Address adr = a.address(KABC::Address::Work);

  for ( AttrPairList::const_iterator it = _attrPairList.begin() ;
	it != _attrPairList.end() ; ++it ) {
    TQLineEdit * le = (*it).second;
    if ( !availForMod( le ) )
      continue;

    const TQString attr = attributeFromKey( (*it).first.upper() );
    if ( attr == "CN" )
      le->setText( a.formattedName() );
    else if ( attr == "EMAIL" )
      le->setText( a.preferredEmail() );
    else if ( attr == "O" )
      le->setText( a.organization() );
    else if ( attr == "OU" )
      le->setText( a.custom( "KADDRESSBOOK", "X-Department" ) );
    else if ( attr == "L" )
      le->setText( adr.locality() );
    else if ( attr == "SP" )
      le->setText( adr.region() );
    else if ( attr == "PC" )
      le->setText( adr.postalCode() );
    else if ( attr == "SN" )
      le->setText( a.familyName() );
    else if ( attr == "GN" )
      le->setText( a.givenName() );
    else if ( attr == "T" )
      le->setText( a.title() );
    else if ( attr == "BC" )
      le->setText( a.role() ); // correct mapping?
  }
}

void CertificateWizardImpl::createPersonalDataPage()
{
  TQGridLayout* grid = new TQGridLayout( edContainer, 2, 1,
				       KDialog::marginHint(), KDialog::spacingHint() );

  KConfigGroup config( KGlobal::config(), "CertificateCreationWizard" );
  TQStringList attrOrder = config.readListEntry( "DNAttributeOrder" );
  if ( attrOrder.empty() )
    attrOrder << "CN!" << "L" << "OU" << "O!" << "C!" << "EMAIL!";
  int row = 0;

  for ( TQStringList::const_iterator it = attrOrder.begin() ; it != attrOrder.end() ; ++it, ++row ) {
    const TQString key = (*it).stripWhiteSpace().upper();
    const TQString attr = attributeFromKey( key );
    if ( attr.isEmpty() ) {
      --row;
      continue;
    }
    const TQString preset = config.readEntry( attr );
    const TQString label = config.readEntry( attr + "_label",
					    attributeLabel( attr, key.endsWith("!") ) );

    TQLineEdit * le = new TQLineEdit( edContainer );
    grid->addWidget( le, row, 1 );
    grid->addWidget( new TQLabel( le, label.isEmpty() ? attr : label, edContainer ), row, 0 );

    le->setText( preset );
    if ( config.entryIsImmutable( attr ) )
      le->setEnabled( false );

    _attrPairList.append(qMakePair(key, le));

    connect( le, TQT_SIGNAL(textChanged(const TQString&)),
	     TQT_SLOT(slotEnablePersonalDataPageExit()) );
  }

  // enable button only if administrator wants to allow it
  if (KABC::StdAddressBook::self( true )->whoAmI().isEmpty() ||
      !config.readBoolEntry("ShowSetWhoAmI", true))
    insertAddressButton->setEnabled( false );

  slotEnablePersonalDataPageExit();
}

bool CertificateWizardImpl::sendToCA() const {
  return sendToCARB->isChecked();
}

TQString CertificateWizardImpl::caEMailAddress() const {
  return caEmailED->text().stripWhiteSpace();
}

void CertificateWizardImpl::slotURLSelected( const TQString& _url )
{
  KURL url = KURL::fromPathOrURL( _url.stripWhiteSpace() );
#if ! KDE_IS_VERSION(3,2,90)
  // The application/pkcs10 mimetype didn't have a native extension,
  // so the filedialog didn't have the checkbox for auto-adding it.
  TQString fileName = url.fileName();
  int pos = fileName.findRev( '.' );
  if ( pos < 0 ) // no extension
    url.setFileName( fileName + ".p10" );
#endif
  storeUR->setURL( url.prettyURL() );
}

KURL CertificateWizardImpl::saveFileUrl() const {
  return KURL::fromPathOrURL( storeUR->url().stripWhiteSpace() );
}

void CertificateWizardImpl::showPage( TQWidget * page )
{
  CertificateWizard::showPage( page );
  if ( page == generatePage ) {
    // Initial settings for the generation page: focus the correct lineedit
    // and disable the other one
    if ( storeInFileRB->isChecked() ) {
      storeUR->setEnabled( true );
      caEmailED->setEnabled( false );
      storeUR->setFocus();
    } else {
      storeUR->setEnabled( false );
      caEmailED->setEnabled( true );
      caEmailED->setFocus();
    }
  }
}

static const char* const dcopObjectId = "KMailIface";
/**
  Send the new certificate by mail using KMail
 */
void CertificateWizardImpl::sendCertificate( const TQString& email, const TQByteArray& certificateData )
{
  TQString error;
  TQCString dcopService;
  int result = KDCOPServiceStarter::self()->
    findServiceFor( "DCOP/Mailer", TQString::null,
                    TQString::null, &error, &dcopService );
  if ( result != 0 ) {
    kdDebug() << "Couldn't connect to KMail\n";
    KMessageBox::error( this,
                        i18n( "DCOP Communication Error, unable to send certificate using KMail.\n%1" ).arg( error ) );
    return;
  }

  TQCString dummy;
  // OK, so kmail (or kontact) is running. Now ensure the object we want is available.
  // [that's not the case when kontact was already running, but kmail not loaded into it... in theory.]
  if ( !kapp->dcopClient()->findObject( dcopService, dcopObjectId, "", TQByteArray(), dummy, dummy ) ) {
    DCOPRef ref( dcopService, dcopService ); // talk to the KUniqueApplication or its kontact wrapper
    DCOPReply reply = ref.call( "load()" );
    if ( reply.isValid() && (bool)reply ) {
      Q_ASSERT( kapp->dcopClient()->findObject( dcopService, dcopObjectId, "", TQByteArray(), dummy, dummy ) );
    } else
      kdWarning() << "Error loading " << dcopService << endl;
  }

  DCOPClient* dcopClient = kapp->dcopClient();
  TQByteArray data;
  TQDataStream arg( data, IO_WriteOnly );
  arg << email;
  arg << certificateData;
  if( !dcopClient->send( dcopService, dcopObjectId,
                         "sendCertificate(TQString,TQByteArray)", data ) ) {
    KMessageBox::error( this,
                        i18n( "DCOP Communication Error, unable to send certificate using KMail." ) );
    return;
  }
  // All good, close dialog
  CertificateWizard::accept();
}

// Called when pressing Finish
// We want to do the emailing/uploading first, before closing the dialog,
// in case of errors during the upload.
void CertificateWizardImpl::accept()
{
  if( sendToCA() ) {
    // Ask KMail to send this key to the CA.
    sendCertificate( caEMailAddress(), _keyData );
  } else {
    // Save in file/URL
    KURL url = saveFileUrl();
    bool overwrite = false;
    if ( KIO::NetAccess::exists( url, false /*dest*/, this ) ) {
      if ( KMessageBox::Cancel == KMessageBox::warningContinueCancel(
                                                                     this,
                                                                     i18n( "A file named \"%1\" already exists. "
                                                                           "Are you sure you want to overwrite it?" ).arg( url.prettyURL() ),
                                                                     i18n( "Overwrite File?" ),
                                                                     i18n( "&Overwrite" ) ) )
        return;
      overwrite = true;
    }

    KIO::Job* uploadJob = KIOext::put( _keyData, url, -1, overwrite, false /*resume*/ );
    uploadJob->setWindow( this );
    connect( uploadJob, TQT_SIGNAL( result( KIO::Job* ) ),
             this, TQT_SLOT( slotUploadResult( KIO::Job* ) ) );
    // Can't press finish again during the upload
    setFinishEnabled( finishPage, false );
  }
}

/**
   This slot is invoked by the KIO job used in newCertificate
   to save/upload the certificate, when finished (success or error).
*/
void CertificateWizardImpl::slotUploadResult( KIO::Job* job )
{
  if ( job->error() ) {
    job->showErrorDialog();
    setFinishEnabled( finishPage, true );
  } else {
    // All good, close dialog
    CertificateWizard::accept();
  }
}

#include "certificatewizardimpl.moc"
