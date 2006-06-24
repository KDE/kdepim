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

#include <q3buttongroup.h>
#include <q3button.h>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <q3listview.h>
#include <QPushButton>
#include <QRadioButton>
//Added by qt3to4:
#include <QGridLayout>
#include <QKeyEvent>
#include <QEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFrame>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QDragEnterEvent>

#include <kacceleratormanager.h>
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
#include <libkdepim/kvcarddrag.h>

#include "core.h"

class DistributionListFactory : public KAB::ExtensionFactory
{
  public:
    KAB::ExtensionWidget *extension( KAB::Core *core, QWidget *parent )
    {
      return new DistributionListWidget( core, parent );
    }

    QString identifier() const
    {
      return "distribution_list_editor";
    }
};

extern "C" {
  KDE_EXPORT void *init_libkaddrbk_distributionlist()
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
      : QObject( parent ), mWidget( parent )
    {
      setObjectName( "DeletePressedCatcher" );
    }

  protected:
    bool eventFilter( QObject*, QEvent *event )
    {
      if ( event->type() == QEvent::AccelOverride ) {
        QKeyEvent *keyEvent = (QKeyEvent*)event;
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

class ContactItem : public Q3ListViewItem
{
  public:
    ContactItem( DistributionListView *parent, const KABC::Addressee &addressee,
               const QString &email = QString() ) :
      Q3ListViewItem( parent ),
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

    QString email() const
    {
      return mEmail;
    }

  protected:
    bool acceptDrop( const QMimeSource* ) const
    {
      return true;
    }

  private:
    KABC::Addressee mAddressee;
    QString mEmail;
};

DistributionListWidget::DistributionListWidget( KAB::Core *core, QWidget *parent )
  : KAB::ExtensionWidget( core, parent )
#ifndef KDEPIM_NEW_DISTRLISTS
  , mManager( 0 )
#endif
{
  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( KDialog::marginHint() );

  mNameCombo = new QComboBox( this );
  topLayout->addWidget( mNameCombo, 0, 0 );
  connect( mNameCombo, SIGNAL( activated( int ) ), SLOT( updateContactView() ) );

  mCreateListButton = new QPushButton( i18n( "New List..." ), this );
  topLayout->addWidget( mCreateListButton, 0, 1 );
  connect( mCreateListButton, SIGNAL( clicked() ), SLOT( createList() ) );

  mEditListButton = new QPushButton( i18n( "Rename List..." ), this );
  topLayout->addWidget( mEditListButton, 0, 2 );
  connect( mEditListButton, SIGNAL( clicked() ), SLOT( editList() ) );

  mRemoveListButton = new QPushButton( i18n( "Remove List" ), this );
  topLayout->addWidget( mRemoveListButton, 0, 3 );
  connect( mRemoveListButton, SIGNAL( clicked() ), SLOT( removeList() ) );

  mContactView = new DistributionListView( this );
  mContactView->addColumn( i18n( "Name" ) );
  mContactView->addColumn( i18n( "Email" ) );
  mContactView->addColumn( i18n( "Use Preferred" ) );
  mContactView->setEnabled( false );
  mContactView->setAllColumnsShowFocus( true );
  mContactView->setFullWidth( true );
  topLayout->addWidget( mContactView, 1, 0, 1, 4 );
  connect( mContactView, SIGNAL( selectionChanged() ),
           SLOT( selectionContactViewChanged() ) );
  connect( mContactView, SIGNAL( dropped( QDropEvent*, Q3ListViewItem* ) ),
           SLOT( dropped( QDropEvent*, Q3ListViewItem* ) ) );

  mAddContactButton = new QPushButton( i18n( "Add Contact" ), this );
  mAddContactButton->setEnabled( false );
  topLayout->addWidget( mAddContactButton, 2, 0 );
  connect( mAddContactButton, SIGNAL( clicked() ), SLOT( addContact() ) );

  mEntryCountLabel = new QLabel( this );
  topLayout->addWidget( mEntryCountLabel, 2, 1 );

  mChangeEmailButton = new QPushButton( i18n( "Change Email..." ), this );
  topLayout->addWidget( mChangeEmailButton, 2, 2 );
  connect( mChangeEmailButton, SIGNAL( clicked() ), SLOT( changeEmail() ) );

  mRemoveContactButton = new QPushButton( i18n( "Remove Contact" ), this );
  topLayout->addWidget( mRemoveContactButton, 2, 3 );
  connect( mRemoveContactButton, SIGNAL( clicked() ), SLOT( removeContact() ) );

#ifdef KDEPIM_NEW_DISTRLISTS
  // When contacts are changed, update both distr list combo and contents of displayed distr list
  connect( core, SIGNAL( contactsUpdated() ),
           this, SLOT( updateNameCombo() ) );
#else
  mManager = new KABC::DistributionListManager( core->addressBook() );

  connect( KABC::DistributionListWatcher::self(), SIGNAL( changed() ),
           this, SLOT( updateNameCombo() ) );
#endif

  connect( core->addressBook(), SIGNAL( addressBookChanged( AddressBook* ) ),
           this, SLOT( updateNameCombo() ) );

  updateNameCombo();

  QObject *catcher = new DeletePressedCatcher( this );
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

bool DistributionListWidget::alreadyExists( const QString& distrListName ) const
{
#ifdef KDEPIM_NEW_DISTRLISTS
  return core()->distributionListNames().contains( distrListName );
#else
  return mManager->listNames().contains( distrListName );
#endif
}

void DistributionListWidget::createList()
{
  QString newName = KInputDialog::getText( i18n( "New Distribution List" ),
                                           i18n( "Please enter name:" ),
                                           QString(), 0, this );

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
  core()->addressBook()->insertAddressee( dist );

  // Creates undo-redo command, calls setModified, also triggers contactsUpdated,
  // which triggers updateNameCombo, so the new name appears
  changed( dist );

#else
  new KABC::DistributionList( mManager, newName );
  changed();

  updateNameCombo();
#endif

  // Select the new one in the list
  mNameCombo->setItemText( mNameCombo->currentIndex(), newName );
  // Display the contents of the list we just selected (well, it's empty)
  updateContactView();
}

void DistributionListWidget::editList()
{
  const QString oldName = mNameCombo->currentText();

  const QString newName = KInputDialog::getText( i18n( "Rename Distribution List" ),
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
  updateNameCombo();
#endif

  // Select the new name in the list (updateNameCombo couldn't know we wanted that one)
  mNameCombo->setItemText( mNameCombo->currentIndex(), newName );
  // Display the contents of the list we just selected
  updateContactView();
}

void DistributionListWidget::removeList()
{
  int result = KMessageBox::warningContinueCancel( this,
      i18n( "<qt>Delete distribution list <b>%1</b>?</qt>" , mNameCombo->currentText() ),
      QString(), KGuiItem( i18n("Delete"), "editdelete") );

  if ( result != KMessageBox::Continue )
    return;

#ifdef KDEPIM_NEW_DISTRLISTS
  KPIM::DistributionList dist = KPIM::DistributionList::findByName(
    core()->addressBook(), mNameCombo->currentText() );
  if ( dist.isEmpty() ) // not found [should be impossible]
    return;

  core()->addressBook()->removeAddressee( dist );

  emit deleted( QStringList( dist.uid() ) );
#else
  mManager->remove( mManager->list( mNameCombo->currentText() ) );
  mNameCombo->removeItem( mNameCombo->currentIndex() );

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
    kDebug(5720) << k_funcinfo << mNameCombo->currentText() << " not found" << endl;
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

  const QString email = EmailSelector::getEmail( contactItem->addressee().emails(),
                                                 contactItem->email(), this );
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

  mEntryCountLabel->setText( i18np( "Count: %n contact", "Count: %n contacts", entryCount ) );
}

void DistributionListWidget::updateNameCombo()
{
  int pos = mNameCombo->currentIndex();
  mNameCombo->clear();
#ifdef KDEPIM_NEW_DISTRLISTS
  const QStringList names = core()->distributionListNames();
#else
  mManager->load();
  const QStringList names = mManager->listNames();
#endif
  mNameCombo->addItems( names );
  mNameCombo->setCurrentIndex( qMin( pos, (int)names.count() - 1 ) );

  updateContactView();
}

void DistributionListWidget::dropEvent( QDropEvent *e )
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

  if ( KVCardDrag::canDecode( e ) ) {
    KABC::Addressee::List lst;
    KVCardDrag::decode( e, lst );

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

QString DistributionListWidget::title() const
{
  return i18n( "Distribution List Editor" );
}

QString DistributionListWidget::identifier() const
{
  return "distribution_list_editor";
}

void DistributionListWidget::dropped( QDropEvent *e, Q3ListViewItem* )
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

DistributionListView::DistributionListView( QWidget *parent )
  : K3ListView( parent )
{
  setDragEnabled( true );
  setAcceptDrops( true );
  setAllColumnsShowFocus( true );
}

void DistributionListView::dragEnterEvent( QDragEnterEvent* e )
{
  bool canDecode = Q3TextDrag::canDecode( e );
  e->setAccepted( canDecode );
}

void DistributionListView::viewportDragMoveEvent( QDragMoveEvent *e )
{
  bool canDecode = Q3TextDrag::canDecode( e );
  e->setAccepted( canDecode );
}

void DistributionListView::viewportDropEvent( QDropEvent *e )
{
  emit dropped( e, 0 );
}

void DistributionListView::dropEvent( QDropEvent *e )
{
  emit dropped( e, 0 );
}


EmailSelector::EmailSelector( const QStringList &emails,
                              const QString &current, QWidget *parent )
  : KDialog( parent )
{
  setCaption( i18n("Select Email Address") );
  setButtons( Ok );
  setDefaultButton( Ok );

  QFrame *topFrame = new QFrame( this );
  setMainWidget( topFrame );
  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mButtonGroup = new Q3ButtonGroup( 1, Qt::Horizontal, i18n("Email Addresses"),
                                   topFrame );
  mButtonGroup->setRadioButtonExclusive( true );
  topLayout->addWidget( mButtonGroup );

  QRadioButton *button = new QRadioButton( i18n("Preferred address"), mButtonGroup );
  button->setDown( true );
  mEmailMap.insert( mButtonGroup->id( button ), "" );

  QStringList::ConstIterator it;
  for ( it = emails.begin(); it != emails.end(); ++it ) {
    button = new QRadioButton( *it, mButtonGroup );
    mEmailMap.insert( mButtonGroup->id( button ), *it );
    if ( (*it) == current )
      button->setDown( true );
  }
}

QString EmailSelector::selected() const
{
  QAbstractButton *button = mButtonGroup->selected();
  if ( button )
    return mEmailMap[ mButtonGroup->id( button ) ];

  return QString();
}

QString EmailSelector::getEmail( const QStringList &emails,
                                 const QString &current, QWidget *parent )
{
  EmailSelector dlg( emails, current, parent );
  dlg.exec();

  return dlg.selected();
}


#include "distributionlistwidget.moc"
