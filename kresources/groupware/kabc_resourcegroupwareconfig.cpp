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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "kabc_resourcegroupwareconfig.h"

#include "kabc_resourcegroupware.h"
#include "kabc_groupwareprefs.h"

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
    AddressBookItem( KListView *parent, const QString &id )
      : QCheckListItem( parent, "", CheckBox ),
        mId( id )
    {
      setText( 0, mId );
#if 0
      if ( ab.isPersonal ) setText( 1, "Yes" );
      else setText( 1, "No" );
      if ( ab.isFrequentContacts ) setText( 2, "Yes" );
      else setText( 2, "No" );
#endif
    }

    QString id() const { return mId; }

  private:
    QString mId;
};

ResourceGroupwareConfig::ResourceGroupwareConfig( QWidget* parent,  const char* name )
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

  QPushButton *updateButton = new QPushButton( i18n( "Retrieve Address Book List from Server" ), this );
  mainLayout->addMultiCellWidget( updateButton, 4, 4, 0, 1 );

  mAddressBookView = new KListView( this );
  mAddressBookView->addColumn( i18n( "Address Book" ) );
#if 0
  mAddressBookView->addColumn( i18n( "Personal" ) );
  mAddressBookView->addColumn( i18n( "Frequent Contacts" ) );
#endif
  mAddressBookView->setFullWidth( true );

  mainLayout->addMultiCellWidget( mAddressBookView, 5, 5, 0, 1 );

  label = new QLabel( i18n( "Address Book for new Contacts:" ), this );
  mAddressBookBox = new KComboBox( this );

  mainLayout->addWidget( label, 6, 0 );
  mainLayout->addWidget( mAddressBookBox, 6, 1 );

  connect( updateButton, SIGNAL( clicked() ), SLOT( updateAddressBookList() ) );
}

void ResourceGroupwareConfig::loadSettings( KRES::Resource *res )
{
  mResource = dynamic_cast<ResourceGroupware*>( res );
  
  if ( !mResource ) {
    kdDebug(5700) << "ResourceGroupwareConfig::loadSettings(): cast failed" << endl;
    return;
  }

  mURL->setURL( mResource->prefs()->url() );
  mUser->setText( mResource->prefs()->user() );
  mPassword->setText( mResource->prefs()->password() );

  updateAddressBookView();
}

void ResourceGroupwareConfig::saveSettings( KRES::Resource *res )
{
  ResourceGroupware *resource = dynamic_cast<ResourceGroupware*>( res );
  
  if ( !resource ) {
    kdDebug(5700) << "ResourceGroupwareConfig::saveSettings(): cast failed" << endl;
    return;
  }

  saveServerSettings( resource );

  saveAddressBookSettings();
}

void ResourceGroupwareConfig::saveServerSettings( ResourceGroupware *resource )
{
  resource->prefs()->setUrl( mURL->url() );
  resource->prefs()->setUser( mUser->text() );
  resource->prefs()->setPassword( mPassword->text() );  
}

void ResourceGroupwareConfig::updateAddressBookList()
{
  saveServerSettings( mResource );

  mResource->retrieveAddressBooks();

  updateAddressBookView();
}

void ResourceGroupwareConfig::saveAddressBookSettings()
{
#if 0
  QStringList selectedRead;
  QString selectedWrite;

  QListViewItemIterator it2( mAddressBookView );
  while ( it2.current() ) {
    AddressBookItem *item = static_cast<AddressBookItem*>( it2.current() );
    if ( item->isOn() )
      selectedRead.append( item->id() );

    ++it2;
  }

  selectedWrite = mWriteAddressBookIds[ mAddressBookBox->currentItem() ];

  mResource->prefs()->setReadAddressBooks( selectedRead );
  mResource->prefs()->setWriteAddressBook( selectedWrite );
#endif
}

void ResourceGroupwareConfig::updateAddressBookView()
{
  if ( mAddressBookBox->count() != 0 ) // we loaded it already
    saveAddressBookSettings();

  mAddressBookView->clear();
  mAddressBookBox->clear();
  mWriteAddressBookIds.clear();

#if 0
  QStringList selectedRead = mResource->prefs()->readAddressBooks();

  Groupware::AddressBook::List addressBooks = mResource->addressBooks();
  Groupware::AddressBook::List::ConstIterator abIt;
  for ( abIt = addressBooks.begin(); abIt != addressBooks.end(); ++abIt ) {
    AddressBookItem *item = new AddressBookItem( mAddressBookView, *abIt );
    if ( selectedRead.find( (*abIt).id ) != selectedRead.end() )
      item->setOn( true );

    mAddressBookBox->insertItem( (*abIt).name );
    mWriteAddressBookIds.append( (*abIt).id );
  }

  int index = mWriteAddressBookIds.findIndex( mResource->prefs()->writeAddressBook() );
  mAddressBookBox->setCurrentItem( index );
#endif
}

#include "kabc_resourcegroupwareconfig.moc"
