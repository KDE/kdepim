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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

#include <kaccelmanager.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kabc/addresseedialog.h>
#include <kabc/distributionlist.h>
#include <kabc/stdaddressbook.h>
#include <kabc/vcardconverter.h>
#include <libkdepim/kvcarddrag.h>

#include "core.h"

#include "distributionlistwidget.h"

class DistributionListFactory : public KAB::ExtensionFactory
{
  public:
    KAB::ExtensionWidget *extension( KAB::Core *core, QWidget *parent, const char *name )
    {
      return new DistributionListWidget( core, parent, name );
    }

    QString identifier() const
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
      : QObject( parent, "DeletePressedCatcher" ), mWidget( parent )
    {
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

class ContactItem : public QListViewItem
{
  public:
    ContactItem( DistributionListView *parent, const KABC::Addressee &addressee,
               const QString &email = QString::null ) :
      QListViewItem( parent ),
      mAddressee( addressee ),
      mEmail( email )
    {
      setText( 0, addressee.realName() );
      if( email.isEmpty() ) {
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
    bool acceptDrop( const QMimeSource* )
    {
      return true;
    }

  private:
    KABC::Addressee mAddressee;
    QString mEmail;
};

DistributionListWidget::DistributionListWidget( KAB::Core *core, QWidget *parent,
                                                const char *name )
  : KAB::ExtensionWidget( core, parent, name ), mManager( 0 )
{
  QGridLayout *topLayout = new QGridLayout( this, 3, 4, KDialog::marginHint(),
                                            KDialog::spacingHint() );

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
  topLayout->addMultiCellWidget( mContactView, 1, 1, 0, 3 );
  connect( mContactView, SIGNAL( selectionChanged() ),
           SLOT( selectionContactViewChanged() ) );
  connect( mContactView, SIGNAL( dropped( QDropEvent*, QListViewItem* ) ),
           SLOT( dropped( QDropEvent*, QListViewItem* ) ) );

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

  mManager = new KABC::DistributionListManager( core->addressBook() );

  connect( KABC::DistributionListWatcher::self(), SIGNAL( changed() ),
           this, SLOT( updateNameCombo() ) );
  connect( core->addressBook(), SIGNAL( addressBookChanged( AddressBook* ) ),
           this, SLOT( updateNameCombo() ) );

  updateNameCombo();

  QObject *catcher = new DeletePressedCatcher( this );
  installEventFilter( catcher );
  mContactView->installEventFilter( catcher );

  KAcceleratorManager::manage( this );
}

DistributionListWidget::~DistributionListWidget()
{
  delete mManager;
}

void DistributionListWidget::save()
{
  mManager->save();
}

void DistributionListWidget::selectionContactViewChanged()
{
  ContactItem *contactItem =
                  static_cast<ContactItem *>( mContactView->selectedItem() );
  bool state = contactItem;

  mChangeEmailButton->setEnabled( state );
  mRemoveContactButton->setEnabled( state );
}

void DistributionListWidget::createList()
{
  QString newName = KInputDialog::getText( i18n( "New Distribution List" ),
                                           i18n( "Please enter name:" ),
                                           QString::null, 0, this );

  if ( newName.isEmpty() ) return;

  if ( mManager->listNames().contains( newName ) ) {
    KMessageBox::sorry( this, i18n( "The name already exists" ) );
    return;
  }
  new KABC::DistributionList( mManager, newName );

  mNameCombo->clear();
  mNameCombo->insertStringList( mManager->listNames() );
  mNameCombo->setCurrentItem( mNameCombo->count() - 1 );

  updateContactView();

  changed();
}

void DistributionListWidget::editList()
{
  QString oldName = mNameCombo->currentText();

  QString newName = KInputDialog::getText( i18n( "Rename Distribution List" ),
                                           i18n( "Please enter name:" ),
                                           oldName, 0, this );

  if ( newName.isEmpty() ) return;

  if ( mManager->listNames().contains( newName ) ) {
    KMessageBox::sorry( this, i18n( "The name already exists" ) );
    return;
  }
  KABC::DistributionList *list = mManager->list( oldName );
  list->setName( newName );

  int pos = mNameCombo->currentItem();
  mNameCombo->clear();
  mNameCombo->insertStringList( mManager->listNames() );
  mNameCombo->setCurrentItem( pos );

  updateContactView();

  changed();
}

void DistributionListWidget::removeList()
{
  int result = KMessageBox::warningContinueCancel( this,
      i18n( "<qt>Delete distribution list <b>%1</b>?</qt>" ) .arg( mNameCombo->currentText() ),
      QString::null, KGuiItem( i18n("Delete"), "editdelete") );

  if ( result != KMessageBox::Continue )
    return;

  mManager->remove( mManager->list( mNameCombo->currentText() ) );
  mNameCombo->removeItem( mNameCombo->currentItem() );

  updateContactView();

  changed();
}

void DistributionListWidget::addContact()
{
  KABC::DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list )
    return;

  const KABC::Addressee::List addrList = selectedContacts();
  KABC::Addressee::List::ConstIterator it;
  for ( it = addrList.begin(); it != addrList.end(); ++it )
    list->insertEntry( *it );

  updateContactView();

  changed();
}

void DistributionListWidget::removeContact()
{
  KABC::DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list )
    return;

  ContactItem *contactItem =
                    static_cast<ContactItem *>( mContactView->selectedItem() );
  if ( !contactItem )
    return;

  list->removeEntry( contactItem->addressee(), contactItem->email() );
  delete contactItem;

  changed();
}

void DistributionListWidget::changeEmail()
{
  KABC::DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list )
    return;

  ContactItem *contactItem =
                    static_cast<ContactItem *>( mContactView->selectedItem() );
  if ( !contactItem )
    return;

  QString email = EmailSelector::getEmail( contactItem->addressee().emails(),
                                           contactItem->email(), this );
  list->removeEntry( contactItem->addressee(), contactItem->email() );
  list->insertEntry( contactItem->addressee(), email );

  updateContactView();

  changed();
}

void DistributionListWidget::updateContactView()
{
  mContactView->clear();

  KABC::DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) {
    mEditListButton->setEnabled( false );
    mRemoveListButton->setEnabled( false );
    mChangeEmailButton->setEnabled( false );
    mRemoveContactButton->setEnabled( false );
    mContactView->setEnabled( false );
    return;
  } else {
    mEditListButton->setEnabled( true );
    mRemoveListButton->setEnabled( true );
    mContactView->setEnabled( true );
  }

  uint entryCount = 0;
  const KABC::DistributionList::Entry::List entries = list->entries();
  KABC::DistributionList::Entry::List::ConstIterator it;
  for( it = entries.begin(); it != entries.end(); ++it, ++entryCount )
    new ContactItem( mContactView, (*it).addressee, (*it).email );

  ContactItem *contactItem =
                    static_cast<ContactItem *>( mContactView->selectedItem() );
  bool state = contactItem;

  mChangeEmailButton->setEnabled( state );
  mRemoveContactButton->setEnabled( state );

  mEntryCountLabel->setText( i18n( "Count: %n contact", "Count: %n contacts", entryCount ) );
}

void DistributionListWidget::updateNameCombo()
{
  mManager->load();

  int pos = mNameCombo->currentItem();
  mNameCombo->clear();
  mNameCombo->insertStringList( mManager->listNames() );
  mNameCombo->setCurrentItem( pos );

  updateContactView();
}

void DistributionListWidget::dropEvent( QDropEvent *e )
{
  KABC::DistributionList *distributionList = mManager->list( mNameCombo->currentText() );
  if ( !distributionList )
    return;

  QString vcards;
  if ( KVCardDrag::decode( e, vcards ) ) {
    KABC::VCardConverter converter;
    const KABC::Addressee::List list = converter.parseVCards( vcards );
    KABC::Addressee::List::ConstIterator it;
    for ( it = list.begin(); it != list.end(); ++it )
      distributionList->insertEntry( *it );

    changed();
    updateContactView();
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

void DistributionListWidget::dropped( QDropEvent *e, QListViewItem* )
{
  dropEvent( e );
}

void DistributionListWidget::changed()
{
  save();
}


DistributionListView::DistributionListView( QWidget *parent, const char* name )
  : KListView( parent, name )
{
  setDragEnabled( true );
  setAcceptDrops( true );
  setAllColumnsShowFocus( true );
}

void DistributionListView::dragEnterEvent( QDragEnterEvent* e )
{
  bool canDecode = QTextDrag::canDecode( e );
  e->accept( canDecode );
}

void DistributionListView::viewportDragMoveEvent( QDragMoveEvent *e )
{
  bool canDecode = QTextDrag::canDecode( e );
  e->accept( canDecode );
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
  : KDialogBase( KDialogBase::Plain, i18n("Select Email Address"), Ok, Ok,
               parent )
{
  QFrame *topFrame = plainPage();
  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mButtonGroup = new QButtonGroup( 1, Horizontal, i18n("Email Addresses"),
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
  QButton *button = mButtonGroup->selected();
  if ( button )
    return mEmailMap[ mButtonGroup->id( button ) ];

  return QString::null;
}

QString EmailSelector::getEmail( const QStringList &emails,
                                 const QString &current, QWidget *parent )
{
  EmailSelector dlg( emails, current, parent );
  dlg.exec();

  return dlg.selected();
}


#include "distributionlistwidget.moc"
