/*
    This file is part of KAddressbook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qcheckbox.h>
#include <qfile.h>
#include <qfont.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <kabc/vcardconverter.h>
#include <kdialogbase.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kurl.h>
#include <kapplication.h>
#include <libkdepim/addresseeview.h>

#include "config.h" // ??

#include "gpgmepp/context.h"
#include "gpgmepp/data.h"
#include "gpgmepp/key.h"
#include "qgpgme/dataprovider.h"

#include "xxportmanager.h"

#include "vcard_xxport.h"

class VCardXXPortFactory : public KAB::XXPortFactory
{
  public:
    KAB::XXPort *xxportObject( KABC::AddressBook *ab, QWidget *parent, const char *name )
    {
      return new VCardXXPort( ab, parent, name );
    }
};

extern "C"
{
  void *init_libkaddrbk_vcard_xxport()
  {
    return ( new VCardXXPortFactory() );
  }
}

class VCardViewerDialog : public KDialogBase
{
  public:
    VCardViewerDialog( const KABC::Addressee::List &list,
                       QWidget *parent, const char *name = 0 );

    KABC::Addressee::List contacts() const;

  protected:
    void slotUser1();
    void slotUser2();
    void slotApply();
    void slotCancel();

  private:
    void updateView();

    KPIM::AddresseeView *mView;

    KABC::Addressee::List mContacts;
    KABC::Addressee::List::Iterator mIt;
};

class VCardExportSelectionDialog : public KDialogBase
{
  public:
    VCardExportSelectionDialog( QWidget *parent, const char *name = 0 );
    ~VCardExportSelectionDialog();

    bool exportPrivateFields() const;
    bool exportBusinessFields() const;
    bool exportOtherFields() const;
    bool exportEncryptionKeys() const;

  private:
    QCheckBox *mPrivateBox;
    QCheckBox *mBusinessBox;
    QCheckBox *mOtherBox;
    QCheckBox *mEncryptionKeys;
};

VCardXXPort::VCardXXPort( KABC::AddressBook *ab, QWidget *parent, const char *name )
  : KAB::XXPort( ab, parent, name )
{
  createImportAction( i18n( "Import vCard..." ) );
  createExportAction( i18n( "Export vCard 2.1..." ), "v21" );
  createExportAction( i18n( "Export vCard 3.0..." ), "v30" );
}

bool VCardXXPort::exportContacts( const KABC::AddresseeList &addrList, const QString &data )
{
  KABC::VCardConverter converter;
  KURL url;
  KABC::AddresseeList list;

  list = filterContacts( addrList );

  bool ok = true;
  if ( list.isEmpty() ) {
    return ok;
  } else if ( list.count() == 1 ) {
    url = KFileDialog::getSaveURL( list[ 0 ].givenName() + "_" + list[ 0 ].familyName() + ".vcf" );
    if ( url.isEmpty() )
      return true;

    if ( data == "v21" )
      ok = doExport( url, converter.createVCards( list, KABC::VCardConverter::v2_1 ) );
    else
      ok = doExport( url, converter.createVCards( list, KABC::VCardConverter::v3_0 ) );
  } else {
    QString msg = i18n( "You have selected a list of contacts, shall they be "
                        "exported to several files?" );

    switch ( KMessageBox::questionYesNo( parentWidget(), msg ) ) {
      case KMessageBox::Yes: {
        KURL baseUrl = KFileDialog::getExistingURL();
        if ( baseUrl.isEmpty() )
          return true;

        KABC::AddresseeList::ConstIterator it;
        for ( it = list.begin(); it != list.end(); ++it ) {
          url = baseUrl.url() + "/" + (*it).givenName() + "_" + (*it).familyName() + ".vcf";

          bool tmpOk;
          KABC::AddresseeList tmpList;
          tmpList.append( *it );

          if ( data == "v21" )
            tmpOk = doExport( url, converter.createVCards( tmpList, KABC::VCardConverter::v2_1 ) );
          else
            tmpOk = doExport( url, converter.createVCards( tmpList, KABC::VCardConverter::v3_0 ) );

          ok = ok && tmpOk;
        }
        break;
      }
      case KMessageBox::No:
      default: {
        url = KFileDialog::getSaveURL( "addressbook.vcf" );
        if ( url.isEmpty() )
          return true;

        if ( data == "v21" )
          ok = doExport( url, converter.createVCards( list, KABC::VCardConverter::v2_1 ) );
        else
          ok = doExport( url, converter.createVCards( list, KABC::VCardConverter::v3_0 ) );
      }
    }
  }

  return ok;
}

KABC::AddresseeList VCardXXPort::importContacts( const QString& ) const
{
  QString fileName;
  KABC::AddresseeList addrList;
  KURL::List urls;

  if ( !XXPortManager::importData.isEmpty() )
    addrList = parseVCard( XXPortManager::importData );
  else {
    if ( XXPortManager::importURL.isEmpty() )
      urls = KFileDialog::getOpenURLs( QString::null, "*.vcf|vCards", parentWidget(),
                                       i18n( "Select vCard to Import" ) );
    else
      urls.append( XXPortManager::importURL );

    if ( urls.count() == 0 )
      return addrList;

    QString caption( i18n( "vCard Import Failed" ) );
    bool anyFailures = false;
    KURL::List::Iterator it;
    for ( it = urls.begin(); it != urls.end(); ++it ) {
      if ( KIO::NetAccess::download( *it, fileName, parentWidget() ) ) {

        QFile file( fileName );

        if ( file.open( IO_ReadOnly ) ) {
          QByteArray rawData = file.readAll();
          file.close();
          if ( rawData.size() > 0 ) {
            QString data = QString::fromUtf8( rawData.data(), rawData.size() );
            addrList += parseVCard( data );
          }

          KIO::NetAccess::removeTempFile( fileName );
        } else {
          QString text = i18n( "<qt>When trying to read the vCard, there was an error opening the file '%1': %2</qt>" );
          text = text.arg( (*it).url() );
          text = text.arg( kapp->translate( "QFile",
                                            file.errorString().latin1() ) );
          KMessageBox::error( parentWidget(), text, caption );
          anyFailures = true;
        }
      } else {
        QString text = i18n( "<qt>Unable to access vCard: %1</qt>" );
        text = text.arg( KIO::NetAccess::lastErrorString() );
        KMessageBox::error( parentWidget(), text, caption );
        anyFailures = true;
      }
    }

    if ( !XXPortManager::importURL.isEmpty() ) { // a vcard was passed via cmd
      if ( addrList.isEmpty() ) {
        if ( anyFailures && urls.count() > 1 )
          KMessageBox::information( parentWidget(),
                                    i18n( "No contacts were imported, due to errors with the vcards." ) );
        else if ( !anyFailures )
          KMessageBox::information( parentWidget(), i18n( "The vCard does not contain any contacts." ) );
      } else {
        VCardViewerDialog dlg( addrList, parentWidget() );
        dlg.exec();
        addrList = dlg.contacts();
      }
    }
  }

  return addrList;
}

KABC::AddresseeList VCardXXPort::parseVCard( const QString &data ) const
{
  KABC::VCardConverter converter;

  return converter.parseVCards( data );
}

bool VCardXXPort::doExport( const KURL &url, const QString &data )
{
  KTempFile tmpFile;
  tmpFile.setAutoDelete( true );

  QTextStream stream( tmpFile.file() );
  stream.setEncoding( QTextStream::UnicodeUTF8 );

  stream << data;
  tmpFile.close();

  return KIO::NetAccess::upload( tmpFile.name(), url, parentWidget() );
}

KABC::AddresseeList VCardXXPort::filterContacts( const KABC::AddresseeList &addrList )
{
  KABC::AddresseeList list;

  if ( addrList.isEmpty() )
    return addrList;

  VCardExportSelectionDialog dlg( parentWidget() );
  if ( !dlg.exec() )
    return list;

  KABC::AddresseeList::ConstIterator it;
  for ( it = addrList.begin(); it != addrList.end(); ++it ) {
    KABC::Addressee addr;

    addr.setUid( (*it).uid() );
    addr.setFormattedName( (*it).formattedName() );
    addr.setPrefix( (*it).prefix() );
    addr.setGivenName( (*it).givenName() );
    addr.setAdditionalName( (*it).additionalName() );
    addr.setFamilyName( (*it).familyName() );
    addr.setSuffix( (*it).suffix() );
    addr.setNickName( (*it).nickName() );
    addr.setMailer( (*it).mailer() );
    addr.setTimeZone( (*it).timeZone() );
    addr.setGeo( (*it).geo() );
    addr.setProductId( (*it).productId() );
    addr.setSortString( (*it).sortString() );
    addr.setUrl( (*it).url() );
    addr.setSecrecy( (*it).secrecy() );
    addr.setSound( (*it).sound() );
    addr.setEmails( (*it).emails() );
    addr.setCategories( (*it).categories() );

    if ( dlg.exportPrivateFields() ) {
      addr.setBirthday( (*it).birthday() );
      addr.setNote( (*it).note() );
      addr.setPhoto( (*it).photo() );
    }

    if ( dlg.exportBusinessFields() ) {
      addr.setTitle( (*it).title() );
      addr.setRole( (*it).role() );
      addr.setOrganization( (*it).organization() );

      addr.setLogo( (*it).logo() );

      KABC::PhoneNumber::List phones = (*it).phoneNumbers( KABC::PhoneNumber::Work );
      KABC::PhoneNumber::List::Iterator phoneIt;
      for ( phoneIt = phones.begin(); phoneIt != phones.end(); ++phoneIt )
        addr.insertPhoneNumber( *phoneIt );

      KABC::Address::List addresses = (*it).addresses( KABC::Address::Work );
      KABC::Address::List::Iterator addrIt;
      for ( addrIt = addresses.begin(); addrIt != addresses.end(); ++addrIt )
        addr.insertAddress( *addrIt );
    }

    KABC::PhoneNumber::List phones = (*it).phoneNumbers();
    KABC::PhoneNumber::List::Iterator phoneIt;
    for ( phoneIt = phones.begin(); phoneIt != phones.end(); ++phoneIt ) {
      int type = (*phoneIt).type();

      if ( type & KABC::PhoneNumber::Home && dlg.exportPrivateFields() )
        addr.insertPhoneNumber( *phoneIt );
      else if ( type & KABC::PhoneNumber::Work && dlg.exportBusinessFields() )
        addr.insertPhoneNumber( *phoneIt );
      else if ( dlg.exportOtherFields() )
        addr.insertPhoneNumber( *phoneIt );
    }

    KABC::Address::List addresses = (*it).addresses();
    KABC::Address::List::Iterator addrIt;
    for ( addrIt = addresses.begin(); addrIt != addresses.end(); ++addrIt ) {
      int type = (*addrIt).type();

      if ( type & KABC::Address::Home && dlg.exportPrivateFields() )
        addr.insertAddress( *addrIt );
      else if ( type & KABC::Address::Work && dlg.exportBusinessFields() )
        addr.insertAddress( *addrIt );
      else if ( dlg.exportOtherFields() )
        addr.insertAddress( *addrIt );
    }

    if ( dlg.exportOtherFields() )
      addr.setCustoms( (*it).customs() );

    if ( dlg.exportEncryptionKeys() ) {
      addKey( addr, KABC::Key::PGP );
      addKey( addr, KABC::Key::X509 );
    }

    list.append( addr );
  }

  return list;
}

void VCardXXPort::addKey( KABC::Addressee &addr, KABC::Key::Types type )
{
  QString fingerprint = addr.custom( "KADDRESSBOOK",
                                     (type == KABC::Key::PGP ? "OPENPGPFP" : "SMIMEFP") );
  if ( fingerprint.isEmpty() )
    return;

  GpgME::Context * context = GpgME::Context::createForProtocol( GpgME::Context::OpenPGP );
  if ( !context ) {
    kdError() << "No context available" << endl;
    return;
  }

  context->setArmor( false );
  context->setTextMode( false );

  QGpgME::QByteArrayDataProvider dataProvider;
  GpgME::Data dataObj( &dataProvider );
  GpgME::Error error = context->exportPublicKeys( fingerprint.latin1(), dataObj );

  if ( error ) {
    kdError() << error.asString() << endl;
    return;
  }

  KABC::Key key;
  key.setType( type );
  key.setBinaryData( dataProvider.data() );

  addr.insertKey( key );
}

// ---------- VCardViewer Dialog ---------------- //

VCardViewerDialog::VCardViewerDialog( const KABC::Addressee::List &list,
                                      QWidget *parent, const char *name )
  : KDialogBase( Plain, i18n( "Import vCard" ), Yes | No | Apply | Cancel, Yes,
                 parent, name, true, true, KStdGuiItem::no(), KStdGuiItem::yes() ),
    mContacts( list )
{
  QFrame *page = plainPage();
  QVBoxLayout *layout = new QVBoxLayout( page, marginHint(), spacingHint() );

  QLabel *label = new QLabel( i18n( "Do you want to import this contact in your address book?" ), page );
  QFont font = label->font();
  font.setBold( true );
  label->setFont( font );
  layout->addWidget( label );

  mView = new KPIM::AddresseeView( page );
  mView->enableLinks( 0 );
  mView->setVScrollBarMode( QScrollView::Auto );
  layout->addWidget( mView );

  setButtonText( Apply, i18n( "Import All..." ) );

  mIt = mContacts.begin();

  updateView();
}

KABC::Addressee::List VCardViewerDialog::contacts() const
{
  return mContacts;
}

void VCardViewerDialog::updateView()
{
  mView->setAddressee( *mIt );

  KABC::Addressee::List::Iterator it = mIt;
  actionButton( Apply )->setEnabled( (++it) != mContacts.end() );
}

void VCardViewerDialog::slotUser1()
{
  mIt = mContacts.remove( mIt );

  if ( mIt == mContacts.end() )
    slotApply();

  updateView();
}

void VCardViewerDialog::slotUser2()
{
  mIt++;

  if ( mIt == mContacts.end() )
    slotApply();

  updateView();
}

void VCardViewerDialog::slotApply()
{
  QDialog::accept();
}

void VCardViewerDialog::slotCancel()
{
  mContacts.clear();
  QDialog::accept();
}

// ---------- VCardExportSelection Dialog ---------------- //

VCardExportSelectionDialog::VCardExportSelectionDialog( QWidget *parent,
                                                        const char *name )
  : KDialogBase( Plain, i18n( "Select vCard Fields" ), Ok | Cancel, Ok,
                 parent, name, true, true )
{
  QFrame *page = plainPage();

  QVBoxLayout *layout = new QVBoxLayout( page, marginHint(), spacingHint() );

  QLabel *label = new QLabel( i18n( "Select the fields which shall be exported in the vCard." ), page );
  layout->addWidget( label );

  mPrivateBox = new QCheckBox( i18n( "Private fields" ), page );
  layout->addWidget( mPrivateBox );

  mBusinessBox = new QCheckBox( i18n( "Business fields" ), page );
  layout->addWidget( mBusinessBox );

  mOtherBox = new QCheckBox( i18n( "Other fields" ), page );
  layout->addWidget( mOtherBox );

  mEncryptionKeys = new QCheckBox( i18n( "Encryption keys" ), page );
  layout->addWidget( mEncryptionKeys );

  KConfig config( "kaddressbookrc" );
  config.setGroup( "XXPortVCard" );

  mPrivateBox->setChecked( config.readBoolEntry( "ExportPrivateFields", true ) );
  mBusinessBox->setChecked( config.readBoolEntry( "ExportBusinessFields", false ) );
  mOtherBox->setChecked( config.readBoolEntry( "ExportOtherFields", false ) );
  mEncryptionKeys->setChecked( config.readBoolEntry( "ExportEncryptionKeys", false ) );
}

VCardExportSelectionDialog::~VCardExportSelectionDialog()
{
  KConfig config( "kaddressbookrc" );
  config.setGroup( "XXPortVCard" );

  config.writeEntry( "ExportPrivateFields", mPrivateBox->isChecked() );
  config.writeEntry( "ExportBusinessFields", mBusinessBox->isChecked() );
  config.writeEntry( "ExportOtherFields", mOtherBox->isChecked() );
  config.writeEntry( "ExportEncryptionKeys", mEncryptionKeys->isChecked() );
}

bool VCardExportSelectionDialog::exportPrivateFields() const
{
  return mPrivateBox->isChecked();
}

bool VCardExportSelectionDialog::exportBusinessFields() const
{
  return mBusinessBox->isChecked();
}

bool VCardExportSelectionDialog::exportOtherFields() const
{
  return mOtherBox->isChecked();
}

bool VCardExportSelectionDialog::exportEncryptionKeys() const
{
  return mEncryptionKeys->isChecked();
}

#include "vcard_xxport.moc"
