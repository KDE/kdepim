/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#include "distributionlistwidget.h"

#include <tqbuttongroup.h>
#include <tqcombobox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlistview.h>
#include <tqpushbutton.h>
#include <tqradiobutton.h>

#include <kaccelmanager.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kabc/addresseedialog.h>
#ifdef KDEPIM_NEW_DISTRLISTS
#include <libkdepim/distributionlist.h>
typedef KPIM::DistributionList DistributionList;
#else
#include <kabc/distributionlist.h>
typedef KABC::DistributionList DistributionList;
#endif
#include <kabc/stdaddressbook.h>
#include <kabc/vcardconverter.h>
#include <libkdepim/kvcarddrag.h>

#include "core.h"

class DistributionListFactory : public KAB::ExtensionFactory
{
  public:
    KAB::ExtensionWidget *extension( KAB::Core *core, TQWidget *parent, const char *name )
    {
      return new DistributionListWidget( core, parent, name );
    }

    TQString identifier() const
    {
      return "distribution_list_editor";
    }
};

extern "C" {
  void *init_libkaddrbk_distributionlist()
  {
    return ( new DistributionListFactory );
  }
}

/**
  We have to catch when the 'Del' key is pressed, otherwise
  the event is forwarded to the ViewManager and it tries to
  remove a contact instead of the distribution list.
 */
class DeletePressedCatcher : public QObject
{
  public:
    DeletePressedCatcher( DistributionListWidget *parent )
      : TQObject( parent, "DeletePressedCatcher" ), mWidget( parent )
    {
    }

  protected:
    bool eventFilter( TQObject*, TQEvent *event )
    {
      if ( event->type() == TQEvent::AccelOverride ) {
        TQKeyEvent *keyEvent = (TQKeyEvent*)event;
        if ( keyEvent->key() == Qt::Key_Delete ) {
          keyEvent->accept();
          mWidget->removeContact();
          return true;
        } else
          return false;
      } else {
        return false;
      }
    }

  private:
    DistributionListWidget *mWidget;
};

class ContactItem : public QListViewItem
{
  public:
    ContactItem( DistributionListView *parent, const KABC::Addressee &addressee,
               const TQString &email = TQString::null ) :
      TQListViewItem( parent ),
      mAddressee( addressee ),
      mEmail( email )
    {
      setText( 0, addressee.realName() );
      if ( email.isEmpty() ) {
        setText( 1, addressee.preferredEmail() );
        setText( 2, i18n( "Yes" ) );
      } else {
        setText( 1, email );
        setText( 2, i18n( "No" ) );
      }
    }

    KABC::Addressee addressee() const
    {
      return mAddressee;
    }

    TQString email() const
    {
      return mEmail;
    }

  protected:
    bool acceptDrop( const TQMimeSource* )
    {
      return true;
    }

  private:
    KABC::Addressee mAddressee;
    TQString mEmail;
};

DistributionListWidget::DistributionListWidget( KAB::Core *core, TQWidget *parent,
                                                const char *name )
  : KAB::ExtensionWidget( core, parent, name )
#ifndef KDEPIM_NEW_DISTRLISTS
  , mManager( 0 )
#endif
{
  TQGridLayout *topLayout = new TQGridLayout( this, 3, 4, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  mNameCombo = new TQComboBox( this );
  topLayout->addWidget( mNameCombo, 0, 0 );
  connect( mNameCombo, TQT_SIGNAL( activated( int ) ), TQT_SLOT( updateContactView() ) );

  mCreateListButton = new TQPushButton( i18n( "New List..." ), this );
  topLayout->addWidget( mCreateListButton, 0, 1 );
  connect( mCreateListButton, TQT_SIGNAL( clicked() ), TQT_SLOT( createList() ) );

  mEditListButton = new TQPushButton( i18n( "Rename List..." ), this );
  topLayout->addWidget( mEditListButton, 0, 2 );
  connect( mEditListButton, TQT_SIGNAL( clicked() ), TQT_SLOT( editList() ) );

  mRemoveListButton = new TQPushButton( i18n( "Remove List" ), this );
  topLayout->addWidget( mRemoveListButton, 0, 3 );
  connect( mRemoveListButton, TQT_SIGNAL( clicked() ), TQT_SLOT( removeList() ) );

  mContactView = new DistributionListView( this );
  mContactView->addColumn( i18n( "Name" ) );
  mContactView->addColumn( i18n( "Email" ) );
  mContactView->addColumn( i18n( "Use Preferred" ) );
  mContactView->setEnabled( false );
  mContactView->setAllColumnsShowFocus( true );
  mContactView->setFullWidth( true );
  topLayout->addMultiCellWidget( mContactView, 1, 1, 0, 3 );
  connect( mContactView, TQT_SIGNAL( selectionChanged() ),
           TQT_SLOT( selectionContactViewChanged() ) );
  connect( mContactView, TQT_SIGNAL( dropped( TQDropEvent*, TQListViewItem* ) ),
           TQT_SLOT( dropped( TQDropEvent*, TQListViewItem* ) ) );

  mAddContactButton = new TQPushButton( i18n( "Add Contact" ), this );
  mAddContactButton->setEnabled( false );
  topLayout->addWidget( mAddContactButton, 2, 0 );
  connect( mAddContactButton, TQT_SIGNAL( clicked() ), TQT_SLOT( addContact() ) );

  mEntryCountLabel = new TQLabel( this );
  topLayout->addWidget( mEntryCountLabel, 2, 1 );

  mChangeEmailButton = new TQPushButton( i18n( "Change Email..." ), this );
  topLayout->addWidget( mChangeEmailButton, 2, 2 );
  connect( mChangeEmailButton, TQT_SIGNAL( clicked() ), TQT_SLOT( changeEmail() ) );

  mRemoveContactButton = new TQPushButton( i18n( "Remove Contact" ), this );
  topLayout->addWidget( mRemoveContactButton, 2, 3 );
  connect( mRemoveContactButton, TQT_SIGNAL( clicked() ), TQT_SLOT( removeContact() ) );

#ifdef KDEPIM_NEW_DISTRLISTS
  // When contacts are changed, update both distr list combo and contents of displayed distr list
  connect( core, TQT_SIGNAL( contactsUpdated() ),
           this, TQT_SLOT( updateNameCombo() ) );
#else
  mManager = new KABC::DistributionListManager( core->addressBook() );

  connect( KABC::DistributionListWatcher::self(), TQT_SIGNAL( changed() ),
           this, TQT_SLOT( updateNameCombo() ) );
#endif

  connect( core->addressBook(), TQT_SIGNAL( addressBookChanged( AddressBook* ) ),
           this, TQT_SLOT( updateNameCombo() ) );

  updateNameCombo();

  TQObject *catcher = new DeletePressedCatcher( this );
  installEventFilter( catcher );
  mContactView->installEventFilter( catcher );

  mContactView->restoreLayout( KGlobal::config(), "DistributionListViewColumns" );

  KAcceleratorManager::manage( this );
}

DistributionListWidget::~DistributionListWidget()
{
#ifndef KDEPIM_NEW_DISTRLISTS
  delete mManager;
#endif

  mContactView->saveLayout( KGlobal::config(), "DistributionListViewColumns" );
}

void DistributionListWidget::save()
{
#ifndef KDEPIM_NEW_DISTRLISTS
  mManager->save();
#endif
}

void DistributionListWidget::selectionContactViewChanged()
{
  ContactItem *contactItem =
                  static_cast<ContactItem *>( mContactView->selectedItem() );
  bool state = contactItem;

  mChangeEmailButton->setEnabled( state );
  mRemoveContactButton->setEnabled( state );
}

bool DistributionListWidget::alreadyExists( const TQString& distrListName ) const
{
#ifdef KDEPIM_NEW_DISTRLISTS
  return core()->distributionListNames().contains( distrListName );
#else
  return mManager->listNames().contains( distrListName );
#endif
}

void DistributionListWidget::createList()
{
  TQString newName = KInputDialog::getText( i18n( "New Distribution List" ),
                                           i18n( "Please enter name:" ),
                                           TQString::null, 0, this );

  if ( newName.isEmpty() ) return;

  if ( alreadyExists( newName ) ) {
    KMessageBox::sorry( this, i18n( "The name already exists" ) );
    return;
  }
#ifdef KDEPIM_NEW_DISTRLISTS
  KABC::Resource* resource = core()->requestResource( this );
  if ( !resource )
    return;

  KPIM::DistributionList dist;
  dist.setResource( resource );
  dist.setName( newName );
  // Creates undo-redo command, calls setModified, also triggers contactsUpdated,
  // which triggers updateNameCombo, so the new name appears
  changed( dist );
  core()->addressBook()->insertAddressee( dist );

#else
  new KABC::DistributionList( mManager, newName );
  changed();

  updateNameCombo();
#endif

  // Select the new one in the list
  mNameCombo->setCurrentText( newName );
  // Display the contents of the list we just selected (well, it's empty)
  updateContactView();
}

void DistributionListWidget::editList()
{
  const TQString oldName = mNameCombo->currentText();

  const TQString newName = KInputDialog::getText( i18n( "Rename Distribution List" ),
                                                 i18n( "Please enter name:" ),
                                                 oldName, 0, this );

  if ( newName.isEmpty() ) return;

  if ( alreadyExists( newName ) ) {
    KMessageBox::sorry( this, i18n( "The name already exists." ) );
    return;
  }
#ifdef KDEPIM_NEW_DISTRLISTS
  KPIM::DistributionList dist = KPIM::DistributionList::findByName(
    core()->addressBook(), mNameCombo->currentText() );
  if ( dist.isEmpty() ) // not found [should be impossible]
    return;

  dist.setFormattedName( newName );
  core()->addressBook()->insertAddressee( dist );

  changed( dist );
#else
  KABC::DistributionList *list = mManager->list( oldName );
  list->setName( newName );
  mManager->save();
  updateNameCombo();
#endif

  // Select the new name in the list (updateNameCombo couldn't know we wanted that one)
  mNameCombo->setCurrentText( newName );
  // Display the contents of the list we just selected
  updateContactView();

#ifndef KDEPIM_NEW_DISTRLISTS
  changed();
#endif
}

void DistributionListWidget::removeList()
{
  int result = KMessageBox::warningContinueCancel( this,
      i18n( "<qt>Delete distribution list <b>%1</b>?</qt>" ) .arg( mNameCombo->currentText() ),
      TQString::null, KGuiItem( i18n("Delete"), "editdelete") );

  if ( result != KMessageBox::Continue )
    return;

#ifdef KDEPIM_NEW_DISTRLISTS
  KPIM::DistributionList dist = KPIM::DistributionList::findByName(
    core()->addressBook(), mNameCombo->currentText() );
  if ( dist.isEmpty() ) // not found [should be impossible]
    return;

  emit deleted( dist.uid() );
  core()->addressBook()->removeAddressee( dist );
#else
  mManager->remove( mManager->list( mNameCombo->currentText() ) );
  mNameCombo->removeItem( mNameCombo->currentItem() );

  updateContactView();

  changed();
#endif
}

void DistributionListWidget::addContact()
{
#ifdef KDEPIM_NEW_DISTRLISTS
  KPIM::DistributionList dist = KPIM::DistributionList::findByName(
    core()->addressBook(), mNameCombo->currentText() );
  if ( dist.isEmpty() ) { // not found
    kdDebug(5720) << k_funcinfo << mNameCombo->currentText() << " not found" << endl;
    return;
  }
#else
  KABC::DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list )
    return;
  KABC::DistributionList& dist = *list;
#endif

  const KABC::Addressee::List addrList = selectedContacts();
  KABC::Addressee::List::ConstIterator it;
  for ( it = addrList.begin(); it != addrList.end(); ++it )
    dist.insertEntry( *it );

#ifdef KDEPIM_NEW_DISTRLISTS
  core()->addressBook()->insertAddressee( dist );
  changed( dist );
#else
  updateContactView();
  changed();
#endif
}

void DistributionListWidget::removeContact()
{
#ifdef KDEPIM_NEW_DISTRLISTS
  KPIM::DistributionList dist = KPIM::DistributionList::findByName(
    core()->addressBook(), mNameCombo->currentText() );
  if ( dist.isEmpty() ) // not found
    return;
#else
  KABC::DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list )
    return;
  KABC::DistributionList& dist = *list;
#endif

  ContactItem *contactItem =
                    static_cast<ContactItem *>( mContactView->selectedItem() );
  if ( !contactItem )
    return;

  dist.removeEntry( contactItem->addressee(), contactItem->email() );
  delete contactItem;

#ifdef KDEPIM_NEW_DISTRLISTS
  core()->addressBook()->insertAddressee( dist );
  changed( dist );
#else
  changed();
#endif
}

void DistributionListWidget::changeEmail()
{
#ifdef KDEPIM_NEW_DISTRLISTS
  KPIM::DistributionList dist = KPIM::DistributionList::findByName(
    core()->addressBook(), mNameCombo->currentText() );
  if ( dist.isEmpty() ) // not found
    return;
#else
  KABC::DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list )
    return;
  KABC::DistributionList& dist = *list;
#endif

  ContactItem *contactItem =
                    static_cast<ContactItem *>( mContactView->selectedItem() );
  if ( !contactItem )
    return;

  bool canceled = false;
  const TQString email = EmailSelector::getEmail( contactItem->addressee().emails(),
                                                 contactItem->email(), this, canceled);
  if( canceled)
     return;
  dist.removeEntry( contactItem->addressee(), contactItem->email() );
  dist.insertEntry( contactItem->addressee(), email );

#ifdef KDEPIM_NEW_DISTRLISTS
  core()->addressBook()->insertAddressee( dist );
  changed( dist );
#else
  updateContactView();
  changed();
#endif
}

void DistributionListWidget::updateContactView()
{
  mContactView->clear();

  bool isListSelected = false;
#ifdef KDEPIM_NEW_DISTRLISTS
  KPIM::DistributionList dist;
  if ( mNameCombo->count() != 0 )
    dist = KPIM::DistributionList::findByName(
      core()->addressBook(), mNameCombo->currentText() );
  isListSelected = !dist.isEmpty();
#else
  KABC::DistributionList *list = mManager->list( mNameCombo->currentText() );
  isListSelected = list != 0;
#endif
  if ( !isListSelected ) {
    mEditListButton->setEnabled( false );
    mRemoveListButton->setEnabled( false );
    mChangeEmailButton->setEnabled( false );
    mRemoveContactButton->setEnabled( false );
    mContactView->setEnabled( false );
    return;
  }
  mEditListButton->setEnabled( true );
  mRemoveListButton->setEnabled( true );
  mContactView->setEnabled( true );

  uint entryCount = 0;
#ifdef KDEPIM_NEW_DISTRLISTS
  const KPIM::DistributionList::Entry::List entries = dist.entries( core()->addressBook() );
  KPIM::DistributionList::Entry::List::ConstIterator it;
#else
  const KABC::DistributionList::Entry::List entries = list->entries();
  KABC::DistributionList::Entry::List::ConstIterator it;
#endif
  for ( it = entries.begin(); it != entries.end(); ++it, ++entryCount )
    new ContactItem( mContactView, (*it).addressee, (*it).email );

  bool state = mContactView->selectedItem() != 0;
  mChangeEmailButton->setEnabled( state );
  mRemoveContactButton->setEnabled( state );

  mEntryCountLabel->setText( i18n( "Count: %n contact", "Count: %n contacts", entryCount ) );
}

void DistributionListWidget::updateNameCombo()
{
  int pos = mNameCombo->currentItem();
  mNameCombo->clear();
#ifdef KDEPIM_NEW_DISTRLISTS
  const TQStringList names = core()->distributionListNames();
#else
  mManager->load();
  const TQStringList names = mManager->listNames();
#endif
  mNameCombo->insertStringList( names );
  mNameCombo->setCurrentItem( QMIN( pos, (int)names.count() - 1 ) );

  updateContactView();
}

void DistributionListWidget::dropEvent( TQDropEvent *e )
{
  if ( mNameCombo->count() == 0 )
    return;

#ifdef KDEPIM_NEW_DISTRLISTS
  KPIM::DistributionList dist = KPIM::DistributionList::findByName(
    core()->addressBook(), mNameCombo->currentText() );
  if ( dist.isEmpty() )
    return;
#else
  KABC::DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list )
    return;
  KABC::DistributionList& dist = *list;
#endif

  TQString vcards;
  if ( KVCardDrag::decode( e, vcards ) ) {
    KABC::VCardConverter converter;
    const KABC::Addressee::List lst = converter.parseVCards( vcards );
    for ( KABC::Addressee::List::ConstIterator it = lst.begin(); it != lst.end(); ++it )
      dist.insertEntry( *it );

#ifdef KDEPIM_NEW_DISTRLISTS
    core()->addressBook()->insertAddressee( dist );
    changed( dist );
#else
    changed();
    updateContactView();
#endif
  }
}

void DistributionListWidget::contactsSelectionChanged()
{
  mAddContactButton->setEnabled( contactsSelected() && mNameCombo->count() > 0 );
}

TQString DistributionListWidget::title() const
{
  return i18n( "Distribution List Editor" );
}

TQString DistributionListWidget::identifier() const
{
  return "distribution_list_editor";
}

void DistributionListWidget::dropped( TQDropEvent *e, TQListViewItem* )
{
  dropEvent( e );
}

#ifdef KDEPIM_NEW_DISTRLISTS
void DistributionListWidget::changed( const KABC::Addressee& dist )
{
  emit modified( KABC::Addressee::List() << dist );
}
#else
void DistributionListWidget::changed()
{
  save();
}
#endif

DistributionListView::DistributionListView( TQWidget *parent, const char* name )
  : KListView( parent, name )
{
  setDragEnabled( true );
  setAcceptDrops( true );
  setAllColumnsShowFocus( true );
}

void DistributionListView::dragEnterEvent( TQDragEnterEvent* e )
{
  bool canDecode = TQTextDrag::canDecode( e );
  e->accept( canDecode );
}

void DistributionListView::viewportDragMoveEvent( TQDragMoveEvent *e )
{
  bool canDecode = TQTextDrag::canDecode( e );
  e->accept( canDecode );
}

void DistributionListView::viewportDropEvent( TQDropEvent *e )
{
  emit dropped( e, 0 );
}

void DistributionListView::dropEvent( TQDropEvent *e )
{
  emit dropped( e, 0 );
}


EmailSelector::EmailSelector( const TQStringList &emails,
                              const TQString &current, TQWidget *parent )
  : KDialogBase( KDialogBase::Plain, i18n("Select Email Address"), Ok|Cancel, Ok,
               parent )
{
  TQFrame *topFrame = plainPage();
  TQBoxLayout *topLayout = new TQVBoxLayout( topFrame );

  mButtonGroup = new TQButtonGroup( 1, Horizontal, i18n("Email Addresses"),
                                   topFrame );
  mButtonGroup->setRadioButtonExclusive( true );
  topLayout->addWidget( mButtonGroup );

  TQRadioButton *button = new TQRadioButton( i18n("Preferred address"), mButtonGroup );
  button->setDown( true );
  mEmailMap.insert( mButtonGroup->id( button ), "" );

  TQStringList::ConstIterator it;
  for ( it = emails.begin(); it != emails.end(); ++it ) {
    button = new TQRadioButton( *it, mButtonGroup );
    mEmailMap.insert( mButtonGroup->id( button ), *it );
    if ( (*it) == current )
      button->setDown( true );
  }
}

TQString EmailSelector::selected() const
{
  TQButton *button = mButtonGroup->selected();
  if ( button )
    return mEmailMap[ mButtonGroup->id( button ) ];

  return TQString::null;
}

TQString EmailSelector::getEmail( const TQStringList &emails,
                                 const TQString &current, TQWidget *parent, bool &canceled )
{
  EmailSelector dlg( emails, current, parent );
  if(dlg.exec())
  {
    canceled = false;
    return dlg.selected();
  }
  canceled = true;
  return TQString();
}


#include "distributionlistwidget.moc"
