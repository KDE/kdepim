/*  -*- mode: C++; c-file-style: "gnu" -*-
    certificatewizardimpl.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klarälvdalens Datakonsult AB

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

// libkleopatra
#include <kleo/oidmap.h>
#include <kleo/keygenerationjob.h>

#include <ui/progressdialog.h>

#include <cryptplugwrapper.h>
#include <cryptplugfactory.h>

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

// Qt
#include <qlineedit.h>
#include <qtextedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qlabel.h>

#include <map>
#include <assert.h>

namespace {
  struct ltstr {
    bool operator()( const char * s1, const char * s2 ) const {
      return qstrcmp( s1, s2 ) < 0 ;
    }
  };
}

std::pair<const char*,const char*> attributeLabels[] = {
#define MAKE_PAIR(x,y) std::pair<const char*,const char*>( x, y )
  MAKE_PAIR( "CN", I18N_NOOP("Common Name") ),
  MAKE_PAIR( "SN", I18N_NOOP("Surname") ),
  MAKE_PAIR( "GN", I18N_NOOP("Given Name") ),
  MAKE_PAIR( "L",  I18N_NOOP("Location") ),
  MAKE_PAIR( "T",  I18N_NOOP("Title") ),
  MAKE_PAIR( "OU", I18N_NOOP("Organizational Unit") ),
  MAKE_PAIR( "O",  I18N_NOOP("Organization") ),
  MAKE_PAIR( "PC", I18N_NOOP("Postal code") ),
  MAKE_PAIR( "C",  I18N_NOOP("Country code") ),
  MAKE_PAIR( "SP", I18N_NOOP("State or Province") ),
  MAKE_PAIR( "DC", I18N_NOOP("Domain component") ),
  MAKE_PAIR( "BC", I18N_NOOP("Business category") ),
  MAKE_PAIR( "EMAIL", I18N_NOOP("EMail Address") )
#undef MAKE_PAIR
};
static const unsigned int numAttributeLabels = sizeof attributeLabels / sizeof *attributeLabels ;

static const std::map<const char*,const char*,ltstr>
attributeLabelMap( attributeLabels, attributeLabels + numAttributeLabels );

static QString attributeLabel( const QString & attr, bool required ) {
  if ( attr.isEmpty() )
    return QString::null;
  const std::map<const char*,const char*>::const_iterator it =
    attributeLabelMap.find( attr.utf8().data() );
  if ( it != attributeLabelMap.end() && (*it).second )
    if ( required )
      return i18n("Format string for the labels in the \"Your Personal Data\" page - required field",
		  "*%1 (%2):").arg( i18n( (*it).second ) ).arg( attr );
    else
      return i18n("Format string for the labels in the \"Your Personal Data\" page",
		  "%1 (%2):").arg( i18n( (*it).second ) ).arg( attr );

  else if ( required )
    return '*' + attr + ':';
  else
    return attr + ':';
}

static const QString attributeFromKey( QString key ) {
  return key.remove( '!' );
}

static bool availForMod( const QLineEdit * le ) {
  return le && le->isEnabled();
}
    
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
    // setNextEnabled( personalDataPage, false ); // ## disable again once we have a criteria when to enable again

    createPersonalDataPage();
   
    connect( this, SIGNAL( helpClicked() ),
	     this, SLOT( slotHelpClicked() ) );
    connect( insertAddressButton, SIGNAL( clicked() ),
	     this, SLOT( slotSetValuesFromWhoAmI() ) );
}

static bool requirementsAreMet( const CertificateWizardImpl::AttrPairList & list ) {
  for ( CertificateWizardImpl::AttrPairList::const_iterator it = list.begin() ;
	it != list.end() ; ++it ) {
    const QLineEdit * le = (*it).second;
    if ( !le )
      continue;
    const QString key = (*it).first;
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

static const char * oidForAttributeName( const QString & attr ) {
  QCString attrUtf8 = attr.utf8();
  for ( unsigned int i = 0 ; i < numOidMaps ; ++i )
    if ( attrUtf8 == oidmap[i].name )
      return oidmap[i].oid;
  return 0;
}

/*
 * protected slot
 */
void CertificateWizardImpl::slotGenerateCertificate()
{
    // Ask gpgme to generate a key and return it
    QString certParms;
    certParms += "<GnupgKeyParms format=\"internal\">\n";
    certParms += "Key-Type: RSA\n";
    certParms += "Key-Length: 1024\n"; // PENDING(NN) Might want to make this user-configurable
    certParms += "Key-Usage: Sign, Encrypt\n"; // PENDING(NN) Might want to make this user-configurable
    certParms += "name-dn: ";

    QString email;
    QStringList rdns;
    for( AttrPairList::const_iterator it = _attrPairList.begin(); it != _attrPairList.end(); ++it ) {
	  const QString attr = attributeFromKey( (*it).first.upper() );
	  const QLineEdit * le = (*it).second;
	  if ( !le )
	    continue;

	  const QString value = le->text().stripWhiteSpace();
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
		rdns.push_back( QString::fromUtf8( oid ) + '=' + value );
	  } else {
		rdns.push_back( attr + '=' + value );
	  }
    }
    certParms += rdns.join(",");
    if( !email.isEmpty() )
      certParms += "\nname-email: " + email;
    certParms += "\n</GnupgKeyParms>\n";
    
    kdDebug() << certParms << endl;

    Kleo::KeyGenerationJob * job =
      Kleo::CryptPlugFactory::instance()->smime()->keyGenerationJob();
    assert( job );

    connect( job, SIGNAL(result(const GpgME::KeyGenerationResult&,const QByteArray&)),
	     SLOT(slotResult(const GpgME::KeyGenerationResult&,const QByteArray&)) );

    certificateTE->setText( certParms );

    const GpgME::Error err = job->start( certParms );
    if ( err )
      KMessageBox::error( this,
			  i18n( "Could not start certificate generation: %1" )
			  .arg( QString::fromLocal8Bit( err.asString() ) ),
			  i18n( "Cerificate Manager Error" ) );
    else
      (void)new Kleo::ProgressDialog( job, i18n("Generating key"), this );
}


void CertificateWizardImpl::slotResult( const GpgME::KeyGenerationResult & res,
					const QByteArray & keyData ) {
    kdDebug() << "keyData.size(): " << keyData.size() << endl;
    _keyData = keyData;

    if ( res.error() ) {
          setNextEnabled( generatePage, false );
          setFinishEnabled( finishPage, false );
          KMessageBox::error( this,
                              i18n( "Could not generate certificate: %1" )
			      .arg( QString::fromLatin1( res.error().asString() ) ),
                              i18n( "Certificate Manager Error" ) );
    } else {
        // next will stay enabled until the user clicks Generate
        // Certificate again
        setNextEnabled( generatePage, true );
        setFinishEnabled( finishPage, true );
	generatePB->setEnabled( false );
    }
}

void CertificateWizardImpl::slotHelpClicked()
{
  kapp->invokeHelp( "newcert" );
}

void CertificateWizardImpl::slotSetValuesFromWhoAmI()
{
  const KABC::Addressee a = KABC::StdAddressBook::self()->whoAmI();
  if ( a.isEmpty() )
    return;
  const KABC::Address adr = a.address(KABC::Address::Work);

  for ( AttrPairList::const_iterator it = _attrPairList.begin() ;
	it != _attrPairList.end() ; ++it ) {
    QLineEdit * le = (*it).second;
    if ( !availForMod( le ) )
      continue;

    const QString attr = attributeFromKey( (*it).first.upper() );
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
  QGridLayout* grid = new QGridLayout( edContainer, 2, 1,
				       KDialog::marginHint(), KDialog::spacingHint() );

  KConfigGroup config( KGlobal::config(), "CertificateCreationWizard" );
  QStringList attrOrder = config.readListEntry( "DNAttributeOrder" );
  if ( attrOrder.empty() )
    attrOrder << "CN!" << "L" << "OU" << "O!" << "C!" << "EMAIL!";
  int row = 0;

  for ( QStringList::const_iterator it = attrOrder.begin() ; it != attrOrder.end() ; ++it, ++row ) {
    const QString key = (*it).stripWhiteSpace().upper();
    const QString attr = attributeFromKey( key );
    if ( attr.isEmpty() ) {
      --row;
      continue;
    }
    const QString preset = config.readEntry( attr );
    const QString label = config.readEntry( attr + "_label",
					    attributeLabel( attr, key.endsWith("!") ) );

    QLineEdit * le = new QLineEdit( edContainer );
    grid->addWidget( le, row, 1 );
    grid->addWidget( new QLabel( le, label.isEmpty() ? attr : label, edContainer ), row, 0 );

    le->setText( preset );
    if ( config.entryIsImmutable( attr ) )
      le->setEnabled( false );

    _attrPairList.append(qMakePair(key, le));

    connect( le, SIGNAL(textChanged(const QString&)),
	     SLOT(slotEnablePersonalDataPageExit()) );
  }

  // enable button only if administrator wants to allow it
  if (KABC::StdAddressBook::self()->whoAmI().isEmpty() ||
      !config.readBoolEntry("ShowSetWhoAmI", true))
    insertAddressButton->setEnabled( false );

  slotEnablePersonalDataPageExit();
}

bool CertificateWizardImpl::sendToCA() const {
  return sendToCARB->isChecked();
}

QString CertificateWizardImpl::caEMailAddress() const {
  return caEmailED->text().stripWhiteSpace();
}

QString CertificateWizardImpl::saveFileUrl() const {
  return storeUR->url().stripWhiteSpace();
}

#include "certificatewizardimpl.moc"
