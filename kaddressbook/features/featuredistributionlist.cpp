/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mirko Boehm <mirko@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlayout.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qdragobject.h>
#include <qtooltip.h>

#include <kdialog.h>
#include <klocale.h>
#include <kabc/distributionlist.h>
#include <kabc/distributionlisteditor.h>
#include <kabc/vcardconverter.h>
#include <klineeditdlg.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <libkdepim/kvcarddrag.h>

#include "addresseeutil.h"
#include "featuredistributionlist.h"
#include "featuredistributionlistview.h"

namespace KABC
{

// MOSTLY A COPY FROM kdelibs/kabc:
class EntryItem : public QListViewItem
{
  protected:
    FeatureDistributionList *list;

  public:
    EntryItem( FeatureDistributionList *l, QListView *parent,
               const Addressee &addressee, const QString &email = QString::null )
      : QListViewItem( parent ), list( l ), mAddressee( addressee ), mEmail( email )
    {
      setDropEnabled( true );
      setText( 0, addressee.realName() );
      if ( email.isEmpty() ) {
        setText( 1, addressee.preferredEmail() );
        setText( 2, i18n( "Yes" ) );
      } else {
        setText( 1, email );
        setText( 2, i18n( "No" ) );
      }
    }

    Addressee addressee() const
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
      // WORK_TO_DO: check data type
      return true;
    }

    void dropped( QDropEvent *e )
    {
      list->slotDropped( e );
    }

  private:
    Addressee mAddressee;
    QString mEmail;
};

}

FeatureDistributionList::FeatureDistributionList( KABC::AddressBook *doc,
                                            QWidget *parent, const char* name )
  : QWidget( parent, name ),
    mDoc( doc ),
    mManager( new KABC::DistributionListManager( doc ) )
{
  initGUI();

  connect( mLvAddressees, SIGNAL(selectionChanged()),
           SLOT(slotAddresseeSelectionChanged()));
  connect( mLvAddressees, SIGNAL(dropped(QDropEvent*)),
           SLOT(slotDropped(QDropEvent*)));

  mLvAddressees->addColumn( i18n( "Name" ) );
  mLvAddressees->addColumn( i18n( "Email" ) );
  mLvAddressees->addColumn( i18n( "Use Preferred" ) );

  mManager->load();
}

FeatureDistributionList::~FeatureDistributionList()
{
  delete mManager;
}

void FeatureDistributionList::update()
{
  int index = mCbListSelect->currentItem();

  mLvAddressees->clear();
  mCbListSelect->clear();
  mCbListSelect->insertStringList(mManager->listNames());

  if ( index < mCbListSelect->count() ) {
    mCbListSelect->setCurrentItem( index );
  }

  updateGUI();
}

void FeatureDistributionList::updateGUI()
{
  KABC::DistributionList *list = mManager->list( mCbListSelect->currentText() );
  if( !list ) {
    mPbListRename->setEnabled( false );
    mPbListRemove->setEnabled( false );
    mPbChangeEmail->setEnabled( false );
    mPbEntryRemove->setEnabled( false );
    mLvAddressees->setEnabled( false );
    mLvAddressees->clear();
    return;
  } else {
    mPbListRename->setEnabled( true );
    mPbListRemove->setEnabled( true );
    mLvAddressees->setEnabled( true );
    KABC::DistributionList::Entry::List entries = list->entries();
    KABC::DistributionList::Entry::List::ConstIterator it;
    for( it = entries.begin(); it != entries.end(); ++it ) {
      new KABC::EntryItem( this, mLvAddressees, (*it).addressee, (*it).email );
    }
  }

  KABC::EntryItem *entryItem = static_cast<KABC::EntryItem *>( mLvAddressees->selectedItem() );

  bool state = entryItem;
  mPbChangeEmail->setEnabled( state );
  mPbEntryRemove->setEnabled( state );
}

void FeatureDistributionList::showEvent( QShowEvent* )
{
  update();
}

void FeatureDistributionList::slotListNew()
{
  KLineEditDlg dlg( i18n( "Please enter name:" ), QString::null, this );
  dlg.setCaption( i18n("New Distribution List") );

  if ( !dlg.exec() )
    return;

  new KABC::DistributionList( mManager, dlg.text() );

  mCbListSelect->clear();
  mCbListSelect->insertStringList( mManager->listNames() );
  mCbListSelect->setCurrentItem( mCbListSelect->count() - 1 );

  commit();
  update();
}

void FeatureDistributionList::slotListRename()
{
  QString oldName = mCbListSelect->currentText();

  KLineEditDlg dlg( i18n( "Please change name:" ), oldName, this );
  dlg.setCaption( i18n( "Distribution List" ) );

  if ( !dlg.exec() )
    return;

  KABC::DistributionList *list = mManager->list( oldName );
  list->setName( dlg.text() );

  mCbListSelect->clear();
  mCbListSelect->insertStringList( mManager->listNames() );
  mCbListSelect->setCurrentItem( mCbListSelect->count() - 1 );

  commit();
  update();
}

void FeatureDistributionList::slotListRemove()
{
  int result = KMessageBox::warningContinueCancel( this,
                  i18n( "Delete distibution list '%1'?" ).arg( mCbListSelect->currentText() ),
                  QString::null, i18n( "Delete" ) );

  if ( result != KMessageBox::Continue)
    return;

  delete mManager->list( mCbListSelect->currentText() );
  mCbListSelect->removeItem( mCbListSelect->currentItem() );

  commit();
  updateGUI();
}

void FeatureDistributionList::slotEntryChangeEmail()
{
  KABC::DistributionList *list = mManager->list( mCbListSelect->currentText() );
  if ( !list )
    return;

  KABC::EntryItem *entryItem = static_cast<KABC::EntryItem *>( mLvAddressees->selectedItem() );
  if ( !entryItem )
    return;

  QString email = KABC::EmailSelectDialog::getEmail( entryItem->addressee().emails(),
                                                     entryItem->email(), this );
  list->removeEntry( entryItem->addressee(), entryItem->email() );
  list->insertEntry( entryItem->addressee(), email );

  commit();
  update();
}

void FeatureDistributionList::slotEntryRemove()
{
  KABC::DistributionList *list = mManager->list( mCbListSelect->currentText() );
  if ( !list )
    return;

  KABC::EntryItem *entryItem = static_cast<KABC::EntryItem *>( mLvAddressees->selectedItem() );
  if ( !entryItem )
    return;

  list->removeEntry( entryItem->addressee(), entryItem->email() );
  delete entryItem;

  commit();
}

void FeatureDistributionList::slotListSelected( int )
{
  update();
}

void FeatureDistributionList::slotAddresseeSelectionChanged()
{
  KABC::EntryItem *entryItem = static_cast<KABC::EntryItem *>( mLvAddressees->selectedItem() );
  bool state = entryItem;

  mPbChangeEmail->setEnabled( state );
  mPbEntryRemove->setEnabled( state );
}

void FeatureDistributionList::commit()
{
  mManager->save();
  emit modified();
}

void FeatureDistributionList::dropEvent( QDropEvent *e )
{
  KABC::DistributionList *distributionList = mManager->list( mCbListSelect->currentText() );
  if ( !distributionList ) {
    kdDebug(5700) << "FeatureDistributionList::dropEvent: No dist list '"
                  << mCbListSelect->currentText() << "'" << endl;
    return;
  }

  QString vcard;
  if ( KVCardDrag::decode( e, vcard ) ) {
    KABC::Addressee addr;
    KABC::VCardConverter converter;
    if ( converter.vCardToAddressee( vcard, addr ) ) {
      distributionList->insertEntry( addr );

      commit();
      update();
    }
  }
}

void FeatureDistributionList::slotDropped( QDropEvent *e )
{
  dropEvent( e );
}

void FeatureDistributionList::initGUI()
{
  setCaption( i18n( "Edit Distribution Lists" ) );

  QGridLayout *layout = new QGridLayout( this, 1, 1, KDialog::marginHint(), KDialog::spacingHint() );
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  layout->addMultiCell( spacer, 3, 4, 2, 2 );

  mCbListSelect = new QComboBox( false, this );
  layout->addWidget( mCbListSelect, 0, 0 );

  mPbListRename = new QPushButton( i18n( "Rename List..." ), this );
  layout->addWidget( mPbListRename, 2, 0 );

  mPbListRemove = new QPushButton( i18n( "Remove List" ), this );
  layout->addWidget( mPbListRemove, 3, 0 );

  QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  layout->addItem( spacer_2, 4, 0 );

  mPbChangeEmail = new QPushButton( i18n( "Change Email..." ), this );
  layout->addWidget( mPbChangeEmail, 0, 2 );

  mPbEntryRemove = new QPushButton( i18n( "Remove Entry" ), this );
  layout->addWidget( mPbEntryRemove, 1, 2 );

  mPbListNew = new QPushButton( i18n( "New List..." ), this );
  layout->addWidget( mPbListNew, 1, 0 );

  mLvAddressees = new FeatureDistributionListView( this );
  layout->addMultiCellWidget( mLvAddressees, 0, 4, 1, 1 );
  QToolTip::add(mLvAddressees, i18n("Drag addressees here to add them to the distribution list."));

  resize( QSize(760, 500).expandedTo(sizeHint()) );

  // signals and slots connections
  connect( mPbListNew, SIGNAL( clicked() ), this, SLOT( slotListNew() ) );
  connect( mPbListRename, SIGNAL( clicked() ), this, SLOT( slotListRename() ) );
  connect( mPbListRemove, SIGNAL( clicked() ), this, SLOT( slotListRemove() ) );
  connect( mPbChangeEmail, SIGNAL( clicked() ), this, SLOT( slotEntryChangeEmail() ) );
  connect( mPbEntryRemove, SIGNAL( clicked() ), this, SLOT( slotEntryRemove() ) );
  connect( mCbListSelect, SIGNAL( activated(int) ), this, SLOT( slotListSelected(int) ) );
}

#include "featuredistributionlist.moc"
