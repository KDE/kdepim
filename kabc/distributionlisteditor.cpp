/*
    This file is part of libkabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qlistview.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qinputdialog.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include <klocale.h>
#include <kdebug.h>

#include "addressbook.h"
#include "addresseedialog.h"
#include "distributionlist.h"

#include "distributionlisteditor.h"
#include "distributionlisteditor.moc"

using namespace KABC;

EmailSelectDialog::EmailSelectDialog( const QStringList &emails, const QString &current,
                                      QWidget *parent ) :
  KDialogBase( KDialogBase::Plain, i18n("Select Email Address"), Ok, Ok,
               parent )
{
  QFrame *topFrame = plainPage();
  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mButtonGroup = new QButtonGroup( 1, Horizontal, i18n("Email Addresses"),
                                   topFrame );
  topLayout->addWidget( mButtonGroup );

  QStringList::ConstIterator it;
  for( it = emails.begin(); it != emails.end(); ++it ) {
    QRadioButton *button = new QRadioButton( *it, mButtonGroup );
    if ( (*it) == current ) {
      button->setDown( true );
    }
  }
}

QString EmailSelectDialog::selected()
{
  QButton *button = mButtonGroup->selected();
  if ( button ) return button->text();
  return QString::null;
}

QString EmailSelectDialog::getEmail( const QStringList &emails, const QString &current,
                                     QWidget *parent )
{
  EmailSelectDialog *dlg = new EmailSelectDialog( emails, current, parent );
  dlg->exec();

  QString result = dlg->selected();

  delete dlg;

  return result;
}

class EntryItem : public QListViewItem
{
  public:
    EntryItem( QListView *parent, const Addressee &addressee,
               const QString &email=QString::null ) :
      QListViewItem( parent ),
      mAddressee( addressee ),
      mEmail( email )
    {
      setText( 0, addressee.realName() );
      if( email.isEmpty() ) {
        setText( 1, addressee.preferredEmail() );
        setText( 2, i18n("Yes") );
      } else {
        setText( 1, email );
        setText( 2, i18n("No") );
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

  private:
    Addressee mAddressee;
    QString mEmail;
};

DistributionListEditor::DistributionListEditor( AddressBook *addressBook, QWidget *parent) :
  QWidget( parent ),
  mAddressBook( addressBook )
{
  kdDebug() << "DistributionListEditor()" << endl;

  QBoxLayout *topLayout = new QVBoxLayout( this );

  QBoxLayout *nameLayout = new QHBoxLayout( topLayout) ;

  mNameCombo = new QComboBox( this );
  nameLayout->addWidget( mNameCombo );
  connect( mNameCombo, SIGNAL( activated( int ) ), SLOT( updateEntryView() ) );

  QPushButton *newButton = new QPushButton( i18n("New Argl List"), this );
  nameLayout->addWidget( newButton );
  connect( newButton, SIGNAL( clicked() ), SLOT( newList() ) );

  QPushButton *removeButton = new QPushButton( i18n("Remove List"), this );
  nameLayout->addWidget( removeButton );
  connect( removeButton, SIGNAL( clicked() ), SLOT( removeList() ) );

  mEntryView = new QListView( this );
  mEntryView->addColumn( i18n("Name") );
  mEntryView->addColumn( i18n("Email") );
  mEntryView->addColumn( i18n("Use preferred") );
  topLayout->addWidget( mEntryView );

  QPushButton *changeEmailButton = new QPushButton( i18n("Change Email"), this );
  topLayout->addWidget( changeEmailButton );
  connect( changeEmailButton, SIGNAL( clicked() ), SLOT( changeEmail() ) );

  QPushButton *removeEntryButton = new QPushButton( i18n("Remove Entry"), this );
  topLayout->addWidget( removeEntryButton );
  connect( removeEntryButton, SIGNAL( clicked() ), SLOT( removeEntry() ) );

  QPushButton *addEntryButton = new QPushButton( i18n("Add Entry"), this );
  topLayout->addWidget( addEntryButton );
  connect( addEntryButton, SIGNAL( clicked() ), SLOT( addEntry() ) );

  mAddresseeView = new QListView( this );
  mAddresseeView->addColumn( i18n("Name") );
  mAddresseeView->addColumn( i18n("Preferred Email") );
  topLayout->addWidget( mAddresseeView );

  mManager = new DistributionListManager( mAddressBook );
  mManager->load();

  updateAddresseeView();
  updateNameCombo();
}

DistributionListEditor::~DistributionListEditor()
{
  kdDebug() << "~DistributionListEditor()" << endl;

  mManager->save();
  delete mManager;
}

void DistributionListEditor::newList()
{
  bool ok = false;
  QString name = QInputDialog::getText( i18n("New Distribution List"),
                                        i18n("Please enter name."),
                                        QLineEdit::Normal, QString::null, &ok,
                                        this );
  if ( !ok || name.isEmpty() ) return;

  new DistributionList( mManager, name );

  mNameCombo->insertItem( name );

  updateEntryView();
}

void DistributionListEditor::removeList()
{
  delete mManager->list( mNameCombo->currentText() );
  mNameCombo->removeItem( mNameCombo->currentItem() );

  updateEntryView();
}

void DistributionListEditor::addEntry()
{
  AddresseeItem *addresseeItem =
      dynamic_cast<AddresseeItem *>( mAddresseeView->selectedItem() );

  if( !addresseeItem ) {
    kdDebug() << "DLE::addEntry(): No addressee selected." << endl;
    return;
  }

  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) {
    kdDebug() << "DLE::addEntry(): No dist list '" << mNameCombo->currentText() << "'" << endl;
    return;
  }

  list->insertEntry( addresseeItem->addressee() );

  updateEntryView();
}

void DistributionListEditor::removeEntry()
{
  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) return;

  EntryItem *entryItem =
      dynamic_cast<EntryItem *>( mEntryView->selectedItem() );
  if ( !entryItem ) return;

  list->removeEntry( entryItem->addressee(), entryItem->email() );
  delete entryItem;
}

void DistributionListEditor::changeEmail()
{
  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) return;

  EntryItem *entryItem =
      dynamic_cast<EntryItem *>( mEntryView->selectedItem() );
  if ( !entryItem ) return;

  QString email = EmailSelectDialog::getEmail( entryItem->addressee().emails(),
                                               entryItem->email(), this );
  list->removeEntry( entryItem->addressee(), entryItem->email() );
  list->insertEntry( entryItem->addressee(), email );

  updateEntryView();
}

void DistributionListEditor::updateEntryView()
{
  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) return;

  mEntryView->clear();
  DistributionList::Entry::List entries = list->entries();
  DistributionList::Entry::List::ConstIterator it;
  for( it = entries.begin(); it != entries.end(); ++it ) {
    new EntryItem( mEntryView, (*it).addressee, (*it).email );
  }
}

void DistributionListEditor::updateAddresseeView()
{
  mAddresseeView->clear();

  AddressBook::Iterator it;
  for( it = mAddressBook->begin(); it != mAddressBook->end(); ++it ) {
    new AddresseeItem( mAddresseeView, *it );
  }
}

void DistributionListEditor::updateNameCombo()
{
  mNameCombo->insertStringList( mManager->listNames() );

  updateEntryView();
}
