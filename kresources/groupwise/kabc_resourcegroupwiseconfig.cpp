/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
*/

#include "kabc_resourcegroupwiseconfig.h"

#include "kabc_resourcegroupwise.h"
#include "kabc_groupwiseprefs.h"

#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <klineedit.h>
#include <klistview.h>
#include <kurlrequester.h>

#include <qlabel.h>
#include <qlayout.h>

using namespace KABC;

class AddressBookItem : public QCheckListItem
{
  public:
    AddressBookItem( KListView *parent, GroupWise::AddressBook ab )
      : QCheckListItem( parent, "", CheckBox ),
        mId( ab.id )
    {
      setText( 0, ab.name );
      if ( ab.isPersonal ) setText( 1, i18n("Yes") );
      else setText( 1, i18n("No") );
      if ( ab.isFrequentContacts ) setText( 2, i18n("Yes") );
      else setText( 2, i18n("No") );
    }

    QString id() const { return mId; }

  private:
    QString mId;
};

ResourceGroupwiseConfig::ResourceGroupwiseConfig( QWidget* parent,  const char* name )
  : KRES::ConfigWidget( parent, name )
{
  QGridLayout *mainLayout = new QGridLayout( this, 7, 2, 0, KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "URL:" ), this );
  mURL = new KURLRequester( this );

  mainLayout->addWidget( label, 0, 0 );
  mainLayout->addWidget( mURL, 0, 1 );

  label = new QLabel( i18n( "User:" ), this );
  mUser = new KLineEdit( this );

  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( mUser, 1, 1 );

  label = new QLabel( i18n( "Password:" ), this );
  mPassword = new KLineEdit( this );
  mPassword->setEchoMode( QLineEdit::Password );

  mainLayout->addWidget( label, 2, 0 );
  mainLayout->addWidget( mPassword, 2, 1 );

  QFrame *hline = new QFrame( this );
  hline->setFrameStyle( QFrame::HLine | QFrame::Sunken );

  mainLayout->addMultiCellWidget( hline, 3, 3, 0, 1 );

  QPushButton *updateButton = new QPushButton( i18n( "Retrieve Address Book List From Server" ), this );
  mainLayout->addMultiCellWidget( updateButton, 4, 4, 0, 1 );

  mAddressBookView = new KListView( this );
  mAddressBookView->addColumn( i18n( "Address Book" ) );
  mAddressBookView->addColumn( i18n( "Personal" ) );
  mAddressBookView->addColumn( i18n( "Frequent Contacts" ) );
  mAddressBookView->setFullWidth( true );

  mainLayout->addMultiCellWidget( mAddressBookView, 5, 5, 0, 1 );

  label = new QLabel( i18n( "Address book for new contacts:" ), this );
  mAddressBookBox = new KComboBox( this );

  mainLayout->addWidget( label, 6, 0 );
  mainLayout->addWidget( mAddressBookBox, 6, 1 );

  connect( updateButton, SIGNAL( clicked() ), SLOT( updateAddressBookList() ) );
}

void ResourceGroupwiseConfig::loadSettings( KRES::Resource *res )
{
  mResource = dynamic_cast<ResourceGroupwise*>( res );
  
  if ( !mResource ) {
    kdDebug(5700) << "ResourceGroupwiseConfig::loadSettings(): cast failed" << endl;
    return;
  }

  mURL->setURL( mResource->prefs()->url() );
  mUser->setText( mResource->prefs()->user() );
  mPassword->setText( mResource->prefs()->password() );
  mReadAddressBookIds = mResource->prefs()->readAddressBooks();
  updateAddressBookView();
}

void ResourceGroupwiseConfig::saveSettings( KRES::Resource *res )
{
  ResourceGroupwise *resource = dynamic_cast<ResourceGroupwise*>( res );
  
  if ( !resource ) {
    kdDebug(5700) << "ResourceGroupwiseConfig::saveSettings(): cast failed" << endl;
    return;
  }

  saveServerSettings( resource );

  saveAddressBookSettings();
}

void ResourceGroupwiseConfig::saveServerSettings( ResourceGroupwise *resource )
{
  resource->prefs()->setUrl( mURL->url() );
  resource->prefs()->setUser( mUser->text() );
  resource->prefs()->setPassword( mPassword->text() );  
}

void ResourceGroupwiseConfig::updateAddressBookList()
{
  saveServerSettings( mResource );

  mResource->retrieveAddressBooks();

  updateAddressBookView();
}

void ResourceGroupwiseConfig::saveAddressBookSettings()
{
  QStringList selectedRead;
  QString selectedWrite;

  QListViewItemIterator it2( mAddressBookView );
  while ( it2.current() ) {
    AddressBookItem *item = static_cast<AddressBookItem*>( it2.current() );
    if ( item->isOn() )
      selectedRead.append( item->id() );

    ++it2;
  }
  // check if the SAB was selected when the settings were loaded and is not selected now,
  // if so, clear the resource to clear the SAB data that is no longer required.
  // also, set the sequence numbers to 0 so that we know the SAB should be re-fetched in its entirety the next time we do load it
  QString sab = mResource->prefs()->systemAddressBook();
  if ( ( mReadAddressBookIds.find( sab ) != mReadAddressBookIds.end() ) && ( selectedRead.find( sab ) == selectedRead.end() ) )
  {
    mResource->clearCache();
    mResource->prefs()->setLastSequenceNumber( 0 );
    mResource->prefs()->setFirstSequenceNumber( 0 );
  }
  selectedWrite = mWriteAddressBookIds[ mAddressBookBox->currentItem() ];

  mResource->prefs()->setReadAddressBooks( selectedRead );
  mResource->prefs()->setWriteAddressBook( selectedWrite );
}

void ResourceGroupwiseConfig::updateAddressBookView()
{
  if ( mAddressBookBox->count() != 0 ) // we loaded it already
    saveAddressBookSettings();

  mAddressBookView->clear();
  mAddressBookBox->clear();
  mWriteAddressBookIds.clear();

  QStringList selectedRead = mResource->prefs()->readAddressBooks();

  GroupWise::AddressBook::List addressBooks = mResource->addressBooks();
  GroupWise::AddressBook::List::ConstIterator abIt;
  for ( abIt = addressBooks.begin(); abIt != addressBooks.end(); ++abIt ) {
    AddressBookItem *item = new AddressBookItem( mAddressBookView, *abIt );
    if ( selectedRead.find( (*abIt).id ) != selectedRead.end() )
      item->setOn( true );

    mAddressBookBox->insertItem( (*abIt).name );
    mWriteAddressBookIds.append( (*abIt).id );
  }

  int index = mWriteAddressBookIds.findIndex( mResource->prefs()->writeAddressBook() );
  mAddressBookBox->setCurrentItem( index );
}

#include "kabc_resourcegroupwiseconfig.moc"
