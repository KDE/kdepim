/*
    This file is part of KAddressbook.
    Copyright (c) 2003 - 2004 Tobias Koenig <tokoe@kde.org>

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
#include <qptrlist.h>
#include <qregexp.h>
#include <qvbox.h>

#include <kabc/addresseelist.h>
#include <kabc/errorhandler.h>
#include <kabc/resource.h>
#include <kabc/stdaddressbook.h>
#include <kabc/vcardconverter.h>
#include <kabc/resourcefile.h>
#include <kaboutdata.h>
#include <kaccelmanager.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <kactionclasses.h>
#include <kcmdlineargs.h>
#include <kcmultidialog.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kimproxy.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprinter.h>
#include <kprotocolinfo.h>
#include <kresources/selectdialog.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <kstdguiitem.h>
#include <ktempfile.h>
#include <kxmlguiclient.h>
#include <ktoolbar.h>
#include <libkdepim/addresseeview.h>
#include <libkdepim/categoryeditdialog.h>
#include <libkdepim/categoryselectdialog.h>

#include "addresseeutil.h"
#include "addresseeeditordialog.h"
#include "extensionmanager.h"
#include "filterselectionwidget.h"
#include "incsearchwidget.h"
#include "jumpbuttonbar.h"
#include "kablock.h"
#include "kabprefs.h"
#include "kaddressbookservice.h"
#include "kaddressbookiface.h"
#include "ldapsearchdialog.h"
#include "locationmap.h"
#include "printing/printingwizard.h"
#include "searchmanager.h"
#include "undocmds.h"
#include "viewmanager.h"
#include "xxportmanager.h"

#include "kabcore.h"

KABCore::KABCore( KXMLGUIClient *client, bool readWrite, QWidget *parent,
                  const QString &file, const char *name )
  : KAB::Core( client, parent, name ), mStatusBar( 0 ), mViewManager( 0 ),
    mExtensionManager( 0 ), mCategorySelectDialog( 0 ), mCategoryEditDialog( 0 ),
    mLdapSearchDialog( 0 ), mReadWrite( readWrite ), mModified( false )
{
  mWidget = new QWidget( parent, name );

  mIsPart = !parent->isA( "KAddressBookMain" );

  if ( file.isEmpty() ) {
    mAddressBook = KABC::StdAddressBook::self( true );
  } else {
    kdDebug(5720) << "KABCore(): document '" << file << "'" << endl;
    mAddressBook = new KABC::AddressBook;
    mAddressBook->addResource( new KABC::ResourceFile( file ) );
    if ( !mAddressBook->load() ) {
      KMessageBox::error( parent, i18n("Unable to load '%1'.").arg( file ) );
    }
  }
  mAddressBook->setErrorHandler( new KABC::GuiErrorHandler( mWidget ) );

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

  mSearchManager = new KAB::SearchManager( mAddressBook, parent );

  initGUI();

  connect( mAddressBook, SIGNAL( addressBookChanged( AddressBook* ) ),
           SLOT( addressBookChanged() ) );
  connect( mAddressBook, SIGNAL( loadingFinished( Resource* ) ),
           SLOT( addressBookChanged() ) );

  mIncSearchWidget->setFocus();

  connect( mViewManager, SIGNAL( selected( const QString& ) ),
           SLOT( setContactSelected( const QString& ) ) );
  connect( mViewManager, SIGNAL( executed( const QString& ) ),
           SLOT( editContact( const QString& ) ) );
  connect( mViewManager, SIGNAL( modified() ),
           SLOT( setModified() ) );
  connect( mViewManager, SIGNAL( urlDropped( const KURL& ) ),
           mXXPortManager, SLOT( importVCard( const KURL& ) ) );
  connect( mViewManager, SIGNAL( viewFieldsChanged() ),
           SLOT( updateIncSearchWidget() ) );
  connect( mExtensionManager, SIGNAL( modified( const KABC::Addressee::List& ) ),
           this, SLOT( extensionModified( const KABC::Addressee::List& ) ) );

  connect( mXXPortManager, SIGNAL( modified() ),
           SLOT( setModified() ) );

  connect( mJumpButtonBar, SIGNAL( jumpToLetter( const QString& ) ),
           SLOT( incrementalJumpButtonSearch( const QString& ) ) );
  connect( mViewManager, SIGNAL( sortFieldChanged() ),
           mJumpButtonBar, SLOT( updateButtons() ) );

  connect( mDetails, SIGNAL( highlightedMessage( const QString& ) ),
           SLOT( detailsHighlighted( const QString& ) ) );

  mAddressBookService = new KAddressBookService( this );

  mSearchManager->reload();

  setModified( false );

  KAcceleratorManager::manage( mWidget );

  mKIMProxy = ::KIMProxy::instance( kapp->dcopClient() );
}

KABCore::~KABCore()
{
  saveSettings();
  KABPrefs::instance()->writeConfig();

  QPtrList<KABC::Resource> resources = mAddressBook->resources();
  QPtrListIterator<KABC::Resource> it( resources );
  while ( it.current() ) {
    it.current()->close();
    ++it;
  }

  mAddressBook->disconnect();

  mAddressBook = 0;
  KABC::StdAddressBook::close();
  mKIMProxy = 0;
}

void KABCore::restoreSettings()
{
  bool state = KABPrefs::instance()->jumpButtonBarVisible();
  mActionJumpBar->setChecked( state );
  setJumpButtonBarVisible( state );

  state = KABPrefs::instance()->detailsPageVisible();
  mActionDetails->setChecked( state );
  setDetailsVisible( state );

  mViewManager->restoreSettings();
  mExtensionManager->restoreSettings();

  updateIncSearchWidget();
  mIncSearchWidget->setCurrentItem( KABPrefs::instance()->currentIncSearchField() );

  QValueList<int> splitterSize = KABPrefs::instance()->extensionsSplitter();
  if ( splitterSize.count() == 0 ) {
    splitterSize.append( mDetailsSplitter->height() / 2 );
    splitterSize.append( mDetailsSplitter->height() / 2 );
  }
  mExtensionBarSplitter->setSizes( splitterSize );

  splitterSize = KABPrefs::instance()->detailsSplitter();
  if ( splitterSize.count() == 0 ) {
    splitterSize.append( 360 );
    splitterSize.append( 260 );
  }
  mDetailsSplitter->setSizes( splitterSize );

}

void KABCore::saveSettings()
{
  KABPrefs::instance()->setJumpButtonBarVisible( mActionJumpBar->isChecked() );
  KABPrefs::instance()->setDetailsPageVisible( mActionDetails->isChecked() );

  KABPrefs::instance()->setExtensionsSplitter( mExtensionBarSplitter->sizes() );
  KABPrefs::instance()->setDetailsSplitter( mDetailsSplitter->sizes() );

  mExtensionManager->saveSettings();
  mViewManager->saveSettings();

  KABPrefs::instance()->setCurrentIncSearchField( mIncSearchWidget->currentItem() );
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

KABC::Field *KABCore::currentSortField() const
{
  return mViewManager->currentSortField();
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
                                      "3.3", I18N_NOOP( "The KDE Address Book" ),
                                      KAboutData::License_GPL_V2,
                                      I18N_NOOP( "(c) 1997-2004, The KDE PIM Team" ) );
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

void KABCore::setStatusBar( KStatusBar *statusBar )
{
  mStatusBar = statusBar;
}

KStatusBar *KABCore::statusBar() const
{
  return mStatusBar;
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
  mActionStoreAddresseeIn->setEnabled( selected );
  mActionMail->setEnabled( selected );
  mActionMailVCard->setEnabled( selected );
  mActionChat->setEnabled( selected );
  mActionWhoAmI->setEnabled( selected );
  mActionCategories->setEnabled( selected );
  mActionMerge->setEnabled( selected );
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
  //QStringList urls;
  KURL::List urls;

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

    QString name = a.givenName().utf8() + "_" + a.familyName().utf8() + ".vcf";

    QString fileName = dirName + "/" + name;

    QFile outFile(fileName);
    if ( outFile.open( IO_WriteOnly ) ) {  // file opened successfully
      KABC::VCardConverter converter;
      KABC::Addressee::List list;
      list.append( a );
      QString vcard = converter.createVCards( list, KABC::VCardConverter::v3_0 );

      QTextStream t( &outFile );  // use a text stream
      t.setEncoding( QTextStream::UnicodeUTF8 );
      t << vcard;

      outFile.close();

      KURL url( fileName );
      url.setFileEncoding( "UTF-8" );
      urls.append( url );
    }
  }
  kapp->invokeMailer( QString::null, QString::null, QString::null,
                      QString::null,  // subject
                      QString::null,  // body
                      QString::null,
                      urls.toStringList() );  // attachments
}

void KABCore::startChat()
{
  QStringList uids = mViewManager->selectedUids();
  if ( !uids.isEmpty() )
    mKIMProxy->chatWithContact( uids.first() );
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
    QStringList names;
    QStringList::ConstIterator it = uids.begin();
    while ( it != uids.end() ) {
      KABC::Addressee addr = mAddressBook->findByUid( *it );
      names.append( addr.realName().isEmpty() ? addr.preferredEmail() : addr.realName() );
      ++it;
    }

    if ( KMessageBox::warningContinueCancelList( mWidget, i18n( "Do you really want to delete these contacts?" ),
                                         names, "", KGuiItem( i18n("&Delete"), "editdelete") ) == KMessageBox::Cancel )
      return;

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

void KABCore::mergeContacts()
{
  KABC::Addressee::List list = mViewManager->selectedAddressees();
  if ( list.count() < 2 )
    return;

  KABC::Addressee addr = mergeContacts( list );

  KABC::Addressee::List::Iterator it = list.begin();
  ++it;
  while ( it != list.end() ) {
    mAddressBook->removeAddressee( *it );
    ++it;
  }

  mAddressBook->insertAddressee( addr );

  mSearchManager->reload();
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
    static_cast<KABC::StdAddressBook*>( KABC::StdAddressBook::self( true ) )->setWhoAmI( addrList[ 0 ] );
}

void KABCore::incrementalTextSearch( const QString& text )
{
  setContactSelected( QString::null );
  mSearchManager->search( text, mIncSearchWidget->currentFields() );
}

void KABCore::incrementalJumpButtonSearch( const QString& character )
{
  mViewManager->setSelected( QString::null, false );

  KABC::AddresseeList list = mSearchManager->contacts();
  KABC::Field *field = mViewManager->currentSortField();
  if ( field ) {
    list.sortByField( field );
    KABC::AddresseeList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
      if ( field->value( *it ).startsWith( character, false ) ) {
        mViewManager->setSelected( (*it).uid(), true );
        return;
      }
    }
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

  mSearchManager->reload();
}

bool KABCore::modified() const
{
  return mModified;
}

void KABCore::contactModified( const KABC::Addressee &addr )
{
  Command *command = 0;

  // check if it exists already
  KABC::Addressee origAddr = mAddressBook->findByUid( addr.uid() );
  if ( origAddr.isEmpty() ) {
    command = new PwNewCommand( mAddressBook, addr );
  } else {
    command = new PwEditCommand( mAddressBook, origAddr, addr );
  }

  UndoStack::instance()->push( command );
  RedoStack::instance()->clear();

  setContactSelected( addr.uid() );
  setModified( true );
}

void KABCore::newContact()
{
  AddresseeEditorDialog *dialog = 0;

  KABC::Resource* resource = requestResource( mWidget );

  if ( resource ) {
    KABC::Addressee addr;
    addr.setResource( resource );

    if ( !KABLock::self( mAddressBook )->lock( addr.resource() ) )
      return;

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

void KABCore::importVCard( const QString &vCardURL )
{
  mXXPortManager->importVCard( vCardURL );
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

      if ( !addr.resource()->readOnly() )
        if ( !KABLock::self( mAddressBook )->lock( addr.resource() ) ) {
          return;
        }

      dialog = createAddresseeEditorDialog( mWidget );

      mEditorDict.insert( addr.uid(), dialog );

      dialog->setAddressee( addr );
    }

    dialog->raise();
    dialog->show();
  }
}

void KABCore::storeContactIn( const QString &uid )
{
  // First, locate the contact entry
  QStringList uidList;
  if ( uid.isNull() ) {
    uidList = mViewManager->selectedUids();
  } else {
    uidList << uid;
  }
  KABC::Resource *resource = requestResource( mWidget );
  if ( !resource ) return;
  KABLock::self( mAddressBook )->lock( resource );
  QStringList::Iterator it( uidList.begin() );
  while ( it != uidList.end() ) {
    KABC::Addressee addr = mAddressBook->findByUid( *it++ );
    if ( !addr.isEmpty() ) {
      KABC::Addressee newAddr( addr );
      // We need to set a new uid, otherwise the insert below is
      // ignored. This is bad for syncing, but unavoidable, afaiks
      newAddr.setUid( KApplication::randomString( 10 ) );
      newAddr.setResource( resource );
      addressBook()->insertAddressee( newAddr );
      KABLock::self( mAddressBook )->lock( addr.resource() );
      addressBook()->removeAddressee( addr );
      KABLock::self( mAddressBook )->unlock( addr.resource() );
    }
  }
  KABLock::self( mAddressBook )->unlock( resource );
}

void KABCore::save()
{
  QPtrList<KABC::Resource> resources = mAddressBook->resources();
  QPtrListIterator<KABC::Resource> it( resources );
  while ( it.current() && !it.current()->readOnly() ) {
    KABC::Ticket *ticket = mAddressBook->requestSaveTicket( it.current() );
    if ( ticket ) {
      if ( !mAddressBook->save( ticket ) ) {
        KMessageBox::error( mWidget,
                            i18n( "<qt>Unable to save address book <b>%1</b></qt>." ).arg( it.current()->resourceName() ) );
        mAddressBook->releaseSaveTicket( ticket );
      } else {
        setModified( false );
      }
    } else {
      KMessageBox::error( mWidget,
                          i18n( "<qt>Unable to get access for saving the address book <b>%1</b></qt>." )
                          .arg( it.current()->resourceName() ) );
    }

    ++it;
  }
/*
  KABC::StdAddressBook *b = dynamic_cast<KABC::StdAddressBook*>( mAddressBook );
  if ( b ) {
    if ( !b->save() ) {
      QString text = i18n( "There was an error while attempting to save the "
    	                   "address book. Please check that no other application "
                           "is using it." );

      KMessageBox::error( mWidget, text, i18n( "Unable to Save" ) );
    } else {
      setModified( false );
    }
  } else {
    // FIXME: Handle locking properly, i.e. get the ticket before doing the
    // first change to the addressbook and don't give up the ticket in case of a
    // save error without asking the user.
    KABC::Ticket *ticket = mAddressBook->requestSaveTicket();
    if ( ticket ) {
      if ( !mAddressBook->save( ticket ) ) {
        KMessageBox::error( mWidget, i18n("Error saving address book.") );
        mAddressBook->releaseSaveTicket( ticket );
      } else {
        setModified( false );
      }
    } else {
      KMessageBox::error( mWidget,
                          i18n("Unable to get access for saving the address "
                               "book.") );
    }
  }
*/
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
    mDetailsPage->show();
  else
    mDetailsPage->hide();
}

void KABCore::extensionModified( const KABC::Addressee::List &list )
{
  if ( list.count() != 0 ) {
    KABC::Addressee::List::ConstIterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
      Command *command = 0;

      // check if it exists already
      KABC::Addressee origAddr = mAddressBook->findByUid( (*it).uid() );
      if ( origAddr.isEmpty() )
        command = new PwNewCommand( mAddressBook, *it );
      else
        command = new PwEditCommand( mAddressBook, origAddr, *it );

      UndoStack::instance()->push( command );
      RedoStack::instance()->clear();
    }

    setModified( true );
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
        ownerName = (*iter).realName();
        found = true;
      }
    }
  }

  return ownerName;
}

void KABCore::openLDAPDialog()
{
  if ( !KProtocolInfo::isKnownProtocol( KURL( "ldap://localhost" ) ) ) {
    KMessageBox::error( mWidget, i18n( "Your KDE installation is missing LDAP "
                                       "support, please ask your administrator or distributor for more information." ),
                        i18n( "No LDAP IO Slave Available" ) );
    return;
  }

  if ( !mLdapSearchDialog ) {
    mLdapSearchDialog = new LDAPSearchDialog( mAddressBook, this, mWidget );
    connect( mLdapSearchDialog, SIGNAL( addresseesAdded() ), mSearchManager,
            SLOT( addressBookChanged() ) );
    connect( mLdapSearchDialog, SIGNAL( addresseesAdded() ),
            SLOT( setModified() ) );
  } else
    mLdapSearchDialog->restoreSettings();

  if ( mLdapSearchDialog->isOK() )
    mLdapSearchDialog->exec();
}

void KABCore::configure()
{
  // Save the current config so we do not loose anything if the user accepts
  saveSettings();

  KCMultiDialog dlg( mWidget, "", true );
  connect( &dlg, SIGNAL( configCommitted() ),
           this, SLOT( configurationChanged() ) );

  dlg.addModule( "kabconfig.desktop" );
  dlg.addModule( "kabldapconfig.desktop" );
  dlg.addModule( "kabcustomfields.desktop" );

  dlg.exec();
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

void KABCore::detailsHighlighted( const QString &msg )
{
  if ( mStatusBar )
    mStatusBar->changeItem( msg, 1 );
}

void KABCore::showContactsAddress( const QString &addrUid )
{
  QStringList uidList = mViewManager->selectedUids();
  if ( uidList.isEmpty() )
    return;

  KABC::Addressee addr = mAddressBook->findByUid( uidList.first() );
  if ( addr.isEmpty() )
    return;

  KABC::Address::List list = addr.addresses();
  KABC::Address::List::Iterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    if ( (*it).id() == addrUid ) {
      LocationMap::instance()->showAddress( *it );
      break;
    }
}

void KABCore::configurationChanged()
{
  mExtensionManager->reconfigure();
  mViewManager->refreshView();
}

void KABCore::addressBookChanged()
{
  mJumpButtonBar->updateButtons();
  mSearchManager->reload();
  mViewManager->setSelected( QString::null, false );
  setContactSelected( QString::null );
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
  AddresseeEditorDialog *dialog = mEditorDict.take( uid );

  KABC::Addressee addr = dialog->addressee();

  if ( !addr.resource()->readOnly() ) {
    QApplication::setOverrideCursor( Qt::waitCursor );
    KABLock::self( mAddressBook )->unlock( addr.resource() );
    QApplication::restoreOverrideCursor();
  }
}

void KABCore::initGUI()
{
  QVBoxLayout *topLayout = new QVBoxLayout( mWidget, 0, 0 );
  KToolBar* searchTB = new KToolBar( mWidget, "search toolbar");
  searchTB->boxLayout()->setSpacing( KDialog::spacingHint() );
  mIncSearchWidget = new IncSearchWidget( searchTB, "kde toolbar widget");
  searchTB->setStretchableWidget( mIncSearchWidget );
  connect( mIncSearchWidget, SIGNAL( doSearch( const QString& ) ),
           SLOT( incrementalTextSearch( const QString& ) ) );

  mFilterSelectionWidget = new FilterSelectionWidget( searchTB , "kde toolbar widget" );

  QHBoxLayout *hbox = new QHBoxLayout( mWidget, 0, 0 );

  mDetailsSplitter = new QSplitter( mWidget );
  hbox->addWidget( mDetailsSplitter );

  topLayout->addWidget( searchTB );
  topLayout->addWidget( mDetailsSplitter );

  mExtensionBarSplitter = new QSplitter( mDetailsSplitter );
  mExtensionBarSplitter->setOrientation( Qt::Vertical );

  QWidget *detailsWidget = new QWidget( mDetailsSplitter );
  QHBoxLayout *detailsLayout = new QHBoxLayout( detailsWidget );

  mDetailsPage = new QWidget( detailsWidget );
  detailsLayout->addWidget( mDetailsPage );

  QHBoxLayout *detailsPageLayout = new QHBoxLayout( mDetailsPage, 0, 0 );
  mDetails = new KPIM::AddresseeView( mDetailsPage );
  detailsPageLayout->addWidget( mDetails );

  connect( mDetails, SIGNAL( addressClicked( const QString&) ),
           this, SLOT( showContactsAddress( const QString& ) ) );

  mViewManager = new ViewManager( this, mExtensionBarSplitter );
  mViewManager->setFilterSelectionWidget( mFilterSelectionWidget );

  connect( mFilterSelectionWidget, SIGNAL( filterActivated( int ) ),
           mViewManager, SLOT( setActiveFilter( int ) ) );

  mExtensionManager = new ExtensionManager( this, mExtensionBarSplitter );

  mJumpButtonBar = new JumpButtonBar( this, detailsWidget );
  detailsLayout->addWidget( mJumpButtonBar );
  detailsLayout->setStretchFactor( mJumpButtonBar, 1 );

  topLayout->setStretchFactor( hbox, 1 );

  mXXPortManager = new XXPortManager( this, mWidget );

  initActions();
}

void KABCore::initActions()
{
  connect( QApplication::clipboard(), SIGNAL( dataChanged() ),
           SLOT( clipboardDataChanged() ) );

  KAction *action;

  // file menu
  mActionMail = new KAction( i18n( "&Send Email to Contact..." ), "mail_send", 0,
                             this, SLOT( sendMail() ), actionCollection(), "file_mail" );
  action = KStdAction::print( this, SLOT( print() ), actionCollection() );
  mActionMail->setWhatsThis( i18n( "Send a mail to all selected contacts." ) );
  action->setWhatsThis( i18n( "Print a special number of contacts." ) );

  mActionSave = KStdAction::save( this,
                             SLOT( save() ), actionCollection(), "file_sync" );
  mActionSave->setWhatsThis( i18n( "Save all changes of the address book to the storage backend." ) );

  action = new KAction( i18n( "&New Contact..." ), "identity", CTRL+Key_N, this,
               SLOT( newContact() ), actionCollection(), "file_new_contact" );
  action->setWhatsThis( i18n( "Create a new contact<p>You will be presented with a dialog where you can add all data about a person, including addresses and phone numbers." ) );

  mActionMailVCard = new KAction( i18n("Send &Contact..."), "mail_post_to", 0,
                                  this, SLOT( mailVCard() ),
                                  actionCollection(), "file_mail_vcard" );
  mActionMailVCard->setWhatsThis( i18n( "Send a mail with the selected contact as attachment." ) );

  mActionChat = new KAction( i18n("Chat &With..."), 0,
                                  this, SLOT( startChat() ),
                                  actionCollection(), "file_chat" );
  mActionChat->setWhatsThis( i18n( "Start a chat with the selected contact." ) );

  mActionEditAddressee = new KAction( i18n( "&Edit Contact..." ), "edit", 0,
                                      this, SLOT( editContact() ),
                                      actionCollection(), "file_properties" );
  mActionEditAddressee->setWhatsThis( i18n( "Edit a contact<p>You will be presented with a dialog where you can change all data about a person, including addresses and phone numbers." ) );

  mActionMerge = new KAction( i18n( "&Merge Contacts" ), "", 0,
                              this, SLOT( mergeContacts() ),
                              actionCollection(), "edit_merge" );

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


  mActionStoreAddresseeIn = new KAction( i18n( "St&ore Contact In..." ), "", 0,
                                      this, SLOT( storeContactIn() ),
                                      actionCollection(), "edit_store_in" );
  mActionStoreAddresseeIn->setWhatsThis( i18n( "Store a contact in a different Addressbook<p>You will be presented with a dialog where you can select a new storage place for this contact." ) );


  mActionUndo->setEnabled( false );
  mActionRedo->setEnabled( false );

  // settings menu
  mActionJumpBar = new KToggleAction( i18n( "Show Jump Bar" ), "next", 0,
                                      actionCollection(), "options_show_jump_bar" );
  mActionJumpBar->setWhatsThis( i18n( "Toggle whether the jump button bar shall be visible." ) );
  mActionJumpBar->setCheckedState( i18n( "Hide Jump Bar") );
  connect( mActionJumpBar, SIGNAL( toggled( bool ) ), SLOT( setJumpButtonBarVisible( bool ) ) );

  mActionDetails = new KToggleAction( i18n( "Show Details" ), 0, 0,
                                      actionCollection(), "options_show_details" );
  mActionDetails->setWhatsThis( i18n( "Toggle whether the details page shall be visible." ) );
  mActionDetails->setCheckedState( i18n( "Hide Details") );
  connect( mActionDetails, SIGNAL( toggled( bool ) ), SLOT( setDetailsVisible( bool ) ) );

  if ( mIsPart )
    action = new KAction( i18n( "&Configure KAddressBook..." ), "configure", 0,
                          this, SLOT( configure() ), actionCollection(),
                          "kaddressbook_configure" );
  else
    action = KStdAction::preferences( this, SLOT( configure() ), actionCollection() );

  action->setWhatsThis( i18n( "You will be presented with a dialog, that offers you all possibilities to configure KAddressBook." ) );

  // misc
  action = new KAction( i18n( "&Lookup Addresses in LDAP Directory..." ), "find", 0,
                        this, SLOT( openLDAPDialog() ), actionCollection(), "ldap_lookup" );
  action->setWhatsThis( i18n( "Search for contacts on a LDAP server<p>You will be presented with a dialog, where you can search for contacts and select the ones you want to add to your local address book." ) );

  mActionWhoAmI = new KAction( i18n( "Set as Personal Contact Data" ), "personal", 0, this,
                               SLOT( setWhoAmI() ), actionCollection(),
                               "edit_set_personal" );
  mActionWhoAmI->setWhatsThis( i18n( "Set the personal contact<p>The data of this contact will be used in many other KDE applications, so you do not have to input your personal data several times." ) );

  mActionCategories = new KAction( i18n( "Select Categories..." ), 0, this,
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

void KABCore::updateIncSearchWidget()
{
  mIncSearchWidget->setViewFields( mViewManager->viewFields() );
}

KABC::Addressee KABCore::mergeContacts( const KABC::Addressee::List &list )
{
  if ( list.count() == 0 ) //emtpy
    return KABC::Addressee();
  else if ( list.count() == 1 ) // nothing to merge
    return list.first();

  KABC::Addressee masterAddressee = list.first();

  KABC::Addressee::List::ConstIterator contactIt = list.begin();
  for ( ++contactIt; contactIt != list.end(); ++contactIt ) {
    // ADR + LABEL
    KABC::Address::List addresses = (*contactIt).addresses();
    KABC::Address::List masterAddresses = masterAddressee.addresses();
    KABC::Address::List::Iterator addrIt ;
    for ( addrIt = addresses.begin(); addrIt != addresses.end(); ++addrIt ) {
      if ( !masterAddresses.contains( *addrIt ) )
        masterAddressee.insertAddress( *addrIt );
    }

    if ( masterAddressee.birthday().isNull() && !(*contactIt).birthday().isNull() )
      masterAddressee.setBirthday( (*contactIt).birthday() );


    // CATEGORIES
    QStringList::Iterator it;
    QStringList categories = (*contactIt).categories();
    QStringList masterCategories = masterAddressee.categories();
    QStringList newCategories( masterCategories );
    for ( it = categories.begin(); it != categories.end(); ++it )
      if ( !masterCategories.contains( *it ) )
        newCategories.append( *it );
    masterAddressee.setCategories( newCategories );

    // CLASS
    if ( !masterAddressee.secrecy().isValid() && (*contactIt).secrecy().isValid() )
      masterAddressee.setSecrecy( (*contactIt).secrecy() );

    // EMAIL
    QStringList emails = (*contactIt).emails();
    QStringList masterEmails = masterAddressee.emails();
    for ( it = emails.begin(); it != emails.end(); ++it )
      if ( !masterEmails.contains( *it ) )
        masterAddressee.insertEmail( *it, false );

    // FN
    if ( masterAddressee.formattedName().isEmpty() && !(*contactIt).formattedName().isEmpty() )
      masterAddressee.setFormattedName( (*contactIt).formattedName() );

    // GEO
    if ( !masterAddressee.geo().isValid() && (*contactIt).geo().isValid() )
      masterAddressee.setGeo( (*contactIt).geo() );

/*
  // KEY
  // LOGO
*/

    // MAILER
    if ( masterAddressee.mailer().isEmpty() && !(*contactIt).mailer().isEmpty() )
      masterAddressee.setMailer( (*contactIt).mailer() );

    // N
    if ( masterAddressee.assembledName().isEmpty() && !(*contactIt).assembledName().isEmpty() )
      masterAddressee.setNameFromString( (*contactIt).assembledName() );

    // NICKNAME
    if ( masterAddressee.nickName().isEmpty() && !(*contactIt).nickName().isEmpty() )
      masterAddressee.setNickName( (*contactIt).nickName() );

    // NOTE
    if ( masterAddressee.note().isEmpty() && !(*contactIt).note().isEmpty() )
      masterAddressee.setNote( (*contactIt).note() );

    // ORG
    if ( masterAddressee.organization().isEmpty() && !(*contactIt).organization().isEmpty() )
      masterAddressee.setOrganization( (*contactIt).organization() );

/*
  // PHOTO
*/

    // PROID
    if ( masterAddressee.productId().isEmpty() && !(*contactIt).productId().isEmpty() )
      masterAddressee.setProductId( (*contactIt).productId() );

    // REV
    if ( masterAddressee.revision().isNull() && !(*contactIt).revision().isNull() )
      masterAddressee.setRevision( (*contactIt).revision() );

    // ROLE
    if ( masterAddressee.role().isEmpty() && !(*contactIt).role().isEmpty() )
      masterAddressee.setRole( (*contactIt).role() );

    // SORT-STRING
    if ( masterAddressee.sortString().isEmpty() && !(*contactIt).sortString().isEmpty() )
      masterAddressee.setSortString( (*contactIt).sortString() );

/*
  // SOUND
*/

    // TEL
    KABC::PhoneNumber::List phones = (*contactIt).phoneNumbers();
    KABC::PhoneNumber::List masterPhones = masterAddressee.phoneNumbers();
    KABC::PhoneNumber::List::ConstIterator phoneIt;
    for ( phoneIt = phones.begin(); phoneIt != phones.end(); ++phoneIt )
      if ( !masterPhones.contains( *it ) )
        masterAddressee.insertPhoneNumber( *it );

    // TITLE
    if ( masterAddressee.title().isEmpty() && !(*contactIt).title().isEmpty() )
      masterAddressee.setTitle( (*contactIt).title() );

    // TZ
    if ( !masterAddressee.timeZone().isValid() && (*contactIt).timeZone().isValid() )
      masterAddressee.setTimeZone( (*contactIt).timeZone() );

    // UID // ignore UID

    // URL
    if ( masterAddressee.url().isEmpty() && !(*contactIt).url().isEmpty() )
      masterAddressee.setUrl( (*contactIt).url() );

    // X-
    QStringList customs = (*contactIt).customs();
    QStringList masterCustoms = masterAddressee.customs();
    QStringList newCustoms( masterCustoms );
    for ( it = customs.begin(); it != customs.end(); ++it )
      if ( !masterCustoms.contains( *it ) )
        newCustoms.append( *it );
    masterAddressee.setCustoms( newCustoms );
  }

  return masterAddressee;
}

void KABCore::setCategories()
{
  // Show the category dialog
  if ( mCategorySelectDialog == 0 ) {
    mCategorySelectDialog = new KPIM::CategorySelectDialog( KABPrefs::instance(), mWidget );
    connect( mCategorySelectDialog, SIGNAL( categoriesSelected( const QStringList& ) ),
             SLOT( categoriesSelected( const QStringList& ) ) );
    connect( mCategorySelectDialog, SIGNAL( editCategories() ), SLOT( editCategories() ) );
  }

  QStringList selected = mCategorySelectDialog->selectedCategories();
  mCategorySelectDialog->setCategories();
  mCategorySelectDialog->setSelected( selected );
  mCategorySelectDialog->show();
  mCategorySelectDialog->raise();
}

void KABCore::categoriesSelected( const QStringList &categories )
{
  bool merge = false;
  QString msg = i18n( "Merge with existing categories?" );
  if ( KMessageBox::questionYesNo( mWidget, msg ) == KMessageBox::Yes )
    merge = true;

  QStringList uids = mViewManager->selectedUids();
  QStringList::ConstIterator it;
  for ( it = uids.begin(); it != uids.end(); ++it ) {
    KABC::Addressee addr = mAddressBook->findByUid( *it );
    if ( !addr.isEmpty() ) {
      if ( !merge )
        addr.setCategories( categories );
      else {
        QStringList addrCategories = addr.categories();
        QStringList::ConstIterator catIt;
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

void KABCore::editCategories()
{
  if ( mCategoryEditDialog == 0 ) {
    mCategoryEditDialog = new KPIM::CategoryEditDialog( KABPrefs::instance(), mWidget );
    connect( mCategoryEditDialog, SIGNAL( categoryConfigChanged() ),
             SLOT( setCategories() ) );
  }

  mCategoryEditDialog->show();
  mCategoryEditDialog->raise();
}

bool KABCore::handleCommandLine( KAddressBookIface* iface )
{
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  QCString addrStr = args->getOption( "addr" );
  QCString uidStr = args->getOption( "uid" );

  QString addr, uid, vcard;
  if ( !addrStr.isEmpty() )
    addr = QString::fromLocal8Bit( addrStr );
  if ( !uidStr.isEmpty() )
    uid = QString::fromLocal8Bit( uidStr );

  bool doneSomething = false;

  // Can not see why anyone would pass both a uid and an email address, so I'll leave it that two contact editors will show if they do
  if ( !addr.isEmpty() ) {
    iface->addEmail( addr );
    doneSomething = true;
  }

  if ( !uid.isEmpty() ) {
    iface->showContactEditor( uid );
    doneSomething = true;
  }

  if ( args->isSet( "new-contact" ) ) {
    iface->newContact();
    doneSomething = true;
  }

  if ( args->count() >= 1 ) {
    for ( int i = 0; i < args->count(); ++i )
      iface->importVCard( args->url( i ).url() );
    doneSomething = true;
  }
  return doneSomething;
}

#include "kabcore.moc"
