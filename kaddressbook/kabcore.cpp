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

#include <qclipboard.h>
#include <qdir.h>
#include <qfile.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qvbox.h>

#include <kabc/addresseelist.h>
#include <kabc/errorhandler.h>
#include <kabc/resource.h>
#include <kabc/stdaddressbook.h>
#include <kabc/vcardtool.h>
#include <kapplication.h>
#include <kactionclasses.h>
#include <kcmultidialog.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <klocale.h>
#include <kkeydialog.h>
#include <kmessagebox.h>
#include <kprinter.h>
#include <kprotocolinfo.h>
#include <kresources/selectdialog.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kxmlguiclient.h>
#include <kaboutdata.h>
#include <libkdepim/categoryselectdialog.h>

#include "addresseeutil.h"
#include "addresseeeditordialog.h"
#include "details/detailsviewcontainer.h"
#include "extensionmanager.h"
#include "filterselectionwidget.h"
#include "incsearchwidget.h"
#include "jumpbuttonbar.h"
#include "kabprefs.h"
#include "kaddressbookservice.h"
#include "ldapsearchdialog.h"
#include "printing/printingwizard.h"
#include "undocmds.h"
#include "viewmanager.h"
#include "xxportmanager.h"

#include "kabcore.h"

KABCore::KABCore( KXMLGUIClient *client, bool readWrite, QWidget *parent,
                  const char *name )
  : KAB::Core( client, parent, name ), mViewManager( 0 ),
    mExtensionManager( 0 ), mConfigureDialog( 0 ), mLdapSearchDialog( 0 ),
    mReadWrite( readWrite ), mModified( false )
{
  mWidget = new QWidget( parent, name );

  mAddressBook = KABC::StdAddressBook::self();
  KABC::StdAddressBook::setAutomaticSave( false );
  mAddressBook->setErrorHandler( new KABC::GUIErrorHandler );

  connect( mAddressBook, SIGNAL( addressBookChanged( AddressBook * ) ),
           SLOT( addressBookChanged() ) );

  mAddressBook->addCustomField( i18n( "Department" ), KABC::Field::Organization,
                                "X-Department", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Profession" ), KABC::Field::Organization,
                                "X-Profession", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Assistant's Name" ), KABC::Field::Organization,
                                "X-AssistantsName", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Manager's Name" ), KABC::Field::Organization,
                                "X-ManagersName", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Spouse's Name" ), KABC::Field::Personal,
                                "X-SpousesName", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Office" ), KABC::Field::Personal,
                                "X-Office", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "IM Address" ), KABC::Field::Personal,
                                "X-IMAddress", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Anniversary" ), KABC::Field::Personal,
                                "X-Anniversary", "KADDRESSBOOK" );

  initGUI();

  mIncSearchWidget->setFocus();

  connect( mViewManager, SIGNAL( selected( const QString& ) ),
           SLOT( setContactSelected( const QString& ) ) );
  connect( mViewManager, SIGNAL( executed( const QString& ) ),
           SLOT( editContact( const QString& ) ) );
  connect( mViewManager, SIGNAL( modified() ),
           SLOT( setModified() ) );
  connect( mViewManager, SIGNAL( urlDropped( const KURL& ) ),
           mXXPortManager, SLOT( importVCard( const KURL& ) ) );

  connect( mExtensionManager, SIGNAL( modified( const KABC::Addressee::List& ) ),
           this, SLOT( extensionModified( const KABC::Addressee::List& ) ) );

  connect( mXXPortManager, SIGNAL( modified() ),
           SLOT( setModified() ) );

  connect( mJumpButtonBar, SIGNAL( jumpToLetter( const QString& ) ),
           SLOT( incrementalSearch( const QString& ) ) );
  connect( mIncSearchWidget, SIGNAL( fieldChanged() ),
           mJumpButtonBar, SLOT( updateButtons() ) );

  connect( mDetails, SIGNAL( sendEmail( const QString& ) ),
           SLOT( sendMail( const QString& ) ) );
  connect( mDetails, SIGNAL( browse( const QString& ) ),
           SLOT( browse( const QString& ) ) );

  mAddressBookService = new KAddressBookService( this );

  setModified( false );
}

KABCore::~KABCore()
{
  saveSettings();
  KABPrefs::instance()->writeConfig();

  mAddressBook = 0;
  KABC::StdAddressBook::close();
}

void KABCore::restoreSettings()
{
  bool state = KABPrefs::instance()->mJumpButtonBarVisible;
  mActionJumpBar->setChecked( state );
  setJumpButtonBarVisible( state );
  
  state = KABPrefs::instance()->mDetailsPageVisible;
  mActionDetails->setChecked( state );
  setDetailsVisible( state );

  QValueList<int> splitterSize = KABPrefs::instance()->mExtensionsSplitter;
  if ( splitterSize.count() == 0 ) {
    splitterSize.append( mWidget->width() / 2 );
    splitterSize.append( mWidget->width() / 2 );
  }
  mExtensionBarSplitter->setSizes( splitterSize );

  splitterSize = KABPrefs::instance()->mDetailsSplitter;
  if ( splitterSize.count() == 0 ) {
    splitterSize.append( mWidget->height() / 2 );
    splitterSize.append( mWidget->height() / 2 );
  }
  mDetailsSplitter->setSizes( splitterSize );

  mViewManager->restoreSettings();
  mExtensionManager->restoreSettings();

  mIncSearchWidget->setCurrentItem( KABPrefs::instance()->mCurrentIncSearchField );
}

void KABCore::saveSettings()
{
  KABPrefs::instance()->mJumpButtonBarVisible = mActionJumpBar->isChecked();
  KABPrefs::instance()->mDetailsPageVisible = mActionDetails->isChecked();

  KABPrefs::instance()->mExtensionsSplitter = mExtensionBarSplitter->sizes();
  KABPrefs::instance()->mDetailsSplitter = mDetailsSplitter->sizes();

  mExtensionManager->saveSettings();
  mViewManager->saveSettings();

  KABPrefs::instance()->mCurrentIncSearchField = mIncSearchWidget->currentItem();
}

KABC::AddressBook *KABCore::addressBook() const
{
  return mAddressBook;
}

KConfig *KABCore::config() const
{
  return KABPrefs::instance()->config();
}

KActionCollection *KABCore::actionCollection() const
{
  return guiClient()->actionCollection();
}

KABC::Field *KABCore::currentSearchField() const
{
  return mIncSearchWidget->currentField();
}

QStringList KABCore::selectedUIDs() const
{
  return mViewManager->selectedUids();
}

KABC::Resource *KABCore::requestResource( QWidget *parent )
{
  QPtrList<KABC::Resource> kabcResources = addressBook()->resources();

  QPtrList<KRES::Resource> kresResources;
  QPtrListIterator<KABC::Resource> resIt( kabcResources );
  KABC::Resource *resource;
  while ( ( resource = resIt.current() ) != 0 ) {
    ++resIt;
    if ( !resource->readOnly() ) {
      KRES::Resource *res = static_cast<KRES::Resource*>( resource );
      if ( res )
        kresResources.append( res );
    }
  }

  KRES::Resource *res = KRES::SelectDialog::getResource( kresResources, parent );
  return static_cast<KABC::Resource*>( res );
}

QWidget *KABCore::widget() const
{
  return mWidget;
}

KAboutData *KABCore::createAboutData()
{
  KAboutData *about = new KAboutData( "kaddressbook", I18N_NOOP( "KAddressBook" ),
                                      "3.1", I18N_NOOP( "The KDE Address Book" ),
                                      KAboutData::License_GPL_V2,
                                      I18N_NOOP( "(c) 1997-2003, The KDE PIM Team" ) );
  about->addAuthor( "Tobias Koenig", I18N_NOOP( "Current maintainer" ), "tokoe@kde.org" );
  about->addAuthor( "Don Sanders", I18N_NOOP( "Original author" ) );
  about->addAuthor( "Cornelius Schumacher",
                    I18N_NOOP( "Co-maintainer, libkabc port, CSV import/export" ),
                    "schumacher@kde.org" );
  about->addAuthor( "Mike Pilone", I18N_NOOP( "GUI and framework redesign" ),
                    "mpilone@slac.com" );
  about->addAuthor( "Greg Stern", I18N_NOOP( "DCOP interface" ) );
  about->addAuthor( "Mark Westcott", I18N_NOOP( "Contact pinning" ) );
  about->addAuthor( "Mischel Boyer de la Giroday", I18N_NOOP( "LDAP Lookup" ),
                    "michel@klaralvdalens-datakonsult.se" );
  about->addAuthor( "Steffen Hansen", I18N_NOOP( "LDAP Lookup" ),
                    "hansen@kde.org" );

  return about;
}

void KABCore::setContactSelected( const QString &uid )
{
  KABC::Addressee addr = mAddressBook->findByUid( uid );
  if ( !mDetails->isHidden() )
    mDetails->setAddressee( addr );

  if ( !addr.isEmpty() ) {
    emit contactSelected( addr.formattedName() );
    KABC::Picture pic = addr.photo();
    if ( pic.isIntern() )
      emit contactSelected( pic.data() );
  }

  mExtensionManager->setSelectionChanged();

  // update the actions
  bool selected = !uid.isEmpty();

  if ( mReadWrite ) {
    mActionCut->setEnabled( selected );
    mActionPaste->setEnabled( selected );
  }

  mActionCopy->setEnabled( selected );
  mActionDelete->setEnabled( selected );
  mActionEditAddressee->setEnabled( selected );
  mActionMail->setEnabled( selected );
  mActionMailVCard->setEnabled( selected );
  mActionWhoAmI->setEnabled( selected );
  mActionCategories->setEnabled( selected );
}

void KABCore::sendMail()
{
  sendMail( mViewManager->selectedEmails().join( ", " ) );
}

void KABCore::sendMail( const QString& email )
{
  kapp->invokeMailer( email, "" );
}

void KABCore::mailVCard()
{
  QStringList uids = mViewManager->selectedUids();
  if ( !uids.isEmpty() )
    mailVCard( uids );
}

void KABCore::mailVCard( const QStringList& uids )
{
  QStringList urls;

  // Create a temp dir, so that we can put the files in it with proper names
  KTempFile tempDir;
  if ( tempDir.status() != 0 ) {
    kdWarning() << strerror( tempDir.status() ) << endl;
    return;
  }

  QString dirName = tempDir.name();
  tempDir.unlink();
  QDir().mkdir( dirName, true );

  for( QStringList::ConstIterator it = uids.begin(); it != uids.end(); ++it ) {
    KABC::Addressee a = mAddressBook->findByUid( *it );

    if ( a.isEmpty() )
      continue;

    QString name = a.givenName() + "_" + a.familyName() + ".vcf";

    QString fileName = dirName + "/" + name;

    QFile outFile(fileName);
    if ( outFile.open( IO_WriteOnly ) ) {  // file opened successfully
      KABC::VCardTool tool;
      KABC::Addressee::List list;
      list.append( a );
      QString vcard = tool.createVCards( list, KABC::VCard::v3_0 );

      QTextStream t( &outFile );  // use a text stream
      t.setEncoding( QTextStream::UnicodeUTF8 );
      t << vcard;

      outFile.close();

      urls.append( fileName );
    }
  }

  kapp->invokeMailer( QString::null, QString::null, QString::null,
                      QString::null,  // subject
                      QString::null,  // body
                      QString::null,
                      urls );  // attachments
}

void KABCore::browse( const QString& url )
{
  kapp->invokeBrowser( url );
}

void KABCore::selectAllContacts()
{
  mViewManager->setSelected( QString::null, true );
}

void KABCore::deleteContacts()
{
  QStringList uidList = mViewManager->selectedUids();

  deleteContacts( uidList );
}

void KABCore::deleteContacts( const QStringList &uids )
{
  if ( uids.count() > 0 ) {
    PwDeleteCommand *command = new PwDeleteCommand( mAddressBook, uids );
    UndoStack::instance()->push( command );
    RedoStack::instance()->clear();

    // now if we deleted anything, refresh
    setContactSelected( QString::null );
    setModified( true );
  }
}

void KABCore::copyContacts()
{
  KABC::Addressee::List addrList = mViewManager->selectedAddressees();

  QString clipText = AddresseeUtil::addresseesToClipboard( addrList );

  kdDebug(5720) << "KABCore::copyContacts: " << clipText << endl;

  QClipboard *cb = QApplication::clipboard();
  cb->setText( clipText );
}

void KABCore::cutContacts()
{
  QStringList uidList = mViewManager->selectedUids();

  if ( uidList.size() > 0 ) {
    PwCutCommand *command = new PwCutCommand( mAddressBook, uidList );
    UndoStack::instance()->push( command );
    RedoStack::instance()->clear();

    setModified( true );
  }
}

void KABCore::pasteContacts()
{
  QClipboard *cb = QApplication::clipboard();

  KABC::Addressee::List list = AddresseeUtil::clipboardToAddressees( cb->text() );

  pasteContacts( list );
}

void KABCore::pasteContacts( KABC::Addressee::List &list )
{
  KABC::Resource *resource = requestResource( mWidget );
  KABC::Addressee::List::Iterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    (*it).setResource( resource );

  PwPasteCommand *command = new PwPasteCommand( this, list );
  UndoStack::instance()->push( command );
  RedoStack::instance()->clear();

  setModified( true );
}

void KABCore::setWhoAmI()
{
  KABC::Addressee::List addrList = mViewManager->selectedAddressees();

  if ( addrList.count() > 1 ) {
    KMessageBox::sorry( mWidget, i18n( "Please select only one contact." ) );
    return;
  }

  QString text( i18n( "<qt>Do you really want to use <b>%1</b> as your new personal contact?</qt>" ) );
  if ( KMessageBox::questionYesNo( mWidget, text.arg( addrList[ 0 ].assembledName() ) ) == KMessageBox::Yes )
    static_cast<KABC::StdAddressBook*>( KABC::StdAddressBook::self() )->setWhoAmI( addrList[ 0 ] );
}

void KABCore::setCategories()
{
  KPIM::CategorySelectDialog dlg( KABPrefs::instance(), mWidget, "", true );
  if ( !dlg.exec() )
    return;

  bool merge = false;
  QString msg = i18n( "Merge with existing categories?" );
  if ( KMessageBox::questionYesNo( mWidget, msg ) == KMessageBox::Yes )
    merge = true;

  QStringList categories = dlg.selectedCategories();

  QStringList uids = mViewManager->selectedUids();
  QStringList::Iterator it;
  for ( it = uids.begin(); it != uids.end(); ++it ) {
    KABC::Addressee addr = mAddressBook->findByUid( *it );
    if ( !addr.isEmpty() ) {
      if ( !merge )
        addr.setCategories( categories );
      else {
        QStringList addrCategories = addr.categories();
        QStringList::Iterator catIt;
        for ( catIt = categories.begin(); catIt != categories.end(); ++catIt ) {
          if ( !addrCategories.contains( *catIt ) )
            addrCategories.append( *catIt );
        }
        addr.setCategories( addrCategories );
      }

      mAddressBook->insertAddressee( addr );
    }
  }

  if ( uids.count() > 0 )
    setModified( true );
}

void KABCore::setSearchFields( const KABC::Field::List &fields )
{
  mIncSearchWidget->setFields( fields );
}

void KABCore::incrementalSearch( const QString& text )
{
  mViewManager->setSelected( QString::null, false );

  if ( !text.isEmpty() ) {
    KABC::Field *field = mIncSearchWidget->currentField();

  QString pattern = text.lower();

#if KDE_VERSION >= 319
    KABC::AddresseeList list( mAddressBook->allAddressees() );
    if (  field ) {
      list.sortByField( field );
      KABC::AddresseeList::Iterator it;
      for ( it = list.begin(); it != list.end(); ++it ) {
        if ( field->value( *it ).lower().startsWith( pattern ) ) {
          mViewManager->setSelected( (*it).uid(), true );
          return;
        }
      }
    } else {
      KABC::AddresseeList::Iterator it;
      for ( it = list.begin(); it != list.end(); ++it ) {
        KABC::Field::List fieldList = mIncSearchWidget->fields();
        KABC::Field::List::ConstIterator fieldIt;
        for ( fieldIt = fieldList.begin(); fieldIt != fieldList.end(); ++fieldIt ) {
          if ( (*fieldIt)->value( *it ).lower().startsWith( pattern ) ) {
            mViewManager->setSelected( (*it).uid(), true );
            return;
          }
        }
      }
    }
#else
    KABC::AddressBook::Iterator it;
    for ( it = mAddressBook->begin(); it != mAddressBook->end(); ++it ) {
      if ( field ) {
        if ( field->value( *it ).lower().startsWith( pattern ) ) {
          mViewManager->setSelected( (*it).uid(), true );
          return;
        }
      } else {
        KABC::Field::List fieldList = mIncSearchWidget->fields();
        KABC::Field::List::ConstIterator fieldIt;
        for ( fieldIt = fieldList.begin(); fieldIt != fieldList.end(); ++fieldIt ) {
          if ( (*fieldIt)->value( *it ).lower().startsWith( pattern ) ) {
            mViewManager->setSelected( (*it).uid(), true );
            return;
          }
        }
      }
    }
#endif
  }
}

void KABCore::setModified()
{
  setModified( true );
}

void KABCore::setModified( bool modified )
{
  mModified = modified;
  mActionSave->setEnabled( mModified );

  mViewManager->refreshView();
}

bool KABCore::modified() const
{
  return mModified;
}

void KABCore::contactModified( const KABC::Addressee &addr )
{
  Command *command = 0;
  QString uid;

  // check if it exists already
  KABC::Addressee origAddr = mAddressBook->findByUid( addr.uid() );
  if ( origAddr.isEmpty() )
    command = new PwNewCommand( mAddressBook, addr );
  else {
    command = new PwEditCommand( mAddressBook, origAddr, addr );
    uid = addr.uid();
  }

  UndoStack::instance()->push( command );
  RedoStack::instance()->clear();

  setModified( true );
}

void KABCore::newContact()
{
  AddresseeEditorDialog *dialog = 0;

  QPtrList<KABC::Resource> kabcResources = mAddressBook->resources();

  QPtrList<KRES::Resource> kresResources;
  QPtrListIterator<KABC::Resource> it( kabcResources );
  KABC::Resource *resource;
  while ( ( resource = it.current() ) != 0 ) {
    ++it;
    if ( !resource->readOnly() ) {
      KRES::Resource *res = static_cast<KRES::Resource*>( resource );
      if ( res )
        kresResources.append( res );
    }
  }

  KRES::Resource *res = KRES::SelectDialog::getResource( kresResources, mWidget );
  resource = static_cast<KABC::Resource*>( res );

  if ( resource ) {
    KABC::Addressee addr;
    addr.setResource( resource );
    dialog = createAddresseeEditorDialog( mWidget );
    dialog->setAddressee( addr );
  } else
    return;

  mEditorDict.insert( dialog->addressee().uid(), dialog );

  dialog->show();
}

void KABCore::addEmail( const QString &aStr )
{
  QString fullName, email;

  KABC::Addressee::parseEmailAddress( aStr, fullName, email );

  // Try to lookup the addressee matching the email address
  bool found = false;
  QStringList emailList;
  KABC::AddressBook::Iterator it;
  for ( it = mAddressBook->begin(); !found && (it != mAddressBook->end()); ++it ) {
    emailList = (*it).emails();
    if ( emailList.contains( email ) > 0 ) {
      found = true;
      (*it).setNameFromString( fullName );
      editContact( (*it).uid() );
    }
  }

  if ( !found ) {
    KABC::Addressee addr;
    addr.setNameFromString( fullName );
    addr.insertEmail( email, true );

    mAddressBook->insertAddressee( addr );
    mViewManager->refreshView( addr.uid() );
    editContact( addr.uid() );
  }
}

void KABCore::importVCard( const KURL &url )
{
  mXXPortManager->importVCard( url );
}

void KABCore::importVCard( const QString &vCard )
{
  mXXPortManager->importVCard( vCard );
}

void KABCore::editContact( const QString &uid )
{
  if ( mExtensionManager->isQuickEditVisible() )
    return;

  // First, locate the contact entry
  QString localUID = uid;
  if ( localUID.isNull() ) {
    QStringList uidList = mViewManager->selectedUids();
    if ( uidList.count() > 0 )
      localUID = *( uidList.at( 0 ) );
  }

  KABC::Addressee addr = mAddressBook->findByUid( localUID );
  if ( !addr.isEmpty() ) {
    AddresseeEditorDialog *dialog = mEditorDict.find( addr.uid() );
    if ( !dialog ) {
      dialog = createAddresseeEditorDialog( mWidget );

      mEditorDict.insert( addr.uid(), dialog );

      dialog->setAddressee( addr );
    }

    dialog->raise();
    dialog->show();
  }
}

void KABCore::save()
{
  KABC::StdAddressBook *b = dynamic_cast<KABC::StdAddressBook*>( mAddressBook );
  if ( !b || !b->save() ) {
    QString text = i18n( "There was an error while attempting to save the "
    			"address book. Please check that some other application is "
    			"not using it. " );

    KMessageBox::error( mWidget, text, i18n( "Unable to Save" ) );
  }

  setModified( false );
}

void KABCore::undo()
{
  UndoStack::instance()->undo();

  // Refresh the view
  mViewManager->refreshView();
}

void KABCore::redo()
{
  RedoStack::instance()->redo();

  // Refresh the view
  mViewManager->refreshView();
}

void KABCore::setJumpButtonBarVisible( bool visible )
{
  if ( visible )
    mJumpButtonBar->show();
  else
    mJumpButtonBar->hide();
}

void KABCore::setDetailsVisible( bool visible )
{
  if ( visible )
    mDetails->show();
  else
    mDetails->hide();
}

void KABCore::extensionModified( const KABC::Addressee::List &list )
{
  if ( list.count() != 0 ) {
    KABC::Addressee::List::ConstIterator it;
    for ( it = list.begin(); it != list.end(); ++it )
      mAddressBook->insertAddressee( *it );

    setModified();
  }

  if ( list.count() == 0 )
    mViewManager->refreshView();
  else
    mViewManager->refreshView( list[ 0 ].uid() );
}

QString KABCore::getNameByPhone( const QString &phone )
{
  QRegExp r( "[/*/-/ ]" );
  QString localPhone( phone );

  bool found = false;
  QString ownerName = "";
  KABC::AddressBook::Iterator iter;
  KABC::PhoneNumber::List::Iterator phoneIter;
  KABC::PhoneNumber::List phoneList;
  for ( iter = mAddressBook->begin(); !found && ( iter != mAddressBook->end() ); ++iter ) {
    phoneList = (*iter).phoneNumbers();
    for ( phoneIter = phoneList.begin(); !found && ( phoneIter != phoneList.end() );
          ++phoneIter) {
      // Get rid of separator chars so just the numbers are compared.
      if ( (*phoneIter).number().replace( r, "" ) == localPhone.replace( r, "" ) ) {
        ownerName = (*iter).formattedName();
        found = true;
      }
    }
  }

  return ownerName;
}

void KABCore::openConfigDialog()
{
  if ( !mConfigureDialog ) {
    mConfigureDialog = new KCMultiDialog( "PIM", mWidget );
    mConfigureDialog->addModule( "PIM/kabconfig.desktop" );
    mConfigureDialog->addModule( "PIM/kabldapconfig.desktop" );
    connect( mConfigureDialog, SIGNAL( applyClicked() ),
             this, SLOT( configurationChanged() ) );
    connect( mConfigureDialog, SIGNAL( okClicked() ),
             this, SLOT( configurationChanged() ) );
  }

  // Save the current config so we do not loose anything if the user accepts
  saveSettings();

  mConfigureDialog->show();
  mConfigureDialog->raise();
}

void KABCore::openLDAPDialog()
{
  if ( !mLdapSearchDialog ) {
    mLdapSearchDialog = new LDAPSearchDialog( mAddressBook, mWidget );
    connect( mLdapSearchDialog, SIGNAL( addresseesAdded() ), mViewManager,
            SLOT( refreshView() ) );
    connect( mLdapSearchDialog, SIGNAL( addresseesAdded() ), this,
            SLOT( setModified() ) );
  } else
    mLdapSearchDialog->restoreSettings();

  if ( mLdapSearchDialog->isOK() )
    mLdapSearchDialog->exec();
}

void KABCore::print()
{
  KPrinter printer;
  if ( !printer.setup( mWidget, i18n("Print Addresses") ) )
    return;

  KABPrinting::PrintingWizard wizard( &printer, mAddressBook,
                                      mViewManager->selectedUids(), mWidget );

  wizard.exec();
}

void KABCore::configurationChanged()
{
  mExtensionManager->reconfigure();
}

void KABCore::addressBookChanged()
{
  QDictIterator<AddresseeEditorDialog> it( mEditorDict );
  while ( it.current() ) {
    if ( it.current()->dirty() ) {
      QString text = i18n( "Data has been changed externally. Unsaved "
                           "changes will be lost." );
      KMessageBox::information( mWidget, text );
    }
    it.current()->setAddressee( mAddressBook->findByUid( it.currentKey() ) );
    ++it;
  }

  mViewManager->refreshView();
}

AddresseeEditorDialog *KABCore::createAddresseeEditorDialog( QWidget *parent,
                                                             const char *name )
{
  AddresseeEditorDialog *dialog = new AddresseeEditorDialog( this, parent,
                                                 name ? name : "editorDialog" );
  connect( dialog, SIGNAL( contactModified( const KABC::Addressee& ) ),
           SLOT( contactModified( const KABC::Addressee& ) ) );
  connect( dialog, SIGNAL( editorDestroyed( const QString& ) ),
           SLOT( slotEditorDestroyed( const QString& ) ) );

  return dialog;
}

void KABCore::slotEditorDestroyed( const QString &uid )
{
  mEditorDict.remove( uid );
}

void KABCore::initGUI()
{
  QVBoxLayout *topLayout = new QVBoxLayout( mWidget );
  topLayout->setSpacing( KDialogBase::spacingHint() );
  QHBoxLayout *hbox = new QHBoxLayout( topLayout, KDialog::spacingHint() );

  mIncSearchWidget = new IncSearchWidget( mWidget );
  connect( mIncSearchWidget, SIGNAL( doSearch( const QString& ) ),
           SLOT( incrementalSearch( const QString& ) ) );

  mFilterSelectionWidget = new FilterSelectionWidget( mWidget );
  hbox->addWidget( mIncSearchWidget );
  hbox->addWidget( mFilterSelectionWidget );

  hbox = new QHBoxLayout( topLayout, KDialog::spacingHint() );

  mDetailsSplitter = new QSplitter( mWidget );
  hbox->addWidget( mDetailsSplitter );

  mExtensionBarSplitter = new QSplitter( mDetailsSplitter );
  mExtensionBarSplitter->setOrientation( Qt::Vertical );

  mDetails = new ViewContainer( mDetailsSplitter );

  mViewManager = new ViewManager( this, mExtensionBarSplitter );
  mViewManager->setFilterSelectionWidget( mFilterSelectionWidget );

  connect( mFilterSelectionWidget, SIGNAL( filterActivated( int ) ),
           mViewManager, SLOT( setActiveFilter( int ) ) );

  mExtensionManager = new ExtensionManager( this, mExtensionBarSplitter );

  mJumpButtonBar = new JumpButtonBar( this, mWidget );

  hbox->addWidget( mJumpButtonBar );
  hbox->setStretchFactor( mJumpButtonBar, 1 );

  topLayout->setStretchFactor( hbox, 1 );

  mXXPortManager = new XXPortManager( this, mWidget );

  initActions();
}

void KABCore::initActions()
{
  connect( QApplication::clipboard(), SIGNAL( dataChanged() ),
           SLOT( clipboardDataChanged() ) );

  KAction *action, *settingsAction;

  // file menu
  mActionMail = KStdAction::mail( this, SLOT( sendMail() ), actionCollection() );
  action = KStdAction::print( this, SLOT( print() ), actionCollection() );
  mActionMail->setWhatsThis( i18n( "Send a mail to all selected contacts." ) );
  action->setWhatsThis( i18n( "Print a special number of contacts." ) );

  mActionSave = new KAction( i18n( "&Save" ), "filesave", CTRL+Key_S, this,
                             SLOT( save() ), actionCollection(), "file_sync" );
  mActionSave->setWhatsThis( i18n( "Save all changes of the address book to the storage backend." ) );

  action = new KAction( i18n( "&New Contact..." ), "filenew", CTRL+Key_N, this,
               SLOT( newContact() ), actionCollection(), "file_new_contact" );
  action->setWhatsThis( i18n( "Create a new contact<p>You will be presented with a dialog where you can add all data of a person, including addresses and phonenumbers." ) );

  mActionMailVCard = new KAction( i18n("Send &Contact..."), "mail_post_to", 0,
                                  this, SLOT( mailVCard() ),
                                  actionCollection(), "file_mail_vcard" );
  mActionMailVCard->setWhatsThis( i18n( "Send a mail with the selected contact as attachment." ) );

  mActionEditAddressee = new KAction( i18n( "&Edit Contact..." ), "edit", 0,
                                      this, SLOT( editContact() ),
                                      actionCollection(), "file_properties" );
  mActionEditAddressee->setWhatsThis( i18n( "Edit a contact<p>You will be presented with a dialog where you can change all data of a person, including addresses and phonenumbers." ) );

  // edit menu
  mActionCopy = KStdAction::copy( this, SLOT( copyContacts() ), actionCollection() );
  mActionCut = KStdAction::cut( this, SLOT( cutContacts() ), actionCollection() );
  mActionPaste = KStdAction::paste( this, SLOT( pasteContacts() ), actionCollection() );
  action = KStdAction::selectAll( this, SLOT( selectAllContacts() ), actionCollection() );
  mActionUndo = KStdAction::undo( this, SLOT( undo() ), actionCollection() );
  mActionRedo = KStdAction::redo( this, SLOT( redo() ), actionCollection() );
  mActionCopy->setWhatsThis( i18n( "Copy the currently selected contact(s) to system clipboard in vCard format." ) );
  mActionCut->setWhatsThis( i18n( "Cuts the currently selected contact(s) to system clipboard in vCard format." ) );
  mActionPaste->setWhatsThis( i18n( "Paste the previously cut or copied contacts from clipboard." ) );
  action->setWhatsThis( i18n( "Selects all visible contacts from current view." ) );
  mActionUndo->setWhatsThis( i18n( "Undoes the last <b>Cut</b>, <b>Copy</b> or <b>Paste</b>." ) );
  mActionRedo->setWhatsThis( i18n( "Redoes the last <b>Cut</b>, <b>Copy</b> or <b>Paste</b>." ) );

  mActionDelete = new KAction( i18n( "&Delete Contact" ), "editdelete",
                               Key_Delete, this, SLOT( deleteContacts() ),
                               actionCollection(), "edit_delete" );
  mActionDelete->setWhatsThis( i18n( "Delete all selected contacts." ) );

  mActionUndo->setEnabled( false );
  mActionRedo->setEnabled( false );

  // settings menu
  action = KStdAction::preferences( this, SLOT( openConfigDialog() ), actionCollection() );
  settingsAction = KStdAction::keyBindings( this, SLOT( configureKeyBindings() ), actionCollection() );
  action->setWhatsThis( i18n( "You will be presented with a dialog, that offers you all possebilities to configure KAddressBook." ) );
  settingsAction->setWhatsThis( i18n( "You will be presented with a dialog, where you can configure the application wide shortcuts." ) );

  mActionJumpBar = new KToggleAction( i18n( "Show Jump Bar" ), "next", 0,
                                      actionCollection(), "options_show_jump_bar" );
  mActionJumpBar->setWhatsThis( i18n( "Toggle whether the jump button bar shall be visible." ) );
  connect( mActionJumpBar, SIGNAL( toggled( bool ) ), SLOT( setJumpButtonBarVisible( bool ) ) );
  
  mActionDetails = new KToggleAction( i18n( "Show Details" ), 0, 0,
                                      actionCollection(), "options_show_details" );
  mActionDetails->setWhatsThis( i18n( "Toggle whether the details page shall be visible." ) );
  connect( mActionDetails, SIGNAL( toggled( bool ) ), SLOT( setDetailsVisible( bool ) ) );

  // misc
  // only enable LDAP lookup if we can handle the protocol
  if ( KProtocolInfo::isKnownProtocol( KURL( "ldap://localhost" ) ) ) {
    action = new KAction( i18n( "&Lookup Addresses in Directory" ), "find", 0,
                          this, SLOT( openLDAPDialog() ), actionCollection(),
                          "ldap_lookup" );
    action->setWhatsThis( i18n( "Search for contacts on a LDAP server<p>You will be presented with a dialog, where you can search for contacts and select the ones you want to add to your local address book." ) );
  }

  mActionWhoAmI = new KAction( i18n( "Set Who Am I" ), "personal", 0, this,
                               SLOT( setWhoAmI() ), actionCollection(),
                               "edit_set_personal" );
  mActionWhoAmI->setWhatsThis( i18n( "Set the personal contact<p>The data of this contact will be used in many other KDE applications, so you don't have to input your personal data several times." ) );

  mActionCategories = new KAction( i18n( "Set Categories" ), 0, this,
                                   SLOT( setCategories() ), actionCollection(),
                                   "edit_set_categories" );
  mActionCategories->setWhatsThis( i18n( "Set the categories for all selected contacts." ) );

  clipboardDataChanged();

  connect( UndoStack::instance(), SIGNAL( changed() ), SLOT( updateActionMenu() ) );
  connect( RedoStack::instance(), SIGNAL( changed() ), SLOT( updateActionMenu() ) );
}

void KABCore::clipboardDataChanged()
{
  if ( mReadWrite )
    mActionPaste->setEnabled( !QApplication::clipboard()->text().isEmpty() );
}

void KABCore::updateActionMenu()
{
  UndoStack *undo = UndoStack::instance();
  RedoStack *redo = RedoStack::instance();

  if ( undo->isEmpty() )
    mActionUndo->setText( i18n( "Undo" ) );
  else
    mActionUndo->setText( i18n( "Undo %1" ).arg( undo->top()->name() ) );

  mActionUndo->setEnabled( !undo->isEmpty() );

  if ( !redo->top() )
    mActionRedo->setText( i18n( "Redo" ) );
  else
    mActionRedo->setText( i18n( "Redo %1" ).arg( redo->top()->name() ) );

  mActionRedo->setEnabled( !redo->isEmpty() );
}

void KABCore::configureKeyBindings()
{
  KKeyDialog::configure( actionCollection(), mWidget );
}

#include "kabcore.moc"
