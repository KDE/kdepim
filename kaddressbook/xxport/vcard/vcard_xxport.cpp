/*
  This file is part of KAddressBook.
  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "vcard_xxport.h"

#include <Akonadi/Contact/ContactViewer>

#ifdef QGPGME_FOUND
#include <gpgme++/context.h>
#include <gpgme++/data.h>
#include <gpgme++/key.h>
#include <qgpgme/dataprovider.h>
#endif // QGPGME_FOUND

#include <KABC/VCardConverter>

#include <KApplication>
#include <KDebug>
#include <KDialog>
#include <KFileDialog>
#include <KLocale>
#include <KMessageBox>
#include <KPushButton>
#include <KTemporaryFile>
#include <KUrl>
#include <KIO/NetAccess>

#include <QtCore/QFile>
#include <QtCore/QPointer>
#include <QCheckBox>
#include <QFont>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>

class VCardViewerDialog : public KDialog
{
  public:
    VCardViewerDialog( const KABC::Addressee::List &list,
                       QWidget *parent );

    KABC::Addressee::List contacts() const;

  protected:
    void slotYes();
    void slotNo();
    void slotApply();
    void slotCancel();

  private:
    void updateView();

    Akonadi::ContactViewer *mView;

    KABC::Addressee::List mContacts;
    KABC::Addressee::List::Iterator mIt;
};

class VCardExportSelectionDialog : public KDialog
{
  public:
    VCardExportSelectionDialog( QWidget *parent );
    ~VCardExportSelectionDialog();

    bool exportPrivateFields() const;
    bool exportBusinessFields() const;
    bool exportOtherFields() const;
    bool exportEncryptionKeys() const;
    bool exportPictureFields() const;
    bool exportDisplayName() const;

  private:
    QCheckBox *mPrivateBox;
    QCheckBox *mBusinessBox;
    QCheckBox *mOtherBox;
    QCheckBox *mEncryptionKeys;
    QCheckBox *mPictureBox;
    QCheckBox *mDisplayNameBox;
};

VCardXXPort::VCardXXPort( QWidget *parent )
  : XXPort( parent )
{
}

bool VCardXXPort::exportContacts( const KABC::Addressee::List &contacts ) const
{
  KABC::VCardConverter converter;
  KUrl url;

  const KABC::Addressee::List list = filterContacts( contacts );
  if ( list.isEmpty() ) { // no contact selected
    return true;
  }

  bool ok = true;
  if ( list.count() == 1 ) {
    url = KFileDialog::getSaveUrl(
      QString( list[ 0 ].givenName() +
               QLatin1Char( '_' ) +
               list[ 0 ].familyName() +
               QLatin1String( ".vcf" ) ) );
    if ( url.isEmpty() ) { // user canceled export
      return true;
    }

    if ( option( "version" ) == "v21" ) {
      ok = doExport( url, converter.exportVCards( list, KABC::VCardConverter::v2_1 ) );
    } else {
      ok = doExport( url, converter.exportVCards( list, KABC::VCardConverter::v3_0 ) );
    }
  } else {
    const int answer =
      KMessageBox::questionYesNo(
        parentWidget(),
        i18nc( "@info",
               "You have selected a list of contacts, "
               "shall they be exported to several files?" ),
        QString(),
        KGuiItem( i18nc( "@action:button", "Export to Several Files" ) ),
        KGuiItem( i18nc( "@action:button", "Export to One File" ) ) );

    switch( answer ) {
    case KMessageBox::Yes:
    {
      const KUrl baseUrl = KFileDialog::getExistingDirectoryUrl();
      if ( baseUrl.isEmpty() ) {
        return true; // user canceled export
      }

      for ( int i = 0; i < list.count(); ++i ) {
        const KABC::Addressee contact = list.at( i );

        url = baseUrl.url() + '/' + contactFileName( contact ) + ".vcf";

        bool tmpOk = false;

        if ( option( "version" ) == "v21" ) {
          tmpOk = doExport( url, converter.exportVCard( contact, KABC::VCardConverter::v2_1 ) );
        } else {
          tmpOk = doExport( url, converter.exportVCard( contact, KABC::VCardConverter::v3_0 ) );
        }

        ok = ok && tmpOk;
      }
      break;
    }
    case KMessageBox::No:
    default:
    {
      url = KFileDialog::getSaveUrl( KUrl( "addressbook.vcf" ) );
      if ( url.isEmpty() ) {
        return true; // user canceled export
      }

      if ( option( "version" ) == "v21" ) {
        ok = doExport( url, converter.exportVCards( list, KABC::VCardConverter::v2_1 ) );
      } else {
        ok = doExport( url, converter.exportVCards( list, KABC::VCardConverter::v3_0 ) );
      }
    }
    }
  }

  return ok;
}

KABC::Addressee::List VCardXXPort::importContacts() const
{
  QString fileName;
  KABC::Addressee::List addrList;
  KUrl::List urls;

  if ( !option( "importData" ).isEmpty() ) {
    addrList = parseVCard( option( "importData" ).toUtf8() );
  } else {
    if ( !option( "importUrl" ).isEmpty() ) {
      urls.append( KUrl( option( "importUrl" ) ) );
    } else {
      urls =
        KFileDialog::getOpenUrls(
          KUrl(),
          "*.vcf|vCards",
          parentWidget(),
          i18nc( "@title:window", "Select vCard to Import" ) );
    }

    if ( urls.isEmpty() ) {
      return addrList;
    }

    const QString caption( i18nc( "@title:window", "vCard Import Failed" ) );
    bool anyFailures = false;

    const int numberOfUrl( urls.count() );
    for ( int i = 0; i < numberOfUrl; ++i ) {
      const KUrl url = urls.at( i );

      if ( KIO::NetAccess::download( url, fileName, parentWidget() ) ) {

        QFile file( fileName );

        if ( file.open( QIODevice::ReadOnly ) ) {
          const QByteArray data = file.readAll();
          file.close();
          if ( !data.isEmpty() ) {
            addrList += parseVCard( data );
          }

          KIO::NetAccess::removeTempFile( fileName );
        } else {
          const QString msg = i18nc(
            "@info",
            "<para>When trying to read the vCard, "
            "there was an error opening the file <filename>%1</filename>:</para>"
            "<para>%2</para>",
            url.pathOrUrl(),
            i18nc( "QFile", file.errorString().toLatin1() ) );
          KMessageBox::error( parentWidget(), msg, caption );
          anyFailures = true;
        }
      } else {
        const QString msg = i18nc(
          "@info",
          "<para>Unable to access vCard:</para><para>%1</para>",
          KIO::NetAccess::lastErrorString() );
        KMessageBox::error( parentWidget(), msg, caption );
        anyFailures = true;
      }
    }

    if ( !option( "importUrl" ).isEmpty() ) { // a vcard was passed via cmd
      if ( addrList.isEmpty() ) {
        if ( anyFailures && urls.count() > 1 ) {
          KMessageBox::information(
            parentWidget(),
            i18nc( "@info", "No contacts were imported, due to errors with the vCards." ) );
        } else if ( !anyFailures ) {
          KMessageBox::information(
            parentWidget(),
            i18nc( "@info", "The vCard does not contain any contacts." ) );
        }
      } else {
        QPointer<VCardViewerDialog> dlg = new VCardViewerDialog( addrList, parentWidget() );
        if ( dlg->exec() && dlg ) {
          addrList = dlg->contacts();
        }

        delete dlg;
      }
    }
  }

  return addrList;
}

KABC::Addressee::List VCardXXPort::parseVCard( const QByteArray &data ) const
{
  KABC::VCardConverter converter;

  return converter.parseVCards( data );
}

bool VCardXXPort::doExport( const KUrl &url, const QByteArray &data ) const
{
  if ( url.isLocalFile() && QFileInfo( url.toLocalFile() ).exists() ) {
    int answer =
      KMessageBox::questionYesNo(
        parentWidget(),
        i18nc( "@info", "Do you want to overwrite file \"%1\"", url.toLocalFile() ) );
    if ( answer == KMessageBox::No ) {
      return false;
    }
  }

  KTemporaryFile tmpFile;
  tmpFile.open();

  tmpFile.write( data );
  tmpFile.flush();

  return KIO::NetAccess::upload( tmpFile.fileName(), url, parentWidget() );
}

KABC::Addressee::List VCardXXPort::filterContacts( const KABC::Addressee::List &addrList ) const
{
  KABC::Addressee::List list;

  if ( addrList.isEmpty() ) {
    return addrList;
  }

  QPointer<VCardExportSelectionDialog> dlg = new VCardExportSelectionDialog( parentWidget() );
  if ( !dlg->exec() || !dlg ) {
    delete dlg;
    return list;
  }

  KABC::Addressee::List::ConstIterator it;
  KABC::Addressee::List::ConstIterator end( addrList.end() );
  for ( it = addrList.begin(); it != end; ++it ) {
    KABC::Addressee addr;

    addr.setUid( (*it).uid() );
    addr.setFormattedName( (*it).formattedName() );

    bool addrDone = false;
    if ( dlg->exportDisplayName() ) {		// output display name as N field
      QString fmtName = (*it).formattedName();
      QStringList splitNames = fmtName.split( ' ', QString::SkipEmptyParts );
      if ( splitNames.count() >= 2 ) {
        addr.setPrefix( QString() );
        addr.setGivenName( splitNames.takeFirst() );
        addr.setFamilyName( splitNames.takeLast() );
        addr.setAdditionalName( splitNames.join( " " ) );
        addr.setSuffix( QString() );
        addrDone = true;
      }
    }

    if ( !addrDone ) {				// not wanted, or could not be split
      addr.setPrefix( (*it).prefix() );
      addr.setGivenName( (*it).givenName() );
      addr.setAdditionalName( (*it).additionalName() );
      addr.setFamilyName( (*it).familyName() );
      addr.setSuffix( (*it).suffix() );
    }

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

    if ( dlg->exportPrivateFields() ) {
      addr.setBirthday( (*it).birthday() );
      addr.setNote( (*it).note() );
    }

    if ( dlg->exportPictureFields() ) {
      if ( dlg->exportPrivateFields() ) {
        addr.setPhoto( (*it).photo() );
      }

      if ( dlg->exportBusinessFields() ) {
        addr.setLogo( (*it).logo() );
      }
    }

    if ( dlg->exportBusinessFields() ) {
      addr.setTitle( (*it).title() );
      addr.setRole( (*it).role() );
      addr.setOrganization( (*it).organization() );
      addr.setDepartment( (*it).department() );

      KABC::PhoneNumber::List phones = (*it).phoneNumbers( KABC::PhoneNumber::Work );
      KABC::PhoneNumber::List::Iterator phoneIt;
      for ( phoneIt = phones.begin(); phoneIt != phones.end(); ++phoneIt ) {
        addr.insertPhoneNumber( *phoneIt );
      }

      KABC::Address::List addresses = (*it).addresses( KABC::Address::Work );
      KABC::Address::List::Iterator addrIt;
      for ( addrIt = addresses.begin(); addrIt != addresses.end(); ++addrIt ) {
        addr.insertAddress( *addrIt );
      }
    }

    KABC::PhoneNumber::List phones = (*it).phoneNumbers();
    KABC::PhoneNumber::List::Iterator phoneIt;
    for ( phoneIt = phones.begin(); phoneIt != phones.end(); ++phoneIt ) {
      int type = (*phoneIt).type();

      if ( type & KABC::PhoneNumber::Home && dlg->exportPrivateFields() ) {
        addr.insertPhoneNumber( *phoneIt );
      } else if ( type & KABC::PhoneNumber::Work && dlg->exportBusinessFields() ) {
        addr.insertPhoneNumber( *phoneIt );
      } else if ( dlg->exportOtherFields() ) {
        addr.insertPhoneNumber( *phoneIt );
      }
    }

    KABC::Address::List addresses = (*it).addresses();
    KABC::Address::List::Iterator addrIt;
    for ( addrIt = addresses.begin(); addrIt != addresses.end(); ++addrIt ) {
      int type = (*addrIt).type();

      if ( type & KABC::Address::Home && dlg->exportPrivateFields() ) {
        addr.insertAddress( *addrIt );
      } else if ( type & KABC::Address::Work && dlg->exportBusinessFields() ) {
        addr.insertAddress( *addrIt );
      } else if ( dlg->exportOtherFields() ) {
        addr.insertAddress( *addrIt );
      }
    }

    if ( dlg->exportOtherFields() ) {
      addr.setCustoms( (*it).customs() );
    }

    if ( dlg->exportEncryptionKeys() ) {
      addKey( addr, KABC::Key::PGP );
      addKey( addr, KABC::Key::X509 );
    }

    list.append( addr );
  }

  delete dlg;

  return list;
}

void VCardXXPort::addKey( KABC::Addressee &addr, KABC::Key::Type type ) const
{
#ifdef QGPGME_FOUND
  const QString fingerprint = addr.custom( "KADDRESSBOOK",
                                           ( type == KABC::Key::PGP ? "OPENPGPFP" : "SMIMEFP" ) );
  if ( fingerprint.isEmpty() ) {
    return;
  }

  GpgME::Context *context = GpgME::Context::createForProtocol( GpgME::OpenPGP );
  if ( !context ) {
    kError() << "No context available";
    return;
  }

  context->setArmor( false );
  context->setTextMode( false );

  QGpgME::QByteArrayDataProvider dataProvider;
  GpgME::Data dataObj( &dataProvider );
  GpgME::Error error = context->exportPublicKeys( fingerprint.toLatin1(), dataObj );
  delete context;

  if ( error ) {
    kError() << error.asString();
    return;
  }

  KABC::Key key;
  key.setType( type );
  key.setBinaryData( dataProvider.data() );

  addr.insertKey( key );
#else
  return;
#endif
}

// ---------- VCardViewer Dialog ---------------- //

VCardViewerDialog::VCardViewerDialog( const KABC::Addressee::List &list, QWidget *parent )
  : KDialog( parent ),
    mContacts( list )
{
  setCaption( i18nc( "@title:window", "Import vCard" ) );
  setButtons( Yes | No | Apply | Cancel );
  setDefaultButton( Yes );
  setModal( true );
  showButtonSeparator( true );

  QFrame *page = new QFrame( this );
  setMainWidget( page );

  QVBoxLayout *layout = new QVBoxLayout( page );
  layout->setSpacing( spacingHint() );
  layout->setMargin( marginHint() );

  QLabel *label =
    new QLabel(
      i18nc( "@info", "Do you want to import this contact into your address book?" ), page );
  QFont font = label->font();
  font.setBold( true );
  label->setFont( font );
  layout->addWidget( label );

  mView = new Akonadi::ContactViewer( page );
  layout->addWidget( mView );

  setButtonText( Apply, i18nc( "@action:button", "Import All..." ) );

  mIt = mContacts.begin();

  connect( this, SIGNAL(yesClicked()), this, SLOT(slotYes()) );
  connect( this, SIGNAL(noClicked()), this, SLOT(slotNo()) );
  connect( this, SIGNAL(applyClicked()), this, SLOT(slotApply()) );
  connect( this, SIGNAL(cancelClicked()), this, SLOT(slotCancel()) );

  updateView();
}

KABC::Addressee::List VCardViewerDialog::contacts() const
{
  return mContacts;
}

void VCardViewerDialog::updateView()
{
  mView->setRawContact( *mIt );

  KABC::Addressee::List::Iterator it = mIt;
  enableButton( Apply, ( ++it ) != mContacts.end() );
}

void VCardViewerDialog::slotYes()
{
  mIt++;

  if ( mIt == mContacts.end() ) {
    slotApply();
  }

  updateView();
}

void VCardViewerDialog::slotNo()
{
  // remove the current contact from the result set
  mIt = mContacts.erase( mIt );

  if ( mIt == mContacts.end() ) {
    slotApply();
  }

  updateView();
}

void VCardViewerDialog::slotApply()
{
  KDialog::accept();
}

void VCardViewerDialog::slotCancel()
{
  mContacts.clear();
  KDialog::accept();
}

// ---------- VCardExportSelection Dialog ---------------- //

VCardExportSelectionDialog::VCardExportSelectionDialog( QWidget *parent )
  : KDialog( parent )
{
  setCaption( i18nc( "@title:window", "Select vCard Fields" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  setModal( true );
  showButtonSeparator( true );

  QFrame *page = new QFrame( this );
  setMainWidget( page );

  QGridLayout *layout = new QGridLayout( page );
  layout->setSpacing( spacingHint() );
  layout->setMargin( marginHint() );

  QGroupBox *gbox = new QGroupBox(
    i18nc( "@title:group", "Fields to be exported" ), page );
  gbox->setFlat( true );
  layout->addWidget( gbox, 0, 0, 1, 2 );

  mPrivateBox = new QCheckBox( i18nc( "@option:check", "Private fields" ), page );
  mPrivateBox->setToolTip(
    i18nc( "@info:tooltip", "Export private fields" ) );
  mPrivateBox->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Check this box if you want to export the contact's "
           "private fields to the vCard output file." ) );
  layout->addWidget( mPrivateBox, 1, 0 );

  mBusinessBox = new QCheckBox( i18nc( "@option:check", "Business fields" ), page );
  mBusinessBox->setToolTip(
    i18nc( "@info:tooltip", "Export business fields" ) );
  mBusinessBox->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Check this box if you want to export the contact's "
           "business fields to the vCard output file." ) );
  layout->addWidget( mBusinessBox, 2, 0 );

  mOtherBox = new QCheckBox( i18nc( "@option:check", "Other fields" ), page );
  mOtherBox->setToolTip(
    i18nc( "@info:tooltip", "Export other fields" ) );
  mOtherBox->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Check this box if you want to export the contact's "
           "other fields to the vCard output file." ) );
  layout->addWidget( mOtherBox, 3, 0 );

  mEncryptionKeys = new QCheckBox( i18nc( "@option:check", "Encryption keys" ), page );
  mEncryptionKeys->setToolTip(
    i18nc( "@info:tooltip", "Export encryption keys" ) );
  mEncryptionKeys->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Check this box if you want to export the contact's "
           "encryption keys to the vCard output file." ) );
  layout->addWidget( mEncryptionKeys, 1, 1 );

  mPictureBox = new QCheckBox( i18nc( "@option:check", "Pictures" ), page );
  mPictureBox->setToolTip(
    i18nc( "@info:tooltip", "Export pictures" ) );
  mPictureBox->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Check this box if you want to export the contact's "
           "picture to the vCard output file." ) );
  layout->addWidget( mPictureBox, 2, 1 );

  gbox = new QGroupBox(
    i18nc( "@title:group", "Export options" ), page );
  gbox->setFlat( true );
  layout->addWidget( gbox, 4, 0, 1, 2 );

  mDisplayNameBox = new QCheckBox( i18nc( "@option:check", "Display name as full name" ), page );
  mDisplayNameBox->setToolTip(
    i18nc( "@info:tooltip", "Export display name as full name" ) );
  mDisplayNameBox->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Check this box if you want to export the contact's display name "
           "in the vCard's full name field.  This may be required to get the "
           "name shown correctly in GMail or Android." ) );
  layout->addWidget( mDisplayNameBox, 5, 0, 1, 2 );

  KConfig config( "kaddressbookrc" );
  const KConfigGroup group( &config, "XXPortVCard" );

  mPrivateBox->setChecked( group.readEntry( "ExportPrivateFields", true ) );
  mBusinessBox->setChecked( group.readEntry( "ExportBusinessFields", true ) );
  mOtherBox->setChecked( group.readEntry( "ExportOtherFields", true ) );
  mEncryptionKeys->setChecked( group.readEntry( "ExportEncryptionKeys", true ) );
  mPictureBox->setChecked( group.readEntry( "ExportPictureFields", true ) );
  mDisplayNameBox->setChecked( group.readEntry( "ExportDisplayName", false ) );
}

VCardExportSelectionDialog::~VCardExportSelectionDialog()
{
  KConfig config( "kaddressbookrc" );
  KConfigGroup group( &config, "XXPortVCard" );

  group.writeEntry( "ExportPrivateFields", mPrivateBox->isChecked() );
  group.writeEntry( "ExportBusinessFields", mBusinessBox->isChecked() );
  group.writeEntry( "ExportOtherFields", mOtherBox->isChecked() );
  group.writeEntry( "ExportEncryptionKeys", mEncryptionKeys->isChecked() );
  group.writeEntry( "ExportPictureFields", mPictureBox->isChecked() );
  group.writeEntry( "ExportDisplayName", mDisplayNameBox->isChecked() );
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

bool VCardExportSelectionDialog::exportPictureFields() const
{
  return mPictureBox->isChecked();
}

bool VCardExportSelectionDialog::exportDisplayName() const
{
  return mDisplayNameBox->isChecked();
}
