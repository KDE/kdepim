/*
    This file is part of KAddressBook.                                  
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

#include <qlayout.h>
#include <qpushbutton.h>

#include <kaccelmanager.h>
#include <kdebug.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kabc/addressbook.h>

#include "kabcore.h"

#include "mergewidget.h"

class MergeFactory : public ExtensionFactory
{
  public:
    ExtensionWidget *extension( KABCore *core, QWidget *parent, const char *name )
    {
      return new MergeWidget( core, parent, name );
    }

    QString identifier() const
    {
      return "merge";
    }
};

extern "C" {
  void *init_libkaddrbk_merge()
  {
    return ( new MergeFactory );
  }
}

class ContactItem : public QListViewItem
{
  public:
    ContactItem( KListView *parent, const KABC::Addressee &addressee )
      : QListViewItem( parent ), mAddressee( addressee )
    {
      KABC::Field::List fieldList = KABC::Field::defaultFields();
      KABC::Field::List::ConstIterator it;

      int i = 0;
      for ( it = fieldList.begin(); it != fieldList.end(); ++it )
        setText( i++, (*it)->value( mAddressee ) );
    }

    KABC::Addressee addressee() const
    {
      return mAddressee;
    }

  private:
    KABC::Addressee mAddressee;
};

MergeWidget::MergeWidget( KABCore *core, QWidget *parent, const char *name )
  : ExtensionWidget( core, parent, name ), mBlockUpdate( false )
{
  QGridLayout *topLayout = new QGridLayout( this, 3, 2, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  mContactView = new KListView( this );
  KABC::Field::List fieldList = KABC::Field::defaultFields();
  KABC::Field::List::ConstIterator it;

  for ( it = fieldList.begin(); it != fieldList.end(); ++it )
    mContactView->addColumn( (*it)->label() );

  mContactView->setEnabled( false );
  mContactView->setAllColumnsShowFocus( true );
  topLayout->addMultiCellWidget( mContactView, 0, 2, 0, 0 );

  connect( mContactView, SIGNAL( selectionChanged() ),
           SLOT( selectionContactViewChanged() ) );

  mMergeAndRemoveButton = new QPushButton( i18n( "Merge && Remove" ), this );
  mMergeAndRemoveButton->setEnabled( false );
  topLayout->addWidget( mMergeAndRemoveButton, 0, 1 );
  connect( mMergeAndRemoveButton, SIGNAL( clicked() ), SLOT( mergeAndRemove() ) );

  mMergeButton = new QPushButton( i18n( "Merge" ), this );
  mMergeButton->setEnabled( false );
  topLayout->addWidget( mMergeButton, 1, 1 );
  connect( mMergeButton, SIGNAL( clicked() ), SLOT( merge() ) );

  KAcceleratorManager::manage( this );
}

MergeWidget::~MergeWidget()
{
}

void MergeWidget::selectionContactViewChanged()
{
  ContactItem *contactItem =
                  dynamic_cast<ContactItem*>( mContactView->selectedItem() );
  bool state = (contactItem != 0);

  mMergeAndRemoveButton->setEnabled( state );
  mMergeButton->setEnabled( state );
}

void MergeWidget::contactsSelectionChanged()
{
  if ( mBlockUpdate )
    return;

  if ( !contactsSelected() ) {
    mContactView->setEnabled( false );
    mContactView->clear();
    mMergeAndRemoveButton->setEnabled( false );
    mMergeButton->setEnabled( false );
  } else {
    KABC::Addressee::List list = selectedContacts();
    if ( list.count() > 1 ) {
      mContactView->setEnabled( false );
      mContactView->clear();
      mMergeAndRemoveButton->setEnabled( false );
      mMergeButton->setEnabled( false );
      return;
    } else {
      mContactView->setEnabled( true );
      mMasterAddressee = list[ 0 ];
      updateView();
    }
  }
}

void MergeWidget::updateView()
{
  mContactView->clear();

  KABC::AddressBook::Iterator it;
  KABC::AddressBook *ab = core()->addressBook();
  if ( !ab )
    return;

  for ( it = ab->begin(); it != ab->end(); ++it )
    if ( (*it).uid() != mMasterAddressee.uid() )
      new ContactItem( mContactView, *it );
}

QString MergeWidget::title() const
{
  return i18n( "Merge Contacts Editor" );
}

QString MergeWidget::identifier() const
{
  return "merge";
}

void MergeWidget::mergeAndRemove()
{
  ContactItem *item = dynamic_cast<ContactItem*>( mContactView->currentItem() );
  if ( !item )
    return;

  QString oldUID = item->addressee().uid();

  doMerge( item->addressee() );

  KABC::Addressee::List retval;
  retval << mMasterAddressee;
  emit modified( retval );

  mBlockUpdate = true;
  core()->deleteContacts( oldUID );
  core()->setContactSelected( mMasterAddressee.uid() );
  mBlockUpdate = false;

  updateView();
}

void MergeWidget::merge()
{
  ContactItem *item = dynamic_cast<ContactItem*>( mContactView->currentItem() );
  if ( !item )
    return;

  doMerge( item->addressee() );

  KABC::Addressee::List retval;
  retval << mMasterAddressee;
  emit modified( retval );

  mBlockUpdate = true;
  core()->setContactSelected( mMasterAddressee.uid() );
  mBlockUpdate = false;

  updateView();
}

void MergeWidget::doMerge( const KABC::Addressee &addr )
{
  // ADR + LABEL
  KABC::Address::List addresses = addr.addresses();
  KABC::Address::List masterAddresses = mMasterAddressee.addresses();
  KABC::Address::List::Iterator addrIt ;
  for ( addrIt = addresses.begin(); addrIt != addresses.end(); ++addrIt ) {
    if ( !masterAddresses.contains( *addrIt ) )
      mMasterAddressee.insertAddress( *addrIt );
  }

  if ( mMasterAddressee.birthday().isNull() && !addr.birthday().isNull() )
    mMasterAddressee.setBirthday( addr.birthday() );


  // CATEGORIES
  QStringList::Iterator it;
  QStringList categories = addr.categories();
  QStringList masterCategories = mMasterAddressee.categories();
  QStringList newCategories( masterCategories );
  for ( it = categories.begin(); it != categories.end(); ++it )
    if ( !masterCategories.contains( *it ) )
      newCategories.append( *it );
  mMasterAddressee.setCategories( newCategories );

  // CLASS
  if ( !mMasterAddressee.secrecy().isValid() && addr.secrecy().isValid() )
    mMasterAddressee.setSecrecy( addr.secrecy() );

  // EMAIL
  QStringList emails = addr.emails();
  QStringList masterEmails = mMasterAddressee.emails();
  for ( it = emails.begin(); it != emails.end(); ++it )
    if ( !masterEmails.contains( *it ) )
      mMasterAddressee.insertEmail( *it, false );

  // FN
  if ( mMasterAddressee.formattedName().isEmpty() && !addr.formattedName().isEmpty() )
    mMasterAddressee.setFormattedName( addr.formattedName() );

  // GEO
  if ( !mMasterAddressee.geo().isValid() && addr.geo().isValid() )
    mMasterAddressee.setGeo( addr.geo() );

/*
  // KEY
  // LOGO
*/

  // MAILER
  if ( mMasterAddressee.mailer().isEmpty() && !addr.mailer().isEmpty() )
    mMasterAddressee.setMailer( addr.mailer() );

  // N
  if ( mMasterAddressee.assembledName().isEmpty() && !addr.assembledName().isEmpty() )
    mMasterAddressee.setNameFromString( addr.assembledName() );

  // NICKNAME
  if ( mMasterAddressee.nickName().isEmpty() && !addr.nickName().isEmpty() )
    mMasterAddressee.setNickName( addr.nickName() );

  // NOTE
  if ( mMasterAddressee.note().isEmpty() && !addr.note().isEmpty() )
    mMasterAddressee.setNote( addr.note() );

  // ORG
  if ( mMasterAddressee.organization().isEmpty() && !addr.organization().isEmpty() )
    mMasterAddressee.setOrganization( addr.organization() );

/*
  // PHOTO
*/

  // PROID
  if ( mMasterAddressee.productId().isEmpty() && !addr.productId().isEmpty() )
    mMasterAddressee.setProductId( addr.productId() );

  // REV
  if ( mMasterAddressee.revision().isNull() && !addr.revision().isNull() )
    mMasterAddressee.setRevision( addr.revision() );

  // ROLE
  if ( mMasterAddressee.role().isEmpty() && !addr.role().isEmpty() )
    mMasterAddressee.setRole( addr.role() );

  // SORT-STRING
  if ( mMasterAddressee.sortString().isEmpty() && !addr.sortString().isEmpty() )
    mMasterAddressee.setSortString( addr.sortString() );

/*
  // SOUND
*/

  // TEL
  KABC::PhoneNumber::List phones = addr.phoneNumbers();
  KABC::PhoneNumber::List masterPhones = mMasterAddressee.phoneNumbers();
  KABC::PhoneNumber::List::ConstIterator phoneIt;
  for ( phoneIt = phones.begin(); phoneIt != phones.end(); ++phoneIt )
    if ( !masterPhones.contains( *it ) )
      mMasterAddressee.insertPhoneNumber( *it );

  // TITLE
  if ( mMasterAddressee.title().isEmpty() && !addr.title().isEmpty() )
    mMasterAddressee.setTitle( addr.title() );

  // TZ
  if ( !mMasterAddressee.timeZone().isValid() && addr.timeZone().isValid() )
    mMasterAddressee.setTimeZone( addr.timeZone() );

  // UID // ignore UID

  // URL
  if ( mMasterAddressee.url().isEmpty() && !addr.url().isEmpty() )
    mMasterAddressee.setUrl( addr.url() );

  // X-
  QStringList customs = addr.customs();
  QStringList masterCustoms = mMasterAddressee.customs();
  QStringList newCustoms( masterCustoms );
  for ( it = customs.begin(); it != customs.end(); ++it )
    if ( !masterCustoms.contains( *it ) )
      newCustoms.append( *it );
  mMasterAddressee.setCustoms( newCustoms );
}

#include "mergewidget.moc"
