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
#include <kabc/vcardconverter.h>
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
#include <kresources/resourceselectdialog.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kxmlguiclient.h>
#include <libkdepim/categoryselectdialog.h>

#include "addresseeutil.h"
#include "addresseeeditordialog.h"
#include "details/detailsviewcontainer.h"
#include "extensionmanager.h"
#include "incsearchwidget.h"
#include "jumpbuttonbar.h"
#include "kabprefs.h"
#include "ldapsearchdialog.h"
#include "printing/printingwizard.h"
#include "undocmds.h"
#include "viewmanager.h"
#include "xxportmanager.h"

#include "kabcore.h"

KABCore::KABCore( KXMLGUIClient *client, bool readWrite, QWidget *parent,
                  const char *name )
  : QWidget( parent, name ), mGUIClient( client ), mViewManager( 0 ),
    mExtensionManager( 0 ), mConfigureDialog( 0 ), mLdapSearchDialog( 0 ),
    mReadWrite( readWrite ), mModified( false ), mConfig( 0 )
{
  mIsPart = !parent->inherits( "KAddressBookMain" );

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
           mJumpButtonBar, SLOT( recreateButtons() ) );

  connect( mDetails, SIGNAL( sendEmail( const QString& ) ),
           SLOT( sendMail( const QString& ) ) );
  connect( mDetails, SIGNAL( browse( const QString& ) ),
           SLOT( browse( const QString& ) ) );

  setModified( false );
}

KABCore::~KABCore()
{
  saveSettings();
  mAddressBook = 0;
  KABC::StdAddressBook::close();
}

void KABCore::restoreSettings()
{
  KConfigGroupSaver saver( config(), "MainWindow" );

  bool state = config()->readBoolEntry( "JumpBar", false );
  mActionJumpBar->setChecked( state );
  setJumpButtonBarVisible( state );
  
  state = config()->readBoolEntry( "Details", true );
  mActionDetails->setChecked( state );
  setDetailsVisible( state );

  QValueList<int> splitterSize;
  KConfigGroupSaver splitterSaver( config(), "Splitter" );
  splitterSize = config()->readIntListEntry( "ExtensionsSplitter" );
  if ( splitterSize.count() == 0 ) {
    splitterSize.append( width() / 2 );
    splitterSize.append( width() / 2 );
  }
  mExtensionBarSplitter->setSizes( splitterSize );

  splitterSize = config()->readIntListEntry( "DetailsSplitter" );
  if ( splitterSize.count() == 0 ) {
    splitterSize.append( height() / 2 );
    splitterSize.append( height() / 2 );
  }
  mDetailsSplitter->setSizes( splitterSize );

  mViewManager->restoreSettings();
  mExtensionManager->restoreSettings();
}

void KABCore::saveSettings()
{
  KConfigGroupSaver saver( config(), "MainWindow" );
  config()->writeEntry( "JumpBar", mActionJumpBar->isChecked() );
  config()->writeEntry( "Details", mActionDetails->isChecked() );
  config()->sync();

  KConfigGroupSaver splitterSaver( config(), "Splitter" );
  config()->writeEntry( "ExtensionsSplitter", mExtensionBarSplitter->sizes() );
  config()->writeEntry( "DetailsSplitter", mDetailsSplitter->sizes() );

  mExtensionManager->saveSettings();
  mViewManager->saveSettings();
}

KABC::AddressBook *KABCore::addressBook() const
{
  return mAddressBook;
}

KConfig *KABCore::config()
{
  static KConfig *mConfig = 0;
  if ( !mConfig )
    mConfig = new KConfig( locateLocal( "config", "kaddressbookrc" ) );
  return mConfig;
}

KActionCollection *KABCore::actionCollection() const
{
  return mGUIClient->actionCollection();
}

KABC::Field *KABCore::currentSearchField() const
{
  return mIncSearchWidget->currentField();
}

QStringList KABCore::selectedUIDs() const
{
  return mViewManager->selectedUids();
}

void KABCore::setContactSelected( const QString &uid )
{
  KABC::Addressee addr = mAddressBook->findByUid( uid );
  if ( !mDetails->isHidden() )
    mDetails->setAddressee( addr );

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
    if ( outFile.open(IO_WriteOnly) ) {  // file opened successfully
      KABC::VCardConverter converter;
      QString vcard;

      converter.addresseeToVCard( a, vcard );

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

  if ( uidList.size() > 0 ) {
    PwDeleteCommand *command = new PwDeleteCommand( mAddressBook, uidList );
    UndoStack::instance()->push( command );
    RedoStack::instance()->clear();

    // now if we deleted anything, refresh
    setContactSelected( QString::null );
    setModified( true );
  }
}

void KABCore::copyContacts()
{
  QStringList uidList = mViewManager->selectedUids();
  KABC::Addressee::List addrList;

  QStringList::Iterator it;
  for ( it = uidList.begin(); it != uidList.end(); ++it )
    addrList.append( mAddressBook->findByUid( *it ) );

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

  PwPasteCommand *command = new PwPasteCommand( this, list );
  UndoStack::instance()->push( command );
  RedoStack::instance()->clear();

  setModified( true );
}

void KABCore::setWhoAmI()
{
  QStringList uidList = mViewManager->selectedUids();

  if ( uidList.count() > 1 ) {
    KMessageBox::sorry( this, i18n( "Please select only one contact." ) );
    return;
  }

  QString text( i18n( "<qt>Do you really want to use <b>%1</b> as your new personal contact?</qt>" ) );
  KABC::Addressee addr = mAddressBook->findByUid( uidList[ 0 ] );
  if ( KMessageBox::questionYesNo( this, text.arg( addr.assembledName() ) ) == KMessageBox::Yes )
    KABC::StdAddressBook::setUsersContact( addr.uid() );
}

void KABCore::setCategories()
{
  KPIM::CategorySelectDialog dlg( KABPrefs::instance(), this, "", true );
  if ( !dlg.exec() )
    return;

  bool merge = false;
  QString msg = i18n( "Merge with existing categories?" );
  if ( KMessageBox::questionYesNo( this, msg ) == KMessageBox::Yes )
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

#if KDE_VERSION >= 319
    KABC::AddresseeList list( mAddressBook->allAddressees() );
    if (  field ) {
      list.sortByField( field );
      KABC::AddresseeList::Iterator it;
      for ( it = list.begin(); it != list.end(); ++it ) {
        if ( field->value( *it ).startsWith( text ) ) {
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
          if ( (*fieldIt)->value( *it ).startsWith( text ) ) {
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
        if ( field->value( *it ).startsWith( text ) ) {
          mViewManager->setSelected( (*it).uid(), true );
          return;
        }
      } else {
        KABC::Field::List fieldList = mIncSearchWidget->fields();
        KABC::Field::List::ConstIterator fieldIt;
        for ( fieldIt = fieldList.begin(); fieldIt != fieldList.end(); ++fieldIt ) {
          if ( (*fieldIt)->value( *it ).startsWith( text ) ) {
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

  if ( modified )
    mJumpButtonBar->recreateButtons();

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

  KRES::Resource *res = KRES::ResourceSelectDialog::getResource( kresResources, this );
  resource = static_cast<KABC::Resource*>( res );

  if ( resource ) {
    KABC::Addressee addr;
    addr.setResource( resource );
    dialog = createAddresseeEditorDialog( this );
    dialog->setAddressee( addr );
  } else
    return;

  mEditorDict.insert( dialog->addressee().uid(), dialog );

  dialog->show();
}

void KABCore::addEmail( QString aStr )
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
      dialog = createAddresseeEditorDialog( this );

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

    KMessageBox::error( this, text, i18n( "Unable to Save" ) );
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
  QRegExp r( "[/*/-]" );
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
    mConfigureDialog = new KCMultiDialog( "PIM", this );
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
    mLdapSearchDialog = new LDAPSearchDialog( mAddressBook, this );
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
  if ( !printer.setup( this ) )
    return;

  KABPrinting::PrintingWizard wizard( &printer, mAddressBook,
                                      mViewManager->selectedUids(), this );

  wizard.exec();
}

void KABCore::addGUIClient( KXMLGUIClient *client )
{
  if ( mGUIClient )
    mGUIClient->insertChildClient( client );
  else
    KMessageBox::error( this, "no KXMLGUICLient");
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
      KMessageBox::information( this, text );
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
  QHBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing( KDialogBase::spacingHint() );

  mExtensionBarSplitter = new QSplitter( this );
  mExtensionBarSplitter->setOrientation( Qt::Vertical );

  mDetailsSplitter = new QSplitter( mExtensionBarSplitter );

  QVBox *viewSpace = new QVBox( mDetailsSplitter );
  mIncSearchWidget = new IncSearchWidget( viewSpace );
  connect( mIncSearchWidget, SIGNAL( doSearch( const QString& ) ),
           SLOT( incrementalSearch( const QString& ) ) );

  mViewManager = new ViewManager( this, viewSpace );
  viewSpace->setStretchFactor( mViewManager, 1 );

  mDetails = new ViewContainer( mDetailsSplitter );

  mJumpButtonBar = new JumpButtonBar( this, this );

  mExtensionManager = new ExtensionManager( this, mExtensionBarSplitter );

  topLayout->addWidget( mExtensionBarSplitter );
  topLayout->setStretchFactor( mExtensionBarSplitter, 100 );
  topLayout->addWidget( mJumpButtonBar );
  topLayout->setStretchFactor( mJumpButtonBar, 1 );

  mXXPortManager = new XXPortManager( this, this );

  initActions();
}

void KABCore::initActions()
{
  connect( QApplication::clipboard(), SIGNAL( dataChanged() ),
           SLOT( clipboardDataChanged() ) );

  // file menu
  if ( mIsPart ) {
    mActionMail = new KAction( i18n( "&Mail" ), "mail_generic", 0, this,
                               SLOT( sendMail() ), actionCollection(),
                               "kaddressbook_mail" );
    new KAction( i18n( "&Print" ), "fileprint", CTRL + Key_P, this,
                 SLOT( print() ), actionCollection(), "kaddressbook_print" );

  } else {
    mActionMail = KStdAction::mail( this, SLOT( sendMail() ), actionCollection() );
    KStdAction::print( this, SLOT( print() ), actionCollection() );
  }

  mActionSave = new KAction( i18n( "&Save" ), "filesave", CTRL+Key_S, this,
                             SLOT( save() ), actionCollection(), "file_sync" );

  new KAction( i18n( "&New Contact..." ), "filenew", CTRL+Key_N, this,
               SLOT( newContact() ), actionCollection(), "file_new_contact" );

  mActionMailVCard = new KAction(i18n("Mail &vCard..."), "mail_post_to", 0,
                                 this, SLOT( mailVCard() ),
                                 actionCollection(), "file_mail_vcard");

  mActionEditAddressee = new KAction( i18n( "&Edit Contact..." ), "edit", 0,
                                      this, SLOT( editContact() ),
                                      actionCollection(), "file_properties" );

  // edit menu
  if ( mIsPart ) {
    mActionCopy = new KAction( i18n( "&Copy" ), "editcopy", CTRL + Key_C, this,
                               SLOT( copyContacts() ), actionCollection(),
                               "kaddressbook_copy" );
    mActionCut = new KAction( i18n( "Cu&t" ), "editcut", CTRL + Key_X, this,
                              SLOT( cutContacts() ), actionCollection(),
                              "kaddressbook_cut" );
    mActionPaste = new KAction( i18n( "&Paste" ), "editpaste", CTRL + Key_V, this,
                                SLOT( pasteContacts() ), actionCollection(),
                                "kaddressbook_paste" );
    new KAction( i18n( "Select &All" ), CTRL + Key_A, this,
                 SLOT( selectAllContacts() ), actionCollection(),
                 "kaddressbook_select_all" );
    mActionUndo = new KAction( i18n( "&Undo" ), "undo", CTRL + Key_Z, this,
                               SLOT( undo() ), actionCollection(),
                               "kaddressbook_undo" );
    mActionRedo = new KAction( i18n( "Re&do" ), "redo", CTRL + SHIFT + Key_Z,
                               this, SLOT( redo() ), actionCollection(),
                               "kaddressbook_redo" );
  } else {
    mActionCopy = KStdAction::copy( this, SLOT( copyContacts() ), actionCollection() );
    mActionCut = KStdAction::cut( this, SLOT( cutContacts() ), actionCollection() );
    mActionPaste = KStdAction::paste( this, SLOT( pasteContacts() ), actionCollection() );
    KStdAction::selectAll( this, SLOT( selectAllContacts() ), actionCollection() );
    mActionUndo = KStdAction::undo( this, SLOT( undo() ), actionCollection() );
    mActionRedo = KStdAction::redo( this, SLOT( redo() ), actionCollection() );
  }

  mActionDelete = new KAction( i18n( "&Delete Contact" ), "editdelete",
                               Key_Delete, this, SLOT( deleteContacts() ),
                               actionCollection(), "edit_delete" );

  mActionUndo->setEnabled( false );
  mActionRedo->setEnabled( false );

  // settings menu
  if ( mIsPart ) {
    new KAction( i18n( "&Configure KAddressBook..." ), "configure", 0, this,
                 SLOT( openConfigDialog() ), actionCollection(),
                 "kaddressbook_configure" );
    new KAction( i18n( "Configure S&hortcuts..." ), "configure_shortcuts", 0,
                 this, SLOT( configureKeyBindings() ), actionCollection(),
                 "kaddressbook_configure_shortcuts" );
  } else {
    KStdAction::preferences( this, SLOT( openConfigDialog() ), actionCollection() );
    KStdAction::keyBindings( this, SLOT( configureKeyBindings() ), actionCollection() );
  }

  mActionJumpBar = new KToggleAction( i18n( "Show Jump Bar" ), "next", 0,
                                      actionCollection(), "options_show_jump_bar" );
  connect( mActionJumpBar, SIGNAL( toggled( bool ) ), SLOT( setJumpButtonBarVisible( bool ) ) );
  
  mActionDetails = new KToggleAction( i18n( "Show Details" ), 0, 0,
                                      actionCollection(), "options_show_details" );
  connect( mActionDetails, SIGNAL( toggled( bool ) ), SLOT( setDetailsVisible( bool ) ) );

  // misc
  // only enable LDAP lookup if we can handle the protocol
  if ( KProtocolInfo::isKnownProtocol( KURL( "ldap://localhost" ) ) ) {
    new KAction( i18n( "&Lookup Addresses in Directory" ), "find", 0,
                 this, SLOT( openLDAPDialog() ), actionCollection(),
                 "ldap_lookup" );
  }

  mActionWhoAmI = new KAction( i18n( "Set Who Am I" ), "personal", 0, this,
                               SLOT( setWhoAmI() ), actionCollection(),
                               "set_personal" );

  mActionCategories = new KAction( i18n( "Set Categories" ), 0, this,
                                   SLOT( setCategories() ), actionCollection(),
                                   "edit_set_categories" );

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
  KKeyDialog::configure( actionCollection(), true );
}

#include "kabcore.moc"
