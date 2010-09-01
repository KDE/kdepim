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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

// Needed for ugly hack, to be removed in 4.0
#include <unistd.h> // for usleep
#include <tqeventloop.h>

#include <tqclipboard.h>
#include <tqdir.h>
#include <tqfile.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqptrlist.h>
#include <tqwidgetstack.h>
#include <tqregexp.h>
#include <tqvbox.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>

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
#include <kpushbutton.h>
#include <kresources/selectdialog.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <kstdguiitem.h>
#include <kxmlguiclient.h>
#include <ktoolbar.h>
#include <libkdepim/addresseeview.h>
#include <libkdepim/categoryeditdialog.h>
#include <libkdepim/categoryselectdialog.h>
#include <libkdepim/resourceabc.h>
#include "distributionlisteditor.h"

#include "addresseeutil.h"
#include "addresseeeditordialog.h"
#include "distributionlistentryview.h"
#include "extensionmanager.h"
#include "filterselectionwidget.h"
#include "incsearchwidget.h"
#include "jumpbuttonbar.h"
#include "kablock.h"
#include "kabprefs.h"
#include "kabtools.h"
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

KABCore::KABCore( KXMLGUIClient *client, bool readWrite, TQWidget *parent,
                  const TQString &file, const char *name )
  : KAB::Core( client, parent, name ), mStatusBar( 0 ), mViewManager( 0 ),
    mExtensionManager( 0 ), mJumpButtonBar( 0 ), mCategorySelectDialog( 0 ),
    mCategoryEditDialog( 0 ), mLdapSearchDialog( 0 ), mReadWrite( readWrite ),
    mModified( false )
{
  mWidget = new TQWidget( parent, name );

  mIsPart = !parent->isA( "KAddressBookMain" );

  mAddressBookChangedTimer = new TQTimer( this );
  connect( mAddressBookChangedTimer, TQT_SIGNAL( timeout() ),
           this, TQT_SLOT( addressBookChanged() ) );

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

#if ! KDE_IS_VERSION(3,5,8)
  mAddressBook->addCustomField( i18n( "Department" ), KABC::Field::Organization,
                                "X-Department", "KADDRESSBOOK" );
#endif
  mAddressBook->addCustomField( i18n( "Profession" ), KABC::Field::Organization,
                                "X-Profession", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Assistant's Name" ), KABC::Field::Organization,
                                "X-AssistantsName", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Manager's Name" ), KABC::Field::Organization,
                                "X-ManagersName", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Partner's Name" ), KABC::Field::Personal,
                                "X-SpousesName", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Office" ), KABC::Field::Personal,
                                "X-Office", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "IM Address" ), KABC::Field::Personal,
                                "X-IMAddress", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Anniversary" ), KABC::Field::Personal,
                                "X-Anniversary", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Blog" ), KABC::Field::Personal,
                                "BlogFeed", "KADDRESSBOOK" );

  mSearchManager = new KAB::SearchManager( mAddressBook, parent );

  connect( mSearchManager, TQT_SIGNAL( contactsUpdated() ),
           this, TQT_SLOT( slotContactsUpdated() ) );

  initGUI();

  connect( mAddressBook, TQT_SIGNAL( addressBookChanged( AddressBook* ) ),
           TQT_SLOT( delayedAddressBookChanged() ) );
  connect( mAddressBook, TQT_SIGNAL( loadingFinished( Resource* ) ),
           TQT_SLOT( delayedAddressBookChanged() ) );

  mIncSearchWidget->setFocus();

  connect( mViewManager, TQT_SIGNAL( selected( const TQString& ) ),
           TQT_SLOT( setContactSelected( const TQString& ) ) );
  connect( mViewManager, TQT_SIGNAL( executed( const TQString& ) ),
           TQT_SLOT( editContact( const TQString& ) ) );
  connect( mViewManager, TQT_SIGNAL( modified() ),
           TQT_SLOT( setModified() ) );
  connect( mViewManager, TQT_SIGNAL( urlDropped( const KURL& ) ),
           mXXPortManager, TQT_SLOT( importVCard( const KURL& ) ) );
  connect( mViewManager, TQT_SIGNAL( viewFieldsChanged() ),
           TQT_SLOT( updateIncSearchWidget() ) );
  connect( mExtensionManager, TQT_SIGNAL( modified( const KABC::Addressee::List& ) ),
           this, TQT_SLOT( extensionModified( const KABC::Addressee::List& ) ) );
  connect( mExtensionManager, TQT_SIGNAL( deleted( const TQStringList& ) ),
           this, TQT_SLOT( extensionDeleted( const TQStringList& ) ) );

  connect( mXXPortManager, TQT_SIGNAL( modified() ),
           TQT_SLOT( setModified() ) );

  connect( mDetailsViewer, TQT_SIGNAL( highlightedMessage( const TQString& ) ),
           TQT_SLOT( detailsHighlighted( const TQString& ) ) );

  connect( mIncSearchWidget, TQT_SIGNAL( scrollUp() ),
           mViewManager, TQT_SLOT( scrollUp() ) );
  connect( mIncSearchWidget, TQT_SIGNAL( scrollDown() ),
           mViewManager, TQT_SLOT( scrollDown() ) );

  mAddressBookService = new KAddressBookService( this );

  mCommandHistory = new KCommandHistory( actionCollection(), true );
  connect( mCommandHistory, TQT_SIGNAL( commandExecuted() ),
           mSearchManager, TQT_SLOT( reload() ) );

  mSearchManager->reload();

  setModified( false );

  KAcceleratorManager::manage( mWidget );

  mKIMProxy = ::KIMProxy::instance( kapp->dcopClient() );
}

KABCore::~KABCore()
{
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

  TQValueList<int> splitterSize = KABPrefs::instance()->detailsSplitter();
  if ( splitterSize.count() == 0 ) {
    splitterSize.append( 360 );
    splitterSize.append( 260 );
  }
  mDetailsSplitter->setSizes( splitterSize );

  const TQValueList<int> leftSplitterSizes = KABPrefs::instance()->leftSplitter();
  if ( !leftSplitterSizes.isEmpty() )
      mLeftSplitter->setSizes( leftSplitterSizes );
}

void KABCore::saveSettings()
{
  KABPrefs::instance()->setJumpButtonBarVisible( mActionJumpBar->isChecked() );
  KABPrefs::instance()->setDetailsPageVisible( mActionDetails->isChecked() );
  KABPrefs::instance()->setDetailsSplitter( mDetailsSplitter->sizes() );
  KABPrefs::instance()->setLeftSplitter( mLeftSplitter->sizes() );

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

TQStringList KABCore::selectedUIDs() const
{
  return mViewManager->selectedUids();
}

KABC::Resource *KABCore::requestResource( TQWidget *parent )
{
  TQPtrList<KABC::Resource> kabcResources = addressBook()->resources();

  TQPtrList<KRES::Resource> kresResources;
  TQPtrListIterator<KABC::Resource> resIt( kabcResources );
  KABC::Resource *resource;
  while ( ( resource = resIt.current() ) != 0 ) {
    ++resIt;
    bool writable = false;
    if ( resource->inherits( "KPIM::ResourceABC" ) ) {
      KPIM::ResourceABC *resAbc = static_cast<KPIM::ResourceABC *>( resource );
      const TQStringList subresources = resAbc->subresources();
      for ( TQStringList::ConstIterator it = subresources.begin(); it != subresources.end(); ++it ) {
        if ( resAbc->subresourceActive(*it) && resAbc->subresourceWritable(*it) ) {
          writable = true;
          break;
        }
      }
    } else {
      if ( !resource->readOnly() ) {
        writable = true;
      }
    }

    if ( writable ) {
      KRES::Resource *res = resource; // downcast
      kresResources.append( res );
    }
  }

  KRES::Resource *res = KRES::SelectDialog::getResource( kresResources, parent );
  return static_cast<KABC::Resource*>( res ); // upcast
}

TQWidget *KABCore::widget() const
{
  return mWidget;
}

KAboutData *KABCore::createAboutData()
{
  KAboutData *about = new KAboutData( "kaddressbook", I18N_NOOP( "KAddressBook" ),
                                      "3.5.11", I18N_NOOP( "The KDE Address Book" ),
                                      KAboutData::License_GPL_V2,
                                      I18N_NOOP( "(c) 2008-2010, The Trinity Team\n(c) 1997-2005, The KDE PIM Team" ) );
  about->addAuthor( "Timothy Pearson", I18N_NOOP( "Current maintainer" ), "kb9vqf@pearsoncomputing.net" );
  about->addAuthor( "Tobias Koenig", I18N_NOOP( "Previous maintainer" ), "tokoe@kde.org" );
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

void KABCore::setContactSelected( const TQString &uid )
{
  // Avoid crash on exit
  if ( !mAddressBook ) {
      return;
  }

  KABC::Addressee addr = mAddressBook->findByUid( uid );
  if ( !mDetailsViewer->isHidden() )
    mDetailsViewer->setAddressee( addr );
#ifdef KDEPIM_NEW_DISTRLISTS
  if ( !mSelectedDistributionList.isNull() && mDistListEntryView->isShown() ) {
      showDistributionListEntry( uid );
  }
#endif
  mExtensionManager->setSelectionChanged();

  KABC::Addressee::List list = mViewManager->selectedAddressees();
  const bool someSelected = list.size() > 0;
  const bool singleSelected = list.size() == 1;
  bool writable = mReadWrite;

  if ( writable ) {
    //check if every single (sub)resource is writable
    //### We have a performance problem here - everytime *one* item is added or
    //    removed we re-check *all* items. If this turns out to be a bottleneck
    //    we need to keep some state and check new items only.
    KABC::Addressee::List::ConstIterator addrIt = list.constBegin();
    for ( ; addrIt != list.constEnd(); ++addrIt ) {
      KABC::Resource *res = ( *addrIt ).resource();
      if ( !res ) {
        kdDebug() << "KABCore::setContactSelected: this addressee has no resource!" << endl;
        writable = false;
        break;
      }
      if ( res->readOnly() ) {
        writable = false;
        break;
      }
      //HACK: manual polymorphism
      if ( res->inherits( "KPIM::ResourceABC" ) ) {
        KPIM::ResourceABC *resAbc = static_cast<KPIM::ResourceABC *>( res );

        TQString subresource = resAbc->uidToResourceMap()[ ( *addrIt ).uid() ];
        if ( !subresource.isEmpty() && !resAbc->subresourceWritable( subresource ) ) {
          writable = false;
          break;
        }
      }
    }
  }

  bool moreThanOneResource = mAddressBook->resources().count() > 1;
  if ( !moreThanOneResource && !mAddressBook->resources().isEmpty() ) {
      KABC::Resource *res = mAddressBook->resources().first();
      if ( res->inherits( "KPIM::ResourceABC" ) ) {
        KPIM::ResourceABC *resAbc = static_cast<KPIM::ResourceABC *>( res );
        const TQStringList subresources = resAbc->subresources();
        int writeables = 0;
        for ( TQStringList::ConstIterator it = subresources.begin(); it != subresources.end(); ++it ) {
            if ( resAbc->subresourceActive(*it) && resAbc->subresourceWritable(*it) ) {
                writeables++;
            }
        }
        moreThanOneResource = ( writeables >= 2 );
      }
  }

  // update the actions

  mActionCopy->setEnabled( someSelected );
  mActionCut->setEnabled( someSelected && writable );
  mActionDelete->setEnabled( someSelected && writable );
  // the "edit" dialog doubles as the details dialog and it knows when the addressee is read-only
  // (### this does not make much sense from the user perspective!)
  mActionEditAddressee->setEnabled( singleSelected && !mExtensionManager->isQuickEditVisible());
  mActionCopyAddresseeTo->setEnabled( someSelected && moreThanOneResource );
  mActionMoveAddresseeTo->setEnabled( someSelected && moreThanOneResource && writable );
  mActionMail->setEnabled( someSelected );
  mActionMailVCard->setEnabled( someSelected );
  mActionChat->setEnabled( singleSelected && mKIMProxy && mKIMProxy->initialize() );
  mActionWhoAmI->setEnabled( singleSelected );
  mActionCategories->setEnabled( someSelected && writable );
  mActionMerge->setEnabled( ( list.size() == 2 ) && writable );

  if ( mReadWrite ) {
    QClipboard *cb = TQApplication::clipboard();
#if defined(KABC_VCARD_ENCODING_FIX)
    const TQMimeSource *data = cb->data( QClipboard::Clipboard );
    list = AddresseeUtil::clipboardToAddressees( data->encodedData( "text/x-vcard" ) );
#else
    list = AddresseeUtil::clipboardToAddressees( cb->text() );
#endif
    mActionPaste->setEnabled( !list.isEmpty() );
  }
#ifdef KDEPIM_NEW_DISTRLISTS
  mAddDistListButton->setEnabled( writable );
  mRemoveDistListButton->setEnabled( someSelected && writable );
#endif
}

void KABCore::sendMail()
{
  //FIXME: breaks with email addresses containing ","
  sendMail( mViewManager->selectedEmails().join( ", " ) );
}

void KABCore::sendMail( const TQString& email )
{
  kapp->invokeMailer( email, "" );
}

void KABCore::mailVCard()
{
  TQStringList uids = mViewManager->selectedUids();
  if ( !uids.isEmpty() )
    mailVCard( uids );
}

void KABCore::mailVCard( const TQStringList &uids )
{
  KABTools::mailVCards( uids, mAddressBook );
}

void KABCore::startChat()
{
  TQStringList uids = mViewManager->selectedUids();
  if ( !uids.isEmpty() )
    mKIMProxy->chatWithContact( uids.first() );
}

void KABCore::browse( const TQString& url )
{
  kapp->invokeBrowser( url );
}

void KABCore::selectAllContacts()
{
  mViewManager->setSelected( TQString::null, true );
}

void KABCore::deleteContacts()
{
  TQStringList uidList = mViewManager->selectedUids();

  deleteContacts( uidList );
}

void KABCore::deleteDistributionLists( const TQStringList & names )
{
  if ( names.isEmpty() )
      return;
  if ( KMessageBox::warningContinueCancelList( mWidget, i18n( "Do you really want to delete this distribution list?",
                                                 "Do you really want to delete these %n distribution lists?", names.count() ),
                                                 names, TQString::null, KStdGuiItem::del() ) == KMessageBox::Cancel )
   return;

  TQStringList uids;
  for ( TQStringList::ConstIterator it = names.begin(); it != names.end(); ++it ) {
      uids.append( KPIM::DistributionList::findByName( mAddressBook, *it ).uid() );
  }
  DeleteCommand *command = new DeleteCommand( mAddressBook, uids );
  mCommandHistory->addCommand( command );
  setModified( true );
}

void KABCore::deleteContacts( const TQStringList &uids )
{
  if ( uids.count() > 0 ) {
    TQStringList names;
    TQStringList::ConstIterator it = uids.begin();
    const TQStringList::ConstIterator endIt( uids.end() );
    while ( it != endIt ) {
      KABC::Addressee addr = mAddressBook->findByUid( *it );
      names.append( addr.realName().isEmpty() ? addr.preferredEmail() : addr.realName() );
      ++it;
    }

    if ( KMessageBox::warningContinueCancelList(
           mWidget,
           i18n( "<qt>"
                 "Do you really want to delete this contact from your addressbook?<br>"
                 "<b>Note:</b>The contact will be also removed from all distribution lists."
                 "</qt>",
                 "<qt>"
                 "Do you really want to delete these %n contacts from your addressbook?<br>"
                 "<b>Note:</b>The contacts will be also removed from all distribution lists."
                 "</qt>",
                 uids.count() ),
           names, TQString::null, KStdGuiItem::del() ) == KMessageBox::Cancel ) {
      return;
    }

    DeleteCommand *command = new DeleteCommand( mAddressBook, uids );
    mCommandHistory->addCommand( command );

    // now if we deleted anything, refresh
    setContactSelected( TQString::null );
    setModified( true );
  }
}

void KABCore::copyContacts()
{
  KABC::Addressee::List addrList = mViewManager->selectedAddressees();

#if defined(KABC_VCARD_ENCODING_FIX)
  TQByteArray clipText = AddresseeUtil::addresseesToClipboard( addrList );
  QClipboard *cb = TQApplication::clipboard();
  cb->setText( TQString::fromUtf8( clipText.data() ) );
#else
  TQString clipText = AddresseeUtil::addresseesToClipboard( addrList );
  QClipboard *cb = TQApplication::clipboard();
  cb->setText( clipText );
#endif
}

void KABCore::cutContacts()
{
  TQStringList uidList = mViewManager->selectedUids();

  if ( uidList.size() > 0 ) {
    CutCommand *command = new CutCommand( mAddressBook, uidList );
    mCommandHistory->addCommand( command );

    setModified( true );
  }
}

void KABCore::pasteContacts()
{
  QClipboard *cb = TQApplication::clipboard();
#if defined(KABC_VCARD_ENCODING_FIX)
  const TQMimeSource *data = cb->data( QClipboard::Clipboard );
  KABC::Addressee::List list = AddresseeUtil::clipboardToAddressees( data->encodedData( "text/x-vcard" ) );
#else
  KABC::Addressee::List list = AddresseeUtil::clipboardToAddressees( cb->text() );
#endif
  pasteContacts( list );
}

void KABCore::pasteContacts( KABC::Addressee::List &list )
{
  KABC::Resource *resource = requestResource( mWidget );
  if ( !resource )
    return;

  KABC::Addressee::List::Iterator it;
  const KABC::Addressee::List::Iterator endIt( list.end() );
  for ( it = list.begin(); it != endIt; ++it )
    (*it).setResource( resource );

  PasteCommand *command = new PasteCommand( this, list );
  mCommandHistory->addCommand( command );

  setModified( true );
}

void KABCore::mergeContacts()
{
  KABC::Addressee::List list = mViewManager->selectedAddressees();
  if ( list.count() < 2 )
    return;

  KABC::Addressee addr = KABTools::mergeContacts( list );

  KABC::Addressee::List::Iterator it = list.begin();
  const KABC::Addressee::List::Iterator endIt( list.end() );
  KABC::Addressee origAddr = *it;
  TQStringList uids;
  ++it;
  while ( it != endIt ) {
    uids.append( (*it).uid() );
    ++it;
  }

  DeleteCommand *command = new DeleteCommand( mAddressBook, uids );
  mCommandHistory->addCommand( command );

  EditCommand *editCommand = new EditCommand( mAddressBook, origAddr, addr );
  mCommandHistory->addCommand( editCommand );

  mSearchManager->reload();
}

void KABCore::setWhoAmI()
{
  KABC::Addressee::List addrList = mViewManager->selectedAddressees();

  if ( addrList.count() > 1 ) {
    // can probably be removed because we now check the selection in setContactSelected().
    KMessageBox::sorry( mWidget, i18n( "Please select only one contact." ) );
    return;
  }

  TQString text( i18n( "<qt>Do you really want to use <b>%1</b> as your new personal contact?</qt>" ) );
  if ( KMessageBox::questionYesNo( mWidget, text.arg( addrList[ 0 ].assembledName() ), TQString::null, i18n("Use"), i18n("Do Not Use") ) == KMessageBox::Yes )
    static_cast<KABC::StdAddressBook*>( KABC::StdAddressBook::self( true ) )->setWhoAmI( addrList[ 0 ] );
}

void KABCore::incrementalTextSearch( const TQString& text )
{
  setContactSelected( TQString::null );
  mSearchManager->search( text, mIncSearchWidget->currentFields() );
}

void KABCore::incrementalJumpButtonSearch( const TQString& character )
{
  mViewManager->setSelected( TQString::null, false );

  KABC::AddresseeList list = mSearchManager->contacts();
  KABC::Field *field = mViewManager->currentSortField();
  if ( field ) {
    list.sortByField( field );
    KABC::AddresseeList::ConstIterator it;
    const KABC::AddresseeList::ConstIterator endIt( list.end() );
    for ( it = list.begin(); it != endIt; ++it ) {
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
    KABC::Addressee::List addressees;
    addressees.append( addr );
    command = new NewCommand( mAddressBook, addressees );
  } else {
    command = new EditCommand( mAddressBook, origAddr, addr );
  }

  mCommandHistory->addCommand( command );

  setContactSelected( addr.uid() );
  setModified( true );
}

void KABCore::newDistributionList()
{
#ifdef KDEPIM_NEW_DISTRLISTS
  KABC::Resource *resource = requestResource( mWidget );
  if ( !resource )
    return;

  TQString name = i18n( "New Distribution List" );
  const KPIM::DistributionList distList = KPIM::DistributionList::findByName( addressBook(), name );
  if ( !distList.isEmpty() ) {
    bool foundUnused = false;
    int i = 1;
    while ( !foundUnused ) {
      name = i18n( "New Distribution List (%1)" ).arg( i++ );
      foundUnused = KPIM::DistributionList::findByName( addressBook(), name ).isEmpty();
    }
  }
  KPIM::DistributionList list;
  list.setUid( KApplication::randomString( 10 ) );
  list.setName( name );
  list.setResource( resource );
  editDistributionList( list );
#endif
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

void KABCore::addEmail( const TQString &aStr )
{
  TQString fullName, email;

  KABC::Addressee::parseEmailAddress( aStr, fullName, email );

#if KDE_IS_VERSION(3,4,89)
  // This ugly hack will be removed in 4.0
  // addressbook may not be reloaded yet, as done asynchronously sometimes, so wait
  while ( !mAddressBook->loadingHasFinished() ) {
    TQApplication::eventLoop()->processEvents( TQEventLoop::ExcludeUserInput );
    // use sleep here to reduce cpu usage
    usleep( 100 );
  }
#endif

  // Try to lookup the addressee matching the email address
  bool found = false;
  TQStringList emailList;
  KABC::AddressBook::Iterator it;
  const KABC::AddressBook::Iterator endIt( mAddressBook->end() );
  for ( it = mAddressBook->begin(); !found && (it != endIt); ++it ) {
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

void KABCore::importVCardFromData( const TQString &vCard )
{
  mXXPortManager->importVCardFromData( vCard );
}

void KABCore::editContact( const TQString &uid )
{
  if ( mExtensionManager->isQuickEditVisible() )
    return;

  // First, locate the contact entry
  TQString localUID = uid;
  if ( localUID.isNull() ) {
    TQStringList uidList = mViewManager->selectedUids();
    if ( uidList.count() > 0 )
      localUID = *( uidList.at( 0 ) );
  }
#if KDE_IS_VERSION(3,4,89)
  // This ugly hack will be removed in 4.0
  // for calls with given uid, as done from commandline and DCOP
  // addressbook may not be reloaded yet, as done asynchronously, so wait
  else while ( !mAddressBook->loadingHasFinished() ) {
    TQApplication::eventLoop()->processEvents( TQEventLoop::ExcludeUserInput );
    // use sleep here to reduce cpu usage
    usleep( 100 );
  }
#endif

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


void KABCore::copySelectedContactToResource()
{
    storeContactIn( TQString(), true /*copy*/);
}

void KABCore::moveSelectedContactToResource()
{
    storeContactIn( TQString(), false /*copy*/);
}

void KABCore::storeContactIn( const TQString &uid, bool copy /*false*/ )
{
  // First, locate the contact entry
  TQStringList uidList;
  if ( uid.isNull() ) {
    uidList = mViewManager->selectedUids();
  } else {
    uidList << uid;
  }
  KABC::Resource *resource = requestResource( mWidget );
  if ( !resource )
    return;

  if ( copy ) {
    CopyToCommand *command = new CopyToCommand( mAddressBook, uidList, resource );
    mCommandHistory->addCommand( command );
  }
  else {
    MoveToCommand *command = new MoveToCommand( this, uidList, resource );
    mCommandHistory->addCommand( command );
  }

  addressBookChanged();
  setModified( true );
}

void KABCore::save()
{
  TQPtrList<KABC::Resource> resources = mAddressBook->resources();
  TQPtrListIterator<KABC::Resource> it( resources );
  while ( it.current() && !it.current()->readOnly() ) {
    KABC::Ticket *ticket = mAddressBook->requestSaveTicket( it.current() );
    if ( ticket ) {
      if ( !mAddressBook->save( ticket ) ) {
        KMessageBox::error( mWidget,
                            i18n( "<qt>Unable to save address book <b>%1</b>.</qt>" ).arg( it.current()->resourceName() ) );
        mAddressBook->releaseSaveTicket( ticket );
      } else {
        setModified( false );
      }
    } else {
      KMessageBox::error( mWidget,
                          i18n( "<qt>Unable to get access for saving the address book <b>%1</b>.</qt>" )
                          .arg( it.current()->resourceName() ) );
    }

    ++it;
  }
}

void KABCore::load()
{
  TQPtrList<KABC::Resource> resources = mAddressBook->resources();
  TQPtrListIterator<KABC::Resource> it( resources );
  while ( it.current() ) {
    mAddressBook->load();
    ++it;
  }
}

void KABCore::setJumpButtonBarVisible( bool visible )
{
  if ( visible ) {
    if ( !mJumpButtonBar )
      createJumpButtonBar();
    mJumpButtonBar->show();
  } else
    if ( mJumpButtonBar )
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
    const KABC::Addressee::List::ConstIterator endIt( list.end() );
    for ( it = list.begin(); it != endIt; ++it ) {
      Command *command = 0;

      // check if it exists already
      KABC::Addressee origAddr = mAddressBook->findByUid( (*it).uid() );
      if ( origAddr.isEmpty() ) {
        KABC::Addressee::List addressees;
        addressees.append( *it );
        command = new NewCommand( mAddressBook, addressees );
      } else
        command = new EditCommand( mAddressBook, origAddr, *it );

      mCommandHistory->blockSignals( true );
      mCommandHistory->addCommand( command );
      mCommandHistory->blockSignals( false );
    }

    setModified(true);
  }
}

void KABCore::extensionDeleted( const TQStringList &uidList )
{
  DeleteCommand *command = new DeleteCommand( mAddressBook, uidList );
  mCommandHistory->addCommand( command );

  // now if we deleted anything, refresh
  setContactSelected( TQString::null );
  setModified( true );
}

TQString KABCore::getNameByPhone( const TQString &phone )
{
#if KDE_IS_VERSION(3,4,89)
  // This ugly hack will be removed in 4.0
  // addressbook may not be reloaded yet, as done asynchronously, so wait
  while ( !mAddressBook->loadingHasFinished() ) {
    TQApplication::eventLoop()->processEvents( TQEventLoop::ExcludeUserInput );
    // use sleep here to reduce cpu usage
    usleep( 100 );
  }
#endif

  TQRegExp r( "[/*/-/ ]" );
  TQString localPhone( phone );

  bool found = false;
  TQString ownerName = "";
  KABC::PhoneNumber::List phoneList;

  KABC::AddressBook::ConstIterator iter;
  const KABC::AddressBook::ConstIterator endIter( mAddressBook->end() );

  for ( iter = mAddressBook->begin(); !found && ( iter != endIter ); ++iter ) {
    phoneList = (*iter).phoneNumbers();
    KABC::PhoneNumber::List::Iterator phoneIter( phoneList.begin() );
    const KABC::PhoneNumber::List::Iterator phoneEndIter( phoneList.end() );
    for ( ; !found && ( phoneIter != phoneEndIter ); ++phoneIter) {
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
    connect( mLdapSearchDialog, TQT_SIGNAL( addresseesAdded() ),
            TQT_SLOT( addressBookChanged() ) );
    connect( mLdapSearchDialog, TQT_SIGNAL( addresseesAdded() ),
            TQT_SLOT( setModified() ) );
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
  connect( &dlg, TQT_SIGNAL( configCommitted() ),
           this, TQT_SLOT( configurationChanged() ) );

  dlg.addModule( "kabconfig.desktop" );
  dlg.addModule( "kabldapconfig.desktop" );
  dlg.addModule( "kabcustomfields.desktop" );

  dlg.exec();
}

void KABCore::print()
{
  KPrinter printer;
  printer.setDocName( i18n( "Address Book" ) );
  printer.setDocFileName( "addressbook" );

  if ( !printer.setup( mWidget, i18n("Print Addresses") ) )
    return;

  KABPrinting::PrintingWizard wizard( &printer, mAddressBook,
                                      mViewManager->selectedUids(), mWidget );

  wizard.exec();
}

void KABCore::detailsHighlighted( const TQString &msg )
{
  if ( mStatusBar ) {
    if ( !mStatusBar->hasItem( 2 ) )
      mStatusBar->insertItem( msg, 2 );
    else
      mStatusBar->changeItem( msg, 2 );
  }
}

void KABCore::showContactsAddress( const TQString &addrUid )
{
  TQStringList uidList = mViewManager->selectedUids();
  if ( uidList.isEmpty() )
    return;

  KABC::Addressee addr = mAddressBook->findByUid( uidList.first() );
  if ( addr.isEmpty() )
    return;

  const KABC::Address::List list = addr.addresses();
  KABC::Address::List::ConstIterator it;
  const KABC::Address::List::ConstIterator endIt( list.end() );
  for ( it = list.begin(); it != endIt; ++it )
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

bool KABCore::queryClose()
{
  saveSettings();
  KABPrefs::instance()->writeConfig();

  TQPtrList<KABC::Resource> resources = mAddressBook->resources();
  TQPtrListIterator<KABC::Resource> it( resources );
  while ( it.current() ) {
    it.current()->close();
    ++it;
  }

  return true;
}

void KABCore::reinitXMLGUI()
{
  mExtensionManager->createActions();
}
void KABCore::delayedAddressBookChanged()
{
  mAddressBookChangedTimer->start( 1000 );
}

void KABCore::addressBookChanged()
{
  const TQStringList selectedUids = mViewManager->selectedUids();

  mAddressBookChangedTimer->stop();

  if ( mJumpButtonBar )
    mJumpButtonBar->updateButtons();

  mSearchManager->reload();

  mViewManager->setSelected( TQString::null, false );

  TQString uid = TQString::null;
  if ( !selectedUids.isEmpty() ) {
    uid = selectedUids.first();
    mViewManager->setSelected( uid, true );
  }

  setContactSelected( uid );

  updateCategories();
}

AddresseeEditorDialog *KABCore::createAddresseeEditorDialog( TQWidget *parent,
                                                             const char *name )
{
  AddresseeEditorDialog *dialog = new AddresseeEditorDialog( this, parent,
                                                 name ? name : "editorDialog" );
  connect( dialog, TQT_SIGNAL( contactModified( const KABC::Addressee& ) ),
           TQT_SLOT( contactModified( const KABC::Addressee& ) ) );
  connect( dialog, TQT_SIGNAL( editorDestroyed( const TQString& ) ),
           TQT_SLOT( slotEditorDestroyed( const TQString& ) ) );

  return dialog;
}

void KABCore::activateDetailsWidget( TQWidget *widget )
{
  if ( mDetailsStack->visibleWidget() == widget )
    return;
  mDetailsStack->raiseWidget( widget );
}

void KABCore::deactivateDetailsWidget( TQWidget *widget )
{
  if ( mDetailsStack->visibleWidget() != widget )
    return;
  mDetailsStack->raiseWidget( mDetailsWidget );
}

void KABCore::slotEditorDestroyed( const TQString &uid )
{
  AddresseeEditorDialog *dialog = mEditorDict.take( uid );

  KABC::Addressee addr = dialog->addressee();

  if ( !addr.resource()->readOnly() ) {
    TQApplication::setOverrideCursor( Qt::waitCursor );
    KABLock::self( mAddressBook )->unlock( addr.resource() );
    TQApplication::restoreOverrideCursor();
  }
}

void KABCore::initGUI()
{
  TQVBoxLayout *topLayout = new TQVBoxLayout( mWidget, 0, 0 );
  KToolBar* searchTB = new KToolBar( mWidget, "search toolbar");
  searchTB->boxLayout()->setSpacing( KDialog::spacingHint() );
  mIncSearchWidget = new IncSearchWidget( searchTB, "kde toolbar widget");
  searchTB->setStretchableWidget( mIncSearchWidget );
  connect( mIncSearchWidget, TQT_SIGNAL( doSearch( const TQString& ) ),
           TQT_SLOT( incrementalTextSearch( const TQString& ) ) );

  mDetailsSplitter = new TQSplitter( mWidget );

  mLeftSplitter = new TQSplitter( mDetailsSplitter );
  mLeftSplitter->setOrientation( KABPrefs::instance()->contactListAboveExtensions() ? Qt::Vertical : Qt::Horizontal );

  topLayout->addWidget( searchTB );
  topLayout->addWidget( mDetailsSplitter );

  mDetailsStack = new TQWidgetStack( mDetailsSplitter );
  mExtensionManager = new ExtensionManager( new TQWidget( mLeftSplitter ), mDetailsStack, this, this );
  connect( mExtensionManager, TQT_SIGNAL( detailsWidgetDeactivated( TQWidget* ) ),
           this, TQT_SLOT( deactivateDetailsWidget( TQWidget* ) ) );
  connect( mExtensionManager, TQT_SIGNAL( detailsWidgetActivated( TQWidget* ) ),
           this, TQT_SLOT( activateDetailsWidget( TQWidget* ) ) );

  TQWidget *viewWidget = new TQWidget( mLeftSplitter );
  if ( KABPrefs::instance()->contactListAboveExtensions() )
    mLeftSplitter->moveToFirst( viewWidget );
  TQVBoxLayout *viewLayout = new TQVBoxLayout( viewWidget );
  viewLayout->setSpacing( KDialog::spacingHint() );

  mViewHeaderLabel = new TQLabel( viewWidget );
//  mViewHeaderLabel->setSizePolicy( TQSizePolicy::Minimum, TQSizePolicy::Fixed );
  mViewHeaderLabel->setText( i18n( "Contacts" ) );
  viewLayout->addWidget( mViewHeaderLabel );
  mViewManager = new ViewManager( this, viewWidget );
  viewLayout->addWidget( mViewManager, 1 );

#ifdef KDEPIM_NEW_DISTRLISTS
  mDistListButtonWidget = new TQWidget( viewWidget );
  TQHBoxLayout *buttonLayout = new TQHBoxLayout( mDistListButtonWidget );
  buttonLayout->setSpacing( KDialog::spacingHint() );
  buttonLayout->addStretch( 1 );

  mAddDistListButton = new KPushButton( mDistListButtonWidget );
  mAddDistListButton->setEnabled( false );
  mAddDistListButton->setText( i18n( "Add" ) );
  TQToolTip::add( mAddDistListButton, i18n( "Add contacts to the distribution list" ) );
  TQWhatsThis::add( mAddDistListButton,
                   i18n( "Click this button if you want to add more contacts to "
                         "the current distribution list. You will be shown a dialog that allows "
                         "to enter a list of existing contacts to this distribution list." ) );
  connect( mAddDistListButton, TQT_SIGNAL( clicked() ),
           this, TQT_SLOT( editSelectedDistributionList() ) );
  buttonLayout->addWidget( mAddDistListButton );
  mDistListButtonWidget->setShown( false );
  viewLayout->addWidget( mDistListButtonWidget );

  mRemoveDistListButton = new KPushButton( mDistListButtonWidget );
  mRemoveDistListButton->setEnabled( false );
  mRemoveDistListButton->setText( i18n( "Remove" ) );
  TQToolTip::add( mRemoveDistListButton, i18n( "Remove contacts from the distribution list" ) );
  TQWhatsThis::add( mRemoveDistListButton,
                   i18n( "Click this button if you want to remove the selected contacts from "
                         "the current distribution list." ) );
  connect( mRemoveDistListButton, TQT_SIGNAL( clicked() ),
           this, TQT_SLOT( removeSelectedContactsFromDistList() ) );
  buttonLayout->addWidget( mRemoveDistListButton );
#endif

  mFilterSelectionWidget = new FilterSelectionWidget( searchTB , "kde toolbar widget" );
  mViewManager->setFilterSelectionWidget( mFilterSelectionWidget );

  connect( mFilterSelectionWidget, TQT_SIGNAL( filterActivated( int ) ),
           mViewManager, TQT_SLOT( setActiveFilter( int ) ) );

  mDetailsWidget = new TQWidget( mDetailsSplitter );
  mDetailsLayout = new TQHBoxLayout( mDetailsWidget );

  mDetailsPage = new TQWidget( mDetailsWidget );
  mDetailsLayout->addWidget( mDetailsPage );

  TQHBoxLayout *detailsPageLayout = new TQHBoxLayout( mDetailsPage, 0, 0 );
  mDetailsViewer = new KPIM::AddresseeView( mDetailsPage );
  mDetailsViewer->setVScrollBarMode( TQScrollView::Auto );
  detailsPageLayout->addWidget( mDetailsViewer );

  mDistListEntryView = new KAB::DistributionListEntryView( this, mWidget );
  connect( mDistListEntryView, TQT_SIGNAL( distributionListClicked( const TQString& ) ),
           this, TQT_SLOT( sendMailToDistributionList( const TQString& ) ) );
  mDetailsStack->addWidget( mDistListEntryView );
  mDetailsStack->addWidget( mDetailsWidget );
  mDetailsStack->raiseWidget( mDetailsWidget );
  mDetailsSplitter->moveToLast( mDetailsStack );

  connect( mDetailsViewer, TQT_SIGNAL( addressClicked( const TQString&) ),
           this, TQT_SLOT( showContactsAddress( const TQString& ) ) );

  topLayout->setStretchFactor( mDetailsSplitter, 1 );

  mXXPortManager = new XXPortManager( this, mWidget );

  initActions();
}

void KABCore::createJumpButtonBar()
{
  mJumpButtonBar = new JumpButtonBar( this, mDetailsWidget );
  mDetailsLayout->addWidget( mJumpButtonBar );
  mDetailsLayout->setStretchFactor( mJumpButtonBar, 1 );

  connect( mJumpButtonBar, TQT_SIGNAL( jumpToLetter( const TQString& ) ),
           TQT_SLOT( incrementalJumpButtonSearch( const TQString& ) ) );
  connect( mViewManager, TQT_SIGNAL( sortFieldChanged() ),
           mJumpButtonBar, TQT_SLOT( updateButtons() ) );
}

void KABCore::initActions()
{
  connect( TQApplication::clipboard(), TQT_SIGNAL( dataChanged() ),
           TQT_SLOT( clipboardDataChanged() ) );

  KAction *action;

  // file menu
  mActionMail = new KAction( i18n( "&Send Email to Contact..." ), "mail_send", 0,
                             this, TQT_SLOT( sendMail() ), actionCollection(), "file_mail" );
  action = KStdAction::print( this, TQT_SLOT( print() ), actionCollection() );
  mActionMail->setWhatsThis( i18n( "Send a mail to all selected contacts." ) );
  action->setWhatsThis( i18n( "Print a special number of contacts." ) );

  mActionSave = KStdAction::save( this,
                             TQT_SLOT( save() ), actionCollection(), "file_sync" );
  mActionSave->setWhatsThis( i18n( "Save all changes of the address book to the storage backend." ) );

  action = new KAction( i18n( "&New Contact..." ), "identity", CTRL+Key_N, this,
               TQT_SLOT( newContact() ), actionCollection(), "file_new_contact" );
  action->setWhatsThis( i18n( "Create a new contact<p>You will be presented with a dialog where you can add all data about a person, including addresses and phone numbers." ) );

  action = new KAction( i18n( "&New Distribution List..." ), "kontact_contacts", 0, this,
               TQT_SLOT( newDistributionList() ), actionCollection(), "file_new_distributionlist" );
  action->setWhatsThis( i18n( "Create a new distribution list<p>You will be presented with a dialog where you can create a new distribution list." ) );

  mActionMailVCard = new KAction( i18n("Send &Contact..."), "mail_post_to", 0,
                                  this, TQT_SLOT( mailVCard() ),
                                  actionCollection(), "file_mail_vcard" );
  mActionMailVCard->setWhatsThis( i18n( "Send a mail with the selected contact as attachment." ) );

  mActionChat = new KAction( i18n("Chat &With..."), 0,
                                  this, TQT_SLOT( startChat() ),
                                  actionCollection(), "file_chat" );
  mActionChat->setWhatsThis( i18n( "Start a chat with the selected contact." ) );

  mActionEditAddressee = new KAction( i18n( "&Edit Contact..." ), "edit", 0,
                                      this, TQT_SLOT( editContact() ),
                                      actionCollection(), "file_properties" );
  mActionEditAddressee->setWhatsThis( i18n( "Edit a contact<p>You will be presented with a dialog where you can change all data about a person, including addresses and phone numbers." ) );

  mActionMerge = new KAction( i18n( "&Merge Contacts" ), "", 0,
                              this, TQT_SLOT( mergeContacts() ),
                              actionCollection(), "edit_merge" );

  // edit menu
  mActionCopy = KStdAction::copy( this, TQT_SLOT( copyContacts() ), actionCollection() );
  mActionCut = KStdAction::cut( this, TQT_SLOT( cutContacts() ), actionCollection() );
  mActionPaste = KStdAction::paste( this, TQT_SLOT( pasteContacts() ), actionCollection() );
  action = KStdAction::selectAll( this, TQT_SLOT( selectAllContacts() ), actionCollection() );
  mActionCopy->setWhatsThis( i18n( "Copy the currently selected contact(s) to system clipboard in vCard format." ) );
  mActionCut->setWhatsThis( i18n( "Cuts the currently selected contact(s) to system clipboard in vCard format." ) );
  mActionPaste->setWhatsThis( i18n( "Paste the previously cut or copied contacts from clipboard." ) );
  action->setWhatsThis( i18n( "Selects all visible contacts from current view." ) );
//  mActionUndo->setWhatsThis( i18n( "Undoes the last <b>Cut</b>, <b>Copy</b> or <b>Paste</b>." ) );
//  mActionRedo->setWhatsThis( i18n( "Redoes the last <b>Cut</b>, <b>Copy</b> or <b>Paste</b>." ) );

  mActionDelete = new KAction( i18n( "&Delete Contact" ), "editdelete",
                               Key_Delete, this, TQT_SLOT( deleteContacts() ),
                               actionCollection(), "edit_delete" );
  mActionDelete->setWhatsThis( i18n( "Delete all selected contacts." ) );


  mActionCopyAddresseeTo = new KAction( i18n( "&Copy Contact To..." ), "", 0,
                                      this, TQT_SLOT( copySelectedContactToResource() ),
                                      actionCollection(), "copy_contact_to" );
  const TQString copyMoveWhatsThis = i18n( "Store a contact in a different Addressbook<p>You will be presented with a dialog where you can select a new storage place for this contact." );
  mActionCopyAddresseeTo->setWhatsThis( copyMoveWhatsThis );

  mActionMoveAddresseeTo = new KAction( i18n( "M&ove Contact To..." ), "", 0,
                                      this, TQT_SLOT( moveSelectedContactToResource() ),
                                      actionCollection(), "move_contact_to" );
  mActionMoveAddresseeTo->setWhatsThis( copyMoveWhatsThis );

  // settings menu
  mActionJumpBar = new KToggleAction( i18n( "Show Jump Bar" ), "next", 0,
                                      actionCollection(), "options_show_jump_bar" );
  mActionJumpBar->setWhatsThis( i18n( "Toggle whether the jump button bar shall be visible." ) );
  mActionJumpBar->setCheckedState( i18n( "Hide Jump Bar") );
  connect( mActionJumpBar, TQT_SIGNAL( toggled( bool ) ), TQT_SLOT( setJumpButtonBarVisible( bool ) ) );

  mActionDetails = new KToggleAction( i18n( "Show Details" ), 0, 0,
                                      actionCollection(), "options_show_details" );
  mActionDetails->setWhatsThis( i18n( "Toggle whether the details page shall be visible." ) );
  mActionDetails->setCheckedState( i18n( "Hide Details") );
  connect( mActionDetails, TQT_SIGNAL( toggled( bool ) ), TQT_SLOT( setDetailsVisible( bool ) ) );

  if ( mIsPart )
    action = new KAction( i18n( "&Configure Address Book..." ), "configure", 0,
                          this, TQT_SLOT( configure() ), actionCollection(),
                          "kaddressbook_configure" );
  else
    action = KStdAction::preferences( this, TQT_SLOT( configure() ), actionCollection() );

  action->setWhatsThis( i18n( "You will be presented with a dialog, that offers you all possibilities to configure KAddressBook." ) );

  // misc
  action = new KAction( i18n( "&Lookup Addresses in LDAP Directory..." ), "find", 0,
                        this, TQT_SLOT( openLDAPDialog() ), actionCollection(), "ldap_lookup" );
  action->setWhatsThis( i18n( "Search for contacts on a LDAP server<p>You will be presented with a dialog, where you can search for contacts and select the ones you want to add to your local address book." ) );

  mActionWhoAmI = new KAction( i18n( "Set as Personal Contact Data" ), "personal", 0, this,
                               TQT_SLOT( setWhoAmI() ), actionCollection(),
                               "edit_set_personal" );
  mActionWhoAmI->setWhatsThis( i18n( "Set the personal contact<p>The data of this contact will be used in many other KDE applications, so you do not have to input your personal data several times." ) );

  mActionCategories = new KAction( i18n( "Select Categories..." ), 0, this,
                                   TQT_SLOT( setCategories() ), actionCollection(),
                                   "edit_set_categories" );
  mActionCategories->setWhatsThis( i18n( "Set the categories for all selected contacts." ) );

  KAction *clearLocation = new KAction( i18n( "Clear Search Bar" ),
					TQApplication::reverseLayout() ? "clear_left" : "locationbar_erase",
					CTRL+Key_L, this, TQT_SLOT( slotClearSearchBar() ), actionCollection(), "clear_search" );
  clearLocation->setWhatsThis( i18n( "Clear Search Bar<p>"
				     "Clears the content of the quick search bar." ) );

  clipboardDataChanged();
}

void KABCore::clipboardDataChanged()
{
  if ( mReadWrite )
    mActionPaste->setEnabled( !TQApplication::clipboard()->text().isEmpty() );
}

void KABCore::updateIncSearchWidget()
{
  mIncSearchWidget->setViewFields( mViewManager->viewFields() );
}

void KABCore::updateCategories()
{
  TQStringList categories( allCategories() );
  categories.sort();

  const TQStringList customCategories( KABPrefs::instance()->customCategories() );
  TQStringList::ConstIterator it;
  const TQStringList::ConstIterator endIt( customCategories.end() );
  for ( it = customCategories.begin(); it != endIt; ++it ) {
    if ( categories.find( *it ) == categories.end() ) {
      categories.append( *it );
    }
  }

  KABPrefs::instance()->mCustomCategories = categories;
  KABPrefs::instance()->writeConfig();

  if ( mCategoryEditDialog )
    mCategoryEditDialog->reload();
}

TQStringList KABCore::allCategories() const
{
  TQStringList categories, allCategories;
  TQStringList::ConstIterator catIt;

  // Avoid crash on exit
  if ( !mAddressBook ) {
      return allCategories;
  }

  KABC::AddressBook::ConstIterator it;
  const KABC::AddressBook::ConstIterator endIt( mAddressBook->end() );
  for ( it = mAddressBook->begin(); it != endIt; ++it ) {
    categories = (*it).categories();
    const TQStringList::ConstIterator catEndIt( categories.end() );
    for ( catIt = categories.begin(); catIt != catEndIt; ++catIt ) {
      if ( !allCategories.contains( *catIt ) )
        allCategories.append( *catIt );
	}
  }

  return allCategories;
}

void KABCore::setCategories()
{
  // Show the category dialog
  if ( mCategorySelectDialog == 0 ) {
    mCategorySelectDialog = new KPIM::CategorySelectDialog( KABPrefs::instance(), mWidget );
    connect( mCategorySelectDialog, TQT_SIGNAL( categoriesSelected( const TQStringList& ) ),
             TQT_SLOT( categoriesSelected( const TQStringList& ) ) );
    connect( mCategorySelectDialog, TQT_SIGNAL( editCategories() ), TQT_SLOT( editCategories() ) );
  }

  mCategorySelectDialog->show();
  mCategorySelectDialog->raise();
}

void KABCore::categoriesSelected( const TQStringList &categories )
{
  bool merge = false;
  TQString msg = i18n( "Merge with existing categories?" );
  if ( KMessageBox::questionYesNo( mWidget, msg, TQString::null, i18n( "Merge" ), i18n( "Do Not Merge" ) ) == KMessageBox::Yes )
    merge = true;

  TQStringList uids = mViewManager->selectedUids();
  TQStringList::ConstIterator it;
  const TQStringList::ConstIterator endIt( uids.end() );
  for ( it = uids.begin(); it != endIt; ++it ) {
    KABC::Addressee addr = mAddressBook->findByUid( *it );
    if ( !addr.isEmpty() ) {
      if ( !merge )
        addr.setCategories( categories );
      else {
        TQStringList addrCategories = addr.categories();
        TQStringList::ConstIterator catIt;
        const TQStringList::ConstIterator catEndIt( categories.end() );
        for ( catIt = categories.begin(); catIt != catEndIt; ++catIt ) {
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
    connect( mCategoryEditDialog, TQT_SIGNAL( categoryConfigChanged() ),
             mCategorySelectDialog, TQT_SLOT( updateCategoryConfig() ) );
  }

  mCategoryEditDialog->show();
  mCategoryEditDialog->raise();
}

void KABCore::slotClearSearchBar()
{
  mIncSearchWidget->clear();
  mIncSearchWidget->setFocus();
}

void KABCore::slotContactsUpdated()
{
  if ( mStatusBar ) {
    TQString msg( i18n( "%n contact matches", "%n contacts matching", mSearchManager->contacts().count() ) );
    if ( !mStatusBar->hasItem( 1 ) )
      mStatusBar->insertItem( msg, 1 );
    else
      mStatusBar->changeItem( msg, 1 );
  }

  emit contactsUpdated();
}

bool KABCore::handleCommandLine( KAddressBookIface* iface )
{
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  TQCString addrStr = args->getOption( "addr" );
  TQCString uidStr = args->getOption( "uid" );

  TQString addr, uid, vcard;
  if ( !addrStr.isEmpty() )
    addr = TQString::fromLocal8Bit( addrStr );
  if ( !uidStr.isEmpty() )
    uid = TQString::fromLocal8Bit( uidStr );

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

void KABCore::removeSelectedContactsFromDistList()
{
#ifdef KDEPIM_NEW_DISTRLISTS

  KPIM::DistributionList dist = KPIM::DistributionList::findByName( addressBook(), mSelectedDistributionList );
  if ( dist.isEmpty() )
    return;
  const TQStringList uids = selectedUIDs();
  if ( uids.isEmpty() )
      return;

  TQStringList names;
  TQStringList::ConstIterator it = uids.begin();
  const TQStringList::ConstIterator endIt( uids.end() );
  while ( it != endIt ) {
    KABC::Addressee addr = mAddressBook->findByUid( *it );
    names.append( addr.realName().isEmpty() ? addr.preferredEmail() : addr.realName() );
    ++it;
  }

  if ( KMessageBox::warningContinueCancelList(
         mWidget,
         i18n( "<qt>"
               "Do you really want to remove this contact from the %1 distribution list?<br>"
               "<b>Note:</b>The contact will be not be removed from your addressbook nor from "
               "any other distribution list."
               "</qt>",
               "<qt>"
               "Do you really want to remove these %n contacts from the %1 distribution list?<br>"
               "<b>Note:</b>The contacts will be not be removed from your addressbook nor from "
               "any other distribution list."
               "</qt>",
               uids.count() ).arg( mSelectedDistributionList ),
         names, TQString::null, KStdGuiItem::del() ) == KMessageBox::Cancel ) {
    return;
  }

  for ( TQStringList::ConstIterator uidIt = uids.begin(); uidIt != uids.end(); ++uidIt ) {
    typedef KPIM::DistributionList::Entry::List EntryList;
    const EntryList entries = dist.entries( addressBook() );
    for ( EntryList::ConstIterator it = entries.begin(); it != entries.end(); ++it ) {
      if ( (*it).addressee.uid() == (*uidIt) ) {
        dist.removeEntry( (*it).addressee, (*it).email );
        break;
      }
    }
  }
  addressBook()->insertAddressee( dist );
  setModified();
#endif
}

void KABCore::sendMailToDistributionList( const TQString &name )
{
#ifdef KDEPIM_NEW_DISTRLISTS
  KPIM::DistributionList dist = KPIM::DistributionList::findByName( addressBook(), name );
  if ( dist.isEmpty() )
    return;
  typedef KPIM::DistributionList::Entry::List EntryList;
  TQStringList mails;
  const EntryList entries = dist.entries( addressBook() );
  for ( EntryList::ConstIterator it = entries.begin(); it != entries.end(); ++it )
    mails += (*it).addressee.fullEmail( (*it).email );
  sendMail( mails.join( ", " ) );
#endif
}

void KABCore::editSelectedDistributionList()
{
#ifdef KDEPIM_NEW_DISTRLISTS
  editDistributionList( KPIM::DistributionList::findByName( addressBook(), mSelectedDistributionList ) );
#endif
}


void KABCore::editDistributionList( const TQString &name )
{
#ifdef KDEPIM_NEW_DISTRLISTS
  editDistributionList( KPIM::DistributionList::findByName( addressBook(), name ) );
#endif
}

#ifdef KDEPIM_NEW_DISTRLISTS

void KABCore::showDistributionListEntry( const TQString& uid )
{
  KPIM::DistributionList dist = KPIM::DistributionList::findByName( addressBook(), mSelectedDistributionList );
  if ( !dist.isEmpty() ) {
    mDistListEntryView->clear();
    typedef KPIM::DistributionList::Entry::List EntryList;
    const EntryList entries = dist.entries( addressBook() );
    for (EntryList::ConstIterator it = entries.begin(); it != entries.end(); ++it ) {
      if ( (*it).addressee.uid() == uid ) {
        mDistListEntryView->setEntry( dist, *it );
        break;
      }
    }
  }
}

void KABCore::editDistributionList( const KPIM::DistributionList &dist )
{
  if ( dist.isEmpty() )
    return;
  TQGuardedPtr<KPIM::DistributionListEditor::EditorWidget> dlg = new KPIM::DistributionListEditor::EditorWidget( addressBook(), widget() );
  dlg->setDistributionList( dist );
  if ( dlg->exec() == TQDialog::Accepted && dlg ) {
    const KPIM::DistributionList newDist = dlg->distributionList();
    if ( newDist != dist ) {
      setModified();
    }
  }
  delete dlg;
}


KPIM::DistributionList::List KABCore::distributionLists() const
{
  return mSearchManager->distributionLists();
}

void KABCore::setSelectedDistributionList( const TQString &name )
{
  mSelectedDistributionList = name;
  mSearchManager->setSelectedDistributionList( name );
  mViewHeaderLabel->setText( name.isNull() ?
                             i18n( "Contacts" ) :
                             i18n( "Distribution List: %1" ).arg( name ) );
  mDistListButtonWidget->setShown( !mSelectedDistributionList.isNull() );
  if ( !name.isNull() ) {
    mDetailsStack->raiseWidget( mDistListEntryView );
    if ( selectedUIDs().isEmpty() ) {
      mViewManager->setFirstSelected( true );
    }
    const TQStringList selectedUids = selectedUIDs();
    showDistributionListEntry( selectedUids.isEmpty() ? TQString() : selectedUids.first() );
  } else {
    mDetailsStack->raiseWidget( mExtensionManager->activeDetailsWidget() ?
                                mExtensionManager->activeDetailsWidget() : mDetailsWidget );
  }
}

TQStringList KABCore::distributionListNames() const
{
  return mSearchManager->distributionListNames();
}
#endif

#include "kabcore.moc"
