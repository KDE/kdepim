/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <qfiledialog.h>
#include <qinputdialog.h>
#include <qmessagebox.h>

#include <klocale.h>
#include <kdebug.h>

using namespace KABC;

class AddresseeItem : public QListViewItem
{
  public:
    AddresseeItem( QListView *parent, const Addressee &a ) :
      QListViewItem( parent, a.name() ), mAddressee( a ) {}
      
    void setAddressee( const Addressee &a ) { mAddressee = a; }
    Addressee &addressee() { return mAddressee; }
    
  private:
    Addressee mAddressee;
};

void MainWindow::init()
{
  mAddressBook = new KABC::AddressBook;
  mCurrentItem = 0;
}

void MainWindow::destroy()
{
  delete mAddressBook;
}

void MainWindow::fileSave()
{
  updateAddressee( mAddresseeList->selectedItem() );

  if ( mAddressBook->fileName().isEmpty() ) {
    QString fn = QFileDialog::getSaveFileName();
    if ( fn.isEmpty() ) return;
    mAddressBook->setFileName( fn );
  }
    
  AddressBook::Ticket *ticket = mAddressBook->requestSaveTicket();
  if ( !ticket ) {
    QMessageBox::critical( this, i18n("Save Error"), i18n("Can't save file '%1'.\n"
      "File is locked by another process."), QMessageBox::Ok, QMessageBox::NoButton,
      QMessageBox::NoButton );
    return;
  }
  mAddressBook->save( ticket );
}

void MainWindow::fileOpen()
{
  QString fileName = QFileDialog::getOpenFileName();
    
  if ( !mAddressBook->load( fileName ) ) return;
   
  mAddresseeList->clear();
  mCurrentItem = 0;
  mCurrentAddress = QString::null;
  readAddressee( Addressee() );
    
  KABC::AddressBook::Iterator it;
  for( it = mAddressBook->begin(); it != mAddressBook->end(); ++it ) {
    new AddresseeItem( mAddresseeList, (*it) );
  }
}

void MainWindow::updateAddressee( QListViewItem *item )
{
  AddresseeItem *addresseeItem = dynamic_cast<AddresseeItem *>( item );
  if ( !addresseeItem ) return;
    
  if (mCurrentItem ) {
    writeAddress( mCurrentAddress );  
    Addressee a = writeAddressee( mCurrentItem->addressee() );
    mCurrentItem->setAddressee( a );
    mAddressBook->insertAddressee( a );
  }
  mCurrentItem = addresseeItem;

  readAddressee( addresseeItem->addressee() );
  updateAddress( mAddressIdCombo->currentItem() );
}


void MainWindow::readAddressee( const KABC::Addressee &a )
{
    kdDebug() << "MainWindow::readAddressee(): " << a.name() << endl;  
    
  mNameEdit->setText( a.name() );
  mEmailEdit->setText( a.email() );
  PhoneNumber::List pl = a.phoneNumbers();
  if ( pl.count() > 0 ) {
    PhoneNumber p = *( pl.begin() );
    mPhoneNumberEdit->setText( p.number() );
    mPhoneNumberCombo->setCurrentItem( int( p.type() ) );
  } else {
    mPhoneNumberEdit->setText( QString::null );
    mPhoneNumberCombo->setCurrentItem( 0 );
  }
  mUrlEdit->setText( a.url().url() );
  mAdditionalNameEdit->setText( a.additionalName() );
  mSuffixEdit->setText( a.suffix() );
  mGivenNameEdit->setText( a.givenName() );
  mPrefixEdit->setText( a.prefix() );
  mFamilyNameEdit->setText( a.familyName() );
  mFormattedNameEdit->setText( a.formattedName() );
  mNickNameEdit->setText( a.nickName() );
  mSortStringEdit->setText( a.sortString() );
  mTitleEdit->setText( a.title() );
  mRoleEdit->setText( a.role() );
  mOrganizationEdit->setText( a.organization() );
  mNoteEdit->setText( a.note() );
//  mLabelEdit->setText( a.label() );
  
  Address::List addresses = a.addresses();
  mAddressIdCombo->clear();
  Address::List::ConstIterator it;
  for( it = addresses.begin(); it != addresses.end(); ++it ) {
    mAddressIdCombo->insertItem( (*it).id() );
  }
  if ( mAddressIdCombo->count() > 0 ) mCurrentAddress = mAddressIdCombo->currentText();
  else mCurrentAddress = QString::null;
  readAddress( mCurrentAddress );
}

KABC::Addressee MainWindow::writeAddressee( const KABC::Addressee &addressee )
{
  Addressee a( addressee );
  a.setName( mNameEdit->text() );
  a.setEmail( mEmailEdit->text() );
  if ( !mPhoneNumberEdit->text().isEmpty() ) {
    PhoneNumber p( mPhoneNumberEdit->text(),
                   PhoneNumber::Type( mPhoneNumberCombo->currentItem() ) );
    a.insertPhoneNumber( p );
  }
 
  if ( !mUrlEdit->text().isEmpty() ) {
    a.setUrl( KURL( mUrlEdit->text() ) );
  }
  
  a.setAdditionalName( mAdditionalNameEdit->text() );
  a.setSuffix( mSuffixEdit->text() );
  a.setGivenName( mGivenNameEdit->text() );
  a.setPrefix( mPrefixEdit->text() );
  a.setFamilyName( mFamilyNameEdit->text() );
  a.setFormattedName( mFormattedNameEdit->text() );
  a.setNickName( mNickNameEdit->text() );
  a.setSortString( mSortStringEdit->text() );
  a.setTitle( mTitleEdit->text() );
  a.setRole( mRoleEdit->text() );
  a.setOrganization( mOrganizationEdit->text() );
  a.setNote( mNoteEdit->text() );
//  a.setLabel( mLabelEdit->text() );
  
  kdDebug() << "MainWindow::writeAddressee()" << endl;
  a.dump();
    
  return a;
}

void MainWindow::newEntry()
{
  bool ok = false;
  QString name = QInputDialog::getText( i18n("New Address Book Entry"),
                                        i18n("Please enter name."),
                                        QLineEdit::Normal, QString::null, &ok,
                                        this );
  if ( !ok || name.isEmpty() ) return;
  
  Addressee a;
  a.setName( name );
  mAddressBook->insertAddressee( a );
  
  new AddresseeItem( mAddresseeList, a );
}

void MainWindow::removeEntry()
{
  AddresseeItem *item = dynamic_cast<AddresseeItem *>(mAddresseeList->selectedItem());
  if ( item ) {
    mAddressBook->removeAddressee( item->addressee() );
    delete item;
    mCurrentItem = 0;
  }
}


void MainWindow::updateAddress( int id )
{
  if( !mCurrentItem ) return;
  
  writeAddress( mCurrentAddress );
  if ( mAddressIdCombo->count()  > 0 ) {
    mCurrentAddress = mAddressIdCombo->text( id );
  } else {
    mCurrentAddress = QString::null;
  }
  readAddress( mCurrentAddress );
}

KABC::Address MainWindow::writeAddress( const KABC::Address &address )
{
  Address a( address );
  
  a.setPostOfficeBox( mAddressPostOfficeBoxEdit->text() );
  a.setExtended( mAddressExtendedEdit->text() );
  a.setStreet( mAddressStreetEdit->text() );
  a.setLocality( mAddressLocalityEdit->text() );
  a.setRegion( mAddressRegionEdit->text() );
  a.setLabel( mAddressLabelEdit->text() );
  a.setPostalCode( mAddressPostalCodeEdit->text() );
  a.setCountry( mAddressCountryEdit->text() );
  
  int type = 0;
  if ( mAddressDomCheck->isChecked() ) type |= Address::Dom;
  if ( mAddressIntlCheck->isChecked() ) type |= Address::Intl;
  if ( mAddressParcelCheck->isChecked() ) type |= Address::Parcel;
  if ( mAddressPostalCheck->isChecked() ) type |= Address::Postal;
  if ( mAddressHomeCheck->isChecked() ) type |= Address::Home;
  if ( mAddressPrefCheck->isChecked() ) type |= Address::Pref;
  if ( mAddressWorkCheck->isChecked() ) type |= Address::Work;
  a.setType( type );
  
  return a;
}

void MainWindow::writeAddress( const QString &id )
{
  if ( !mCurrentItem ) return;
 
  if ( id.isEmpty() ) return;
 
  Address address;
  address.setId( id );
  address = writeAddress( address );
 
  mCurrentItem->addressee().insertAddress( address );
}

void MainWindow::readAddress( const KABC::Address &a )
{
  mAddressPostOfficeBoxEdit->setText( a.postOfficeBox() );
  mAddressExtendedEdit->setText( a.extended() );
  mAddressStreetEdit->setText( a.street() );
  mAddressLocalityEdit->setText( a.locality() );
  mAddressRegionEdit->setText( a.region() );
  mAddressLabelEdit->setText( a.label() );
  mAddressPostalCodeEdit->setText( a.postalCode() );
  mAddressCountryEdit->setText( a.country() );
  
  int type = a.type();
  if ( type & Address::Dom ) mAddressDomCheck->setChecked( true );
  else mAddressDomCheck->setChecked( false );
  if ( type & Address::Intl ) mAddressIntlCheck->setChecked( true );
  else mAddressIntlCheck->setChecked( false );
  if ( type & Address::Parcel ) mAddressParcelCheck->setChecked( true );
  else mAddressParcelCheck->setChecked( false );
  if ( type & Address::Postal ) mAddressPostalCheck->setChecked( true );
  else mAddressPostalCheck->setChecked( false );
  if ( type & Address::Home ) mAddressHomeCheck->setChecked( true );
  else mAddressHomeCheck->setChecked( false );
  if ( type & Address::Pref ) mAddressPrefCheck->setChecked( true );
  else mAddressPrefCheck->setChecked( false );
  if ( type & Address::Work ) mAddressWorkCheck->setChecked( true );
  else mAddressWorkCheck->setChecked( false );
}

void MainWindow::readAddress( const QString &id )
{
  if ( !mCurrentItem || id.isEmpty() ) {
    readAddress( Address() );
    return;
  }

  Address address;
  address.setId( id );
  
  address = mCurrentItem->addressee().findAddress( address );
  readAddress( address );
}


void MainWindow::newAddress()
{
  if( !mCurrentItem ) return;
  
  Address address;
  mCurrentItem->addressee().insertAddress( address );
  
  mAddressIdCombo->insertItem( address.id() );
}

void MainWindow::removeAddress()
{
  if ( !mCurrentItem ) return;
  
  QString id = mAddressIdCombo->currentText();
  if ( id.isEmpty() ) return;
 	 
  Address address;
  address.setId( id );
  mCurrentItem->addressee().removeAddress( address );
  
  readAddressee( mCurrentItem->addressee() );
}


void MainWindow::dumpAddressBook()
{
  mAddressBook->dump();
}
