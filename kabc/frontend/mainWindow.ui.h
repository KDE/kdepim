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
    Addressee addressee() { return mAddressee; }
    
  private:
    Addressee mAddressee;
};

void MainWindow::updateAddressee( QListViewItem *item )
{
  AddresseeItem *addresseeItem = dynamic_cast<AddresseeItem *>( item );
  if ( !addresseeItem ) return;
    
  if (mCurrentItem ) {
    Addressee a = writeAddressee( mCurrentItem->addressee() );
    mCurrentItem->setAddressee( a );
    mAddressBook->insertAddressee( a );
  }   
  mCurrentItem = addresseeItem;
    
  readAddressee( addresseeItem->addressee() );
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
  readAddressee( Addressee() );
    
  KABC::AddressBook::Iterator it;
  for( it = mAddressBook->begin(); it != mAddressBook->end(); ++it ) {
    new AddresseeItem( mAddresseeList, (*it) );
  }
}

void MainWindow::init()
{
  mAddressBook = new KABC::AddressBook;
  mCurrentItem = 0;
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

void MainWindow::destroy()
{
  delete mAddressBook;
}

void MainWindow::readAddressee( const KABC::Addressee &a )
{
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
