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

#include <core.h>
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
  : KAB::ExtensionWidget( core, parent, name )
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

  mChangeEmailButton = new QPushButton( i18n( "Change Email..." ), this );
  topLayout->addWidget( mChangeEmailButton, 2, 2 );
  connect( mChangeEmailButton, SIGNAL( clicked() ), SLOT( changeEmail() ) );

  mRemoveContactButton = new QPushButton( i18n( "Remove Contact" ), this );
  topLayout->addWidget( mRemoveContactButton, 2, 3 );
  connect( mRemoveContactButton, SIGNAL( clicked() ), SLOT( removeContact() ) );

  connect( core, SIGNAL( contactsUpdated() ),
           this, SLOT( updateNameCombo() ) );

  updateNameCombo();

  KAcceleratorManager::manage( this );
}

DistributionListWidget::~DistributionListWidget()
{
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

  QStringList names = core()->distributionListNames();
  if ( names.contains( newName ) ) {
    KMessageBox::sorry( this, i18n( "The name already exists" ) );
    return;
  }

  KABC::Resource* resource = core()->requestResource( this );
  if ( !resource )
    return;

  KPIM::DistributionList dist;
  dist.setResource( resource );
  dist.setName( newName );
  core()->addressBook()->insertAddressee( dist );

  // For undo-redo command and setModified, also triggers contactsUpdated,
  // which triggers updateNameCombo, so the new name appears
  changed( dist );

  // Select the new one in the list
  mNameCombo->setCurrentText( newName );
  // Display the contents of the list we just selected (well, it's empty)
  updateContactView();
}

void DistributionListWidget::editList()
{
  QString oldName = mNameCombo->currentText();

  QString newName = KInputDialog::getText( i18n( "Distribution List Editor" ),
                                           i18n( "Please enter name:" ),
                                           oldName, 0, this );

  if ( newName.isEmpty() ) return;

  QStringList names = core()->distributionListNames();

  if ( names.contains( newName ) ) {
    KMessageBox::sorry( this, i18n( "The name already exists" ) );
    return;
  }

  KPIM::DistributionList dist = KPIM::DistributionList::findByName(
    core()->addressBook(), mNameCombo->currentText() );
  if ( dist.isEmpty() ) // not found [should be impossible]
    return;

  dist.setFormattedName( newName );
  core()->addressBook()->insertAddressee( dist );

  changed( dist );

  // Select the new name in the list (updateNameCombo couldn't know we wanted that one)
  mNameCombo->setCurrentText( newName );
  // Display the contents of the list we just selected
  updateContactView();
}

void DistributionListWidget::removeList()
{
  int result = KMessageBox::warningContinueCancel( this,
      i18n( "<qt>Delete distribution list <b>%1</b>?</qt>" ) .arg( mNameCombo->currentText() ),
      QString::null, KGuiItem( i18n("Delete"), "editdelete") );

  if ( result != KMessageBox::Continue )
    return;

  KPIM::DistributionList dist = KPIM::DistributionList::findByName(
    core()->addressBook(), mNameCombo->currentText() );
  if ( dist.isEmpty() ) // not found [should be impossible]
    return;

  core()->addressBook()->removeAddressee( dist );

  emit deleted( dist.uid() );
}

void DistributionListWidget::addContact()
{
  KPIM::DistributionList dist = KPIM::DistributionList::findByName(
    core()->addressBook(), mNameCombo->currentText() );
  if ( dist.isEmpty() ) { // not found
    kdDebug(5720) << k_funcinfo << mNameCombo->currentText() << " not found" << endl;
    return;
  }

  KABC::Addressee::List addrList = selectedContacts();
  KABC::Addressee::List::Iterator it;
  for ( it = addrList.begin(); it != addrList.end(); ++it )
    dist.insertEntry( *it );

  core()->addressBook()->insertAddressee( dist );

  //updateContactView();

  changed( dist );
}

void DistributionListWidget::removeContact()
{
  KPIM::DistributionList dist = KPIM::DistributionList::findByName(
    core()->addressBook(), mNameCombo->currentText() );
  if ( dist.isEmpty() ) // not found
    return;

  ContactItem *contactItem =
                    static_cast<ContactItem *>( mContactView->selectedItem() );
  if ( !contactItem )
    return;

  dist.removeEntry( contactItem->addressee(), contactItem->email() );
  core()->addressBook()->insertAddressee( dist );
  delete contactItem;

  changed( dist );
}

void DistributionListWidget::changeEmail()
{
  KPIM::DistributionList dist = KPIM::DistributionList::findByName(
    core()->addressBook(), mNameCombo->currentText() );
  if ( dist.isEmpty() ) // not found
    return;

  ContactItem *contactItem =
                    static_cast<ContactItem *>( mContactView->selectedItem() );
  if ( !contactItem )
    return;

  QString email = EmailSelector::getEmail( contactItem->addressee().emails(),
                                           contactItem->email(), this );
  dist.removeEntry( contactItem->addressee(), contactItem->email() );
  dist.insertEntry( contactItem->addressee(), email );
  core()->addressBook()->insertAddressee( dist );

  //updateContactView();

  changed( dist );
}

void DistributionListWidget::updateContactView()
{
  mContactView->clear();

  KPIM::DistributionList dist = KPIM::DistributionList::findByName(
    core()->addressBook(), mNameCombo->currentText() );
  if ( dist.isEmpty() ) { // not found
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

  KPIM::DistributionList::Entry::List entries = dist.entries( core()->addressBook() );
  KPIM::DistributionList::Entry::List::ConstIterator it;
  for( it = entries.begin(); it != entries.end(); ++it )
    new ContactItem( mContactView, (*it).addressee, (*it).email );

  ContactItem *contactItem =
                    static_cast<ContactItem *>( mContactView->selectedItem() );
  bool state = contactItem;

  mChangeEmailButton->setEnabled( state );
  mRemoveContactButton->setEnabled( state );
}

void DistributionListWidget::updateNameCombo()
{
  int pos = mNameCombo->currentItem();
  mNameCombo->clear();
  const QStringList names = core()->distributionListNames();
  mNameCombo->insertStringList( names );
  mNameCombo->setCurrentItem( QMIN( pos, (int)names.count() - 1 ) );

  updateContactView();
}

void DistributionListWidget::dropEvent( QDropEvent *e )
{
  KPIM::DistributionList dist = KPIM::DistributionList::findByName(
    core()->addressBook(), mNameCombo->currentText() );
  if ( dist.isEmpty() )
    return;

  QString vcards;
  if ( KVCardDrag::decode( e, vcards ) ) {
    KABC::VCardConverter converter;
    KABC::Addressee::List list = converter.parseVCards( vcards );
    KABC::Addressee::List::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it )
      dist.insertEntry( *it );

    core()->addressBook()->insertAddressee( dist );
    //updateContactView();
    changed( dist );
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

void DistributionListWidget::changed( const KABC::Addressee& dist )
{
  emit modified( KABC::Addressee::List() << dist );
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
