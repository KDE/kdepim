/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>
                  2003 Tobias Koenig <tokoe@kde.org>

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

#include "addresseditwidget.h"

#include <QtCore/QEvent>
#include <QtCore/QList>
#include <QtGui/QApplication>
#include <QtGui/QBoxLayout>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QKeyEvent>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

#include <kacceleratormanager.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <khbox.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kseparator.h>
#include <ktextedit.h>

#include "addresseeconfig.h"

class TabPressEater : public QObject
{
  public:
    TabPressEater( QObject *parent )
      : QObject( parent )
    {
      setObjectName( "TabPressEater" );
    }

  protected:
    bool eventFilter( QObject*, QEvent *event )
    {
      if ( event->type() == QEvent::KeyPress ) {
        QKeyEvent *keyEvent = (QKeyEvent*)event;
        if ( keyEvent->key() == Qt::Key_Tab ) {
          QApplication::sendEvent( parent(), event );
          return true;
        } else
          return false;
      } else {
        return false;
      }
    }
};


AddressEditWidget::AddressEditWidget( QWidget *parent, const char *name )
  : QWidget( parent )
{
  setObjectName( name );
  QBoxLayout *layout = new QVBoxLayout( this );
  layout->setSpacing( 2 );
  layout->setMargin( 4 );
  layout->setSpacing( KDialog::spacingHint() );

  mTypeCombo = new AddressTypeCombo( mAddressList, this );
  connect( mTypeCombo, SIGNAL( activated( int ) ),
           SLOT( updateAddressEdit() ) );
  layout->addWidget( mTypeCombo );

  mAddressField = new QLabel( this );
  mAddressField->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  mAddressField->setMinimumHeight( 20 );
  mAddressField->setAlignment( Qt::AlignTop );
  mAddressField->setTextFormat( Qt::PlainText );
  mAddressField->setTextInteractionFlags( 
    Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse );
  layout->addWidget( mAddressField );

  mEditButton = new QPushButton( i18nc( "street/postal", "&Edit Addresses..." ), this );
  connect( mEditButton, SIGNAL( clicked() ), this, SLOT( edit() ) );

  layout->addWidget( mEditButton );
}

AddressEditWidget::~AddressEditWidget()
{
}

void AddressEditWidget::setReadOnly( bool readOnly )
{
  mEditButton->setEnabled( !readOnly );
}

KABC::Address::List AddressEditWidget::addresses()
{
  KABC::Address::List retList;

  KABC::Address::List::ConstIterator it;
  for ( it = mAddressList.constBegin(); it != mAddressList.constEnd(); ++it )
    if ( !(*it).isEmpty() )
      retList.append( *it );

  return retList;
}

void AddressEditWidget::setAddresses( const KABC::Addressee &addr,
                                      const KABC::Address::List &list )
{
  mAddressee = addr;

  mAddressList.clear();

  // Insert types for existing numbers.
  mTypeCombo->insertTypeList( list );

  QList<KABC::Address::Type> defaultTypes;
  defaultTypes << KABC::Address::Home;
  defaultTypes << KABC::Address::Work;

  AddresseeConfig config( mAddressee );
  const QList<KABC::Address::Type> configList = config.noDefaultAddrTypes();
  QList<KABC::Address::Type>::ConstIterator it;
  for ( it = configList.constBegin(); it != configList.constEnd(); ++it )
    defaultTypes.removeAll( *it );

  // Insert default types.
  // Doing this for mPrefCombo is enough because the list is shared by all
  // combos.
  for ( it = defaultTypes.constBegin(); it != defaultTypes.constEnd(); ++it ) {
    if ( !mTypeCombo->hasType( *it ) )
      mTypeCombo->insertType( list, *it, Address( *it ) );
  }

  mTypeCombo->updateTypes();

  // find preferred address which will be shown
  KABC::Address::Type preferred( KABC::Address::Home );  // default if no preferred address set
  KABC::Address::List::ConstIterator addrIt;
  for ( addrIt = list.constBegin(); addrIt != list.constEnd(); ++addrIt )
    if ( (*addrIt).type() & KABC::Address::Pref ) {
      preferred = (*addrIt).type();
      break;
    }

  mTypeCombo->selectType( preferred );

  updateAddressEdit();
}

void AddressEditWidget::updateAddressee( const KABC::Addressee &addr )
{
  mAddressee = addr;
  updateAddressEdit();
}

void AddressEditWidget::edit()
{
  AddressEditDialog dialog( mAddressList, mTypeCombo->currentIndex(), this );
  if ( dialog.exec() ) {
    if ( dialog.changed() ) {
      mAddressList = dialog.addresses();

      bool hasHome = false, hasWork = false;
      KABC::Address::List::ConstIterator it;
      for ( it = mAddressList.constBegin(); it != mAddressList.constEnd(); ++it ) {
        if ( (*it).type() == KABC::Address::Home ) {
          if ( !(*it).isEmpty() )
            hasHome = true;
        }
        if ( (*it).type() == KABC::Address::Work ) {
          if ( !(*it).isEmpty() )
            hasWork = true;
        }
      }

      AddresseeConfig config( mAddressee );
      QList<KABC::Address::Type> configList;
      if ( !hasHome )
        configList << KABC::Address::Home;
      if ( !hasWork )
        configList << KABC::Address::Work;
      config.setNoDefaultAddrTypes( configList );

      mTypeCombo->updateTypes();
      updateAddressEdit();
      emit modified();
    }
  }
}

void AddressEditWidget::updateAddressEdit()
{
  KABC::Address::List::Iterator it = mTypeCombo->selectedElement();

  bool block = signalsBlocked();
  blockSignals( true );

  mAddressField->setText( "" );

  if ( it != mAddressList.end() ) {
    KABC::Address a = *it;
    if ( !a.isEmpty() ) {
      if ( a.type() & KABC::Address::Work && mAddressee.realName() != mAddressee.organization() ) {
        mAddressField->setText( a.formattedAddress( mAddressee.realName(),
                                   mAddressee.organization() ) );
      } else {
        mAddressField->setText( a.formattedAddress( mAddressee.realName() ) );
      }
    }
  }

  blockSignals( block );
}

AddressEditDialog::AddressEditDialog( const KABC::Address::List &list,
                                      int selected, QWidget *parent )
  : KDialog(parent),
    mPreviousAddress( 0 )
{
  setCaption( i18nc( "street/postal", "Edit Address" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( true );
  mAddressList = list;

  QWidget *page = new QWidget( this );
  setMainWidget( page );

  QGridLayout *topLayout = new QGridLayout( page );
  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( 0 );

  mTypeCombo = new AddressTypeCombo( mAddressList, page );
  topLayout->addWidget( mTypeCombo, 0, 0, 1, 2 );

  QLabel *label = new QLabel( i18nc( "<streetLabel>:", "%1:", KABC::Address::streetLabel() ), page );
  label->setAlignment( Qt::AlignTop | Qt::AlignLeft );
  topLayout->addWidget( label, 1, 0 );
  mStreetTextEdit = new KTextEdit( page );
  mStreetTextEdit->setAcceptRichText( false );
  label->setBuddy( mStreetTextEdit );
  topLayout->addWidget( mStreetTextEdit, 1, 1 );

  TabPressEater *eater = new TabPressEater( this );
  mStreetTextEdit->installEventFilter( eater );

  label = new QLabel( i18nc( "<postOfficeBoxLabel>:", "%1:", KABC::Address::postOfficeBoxLabel() ), page );
  topLayout->addWidget( label, 2 , 0 );
  mPOBoxEdit = new KLineEdit( page );
  label->setBuddy( mPOBoxEdit );
  topLayout->addWidget( mPOBoxEdit, 2, 1 );

  label = new QLabel( i18nc( "<localityLabel>:", "%1:", KABC::Address::localityLabel() ), page );
  topLayout->addWidget( label, 3, 0 );
  mLocalityEdit = new KLineEdit( page );
  label->setBuddy( mLocalityEdit );
  topLayout->addWidget( mLocalityEdit, 3, 1 );

  label = new QLabel( i18nc( "<regionLabel>:", "%1:", KABC::Address::regionLabel() ), page );
  topLayout->addWidget( label, 4, 0 );
  mRegionEdit = new KLineEdit( page );
  label->setBuddy( mRegionEdit );
  topLayout->addWidget( mRegionEdit, 4, 1 );

  label = new QLabel( i18nc( "<postalCodeLabel>:", "%1:", KABC::Address::postalCodeLabel() ), page );
  topLayout->addWidget( label, 5, 0 );
  mPostalCodeEdit = new KLineEdit( page );
  label->setBuddy( mPostalCodeEdit );
  topLayout->addWidget( mPostalCodeEdit, 5, 1 );

  label = new QLabel( i18nc( "<countryLabel>:", "%1:", KABC::Address::countryLabel() ), page );
  topLayout->addWidget( label, 6, 0 );
  mCountryCombo = new KComboBox( page );
  mCountryCombo->setEditable( true );
  mCountryCombo->setDuplicatesEnabled( false );

  QPushButton *labelButton = new QPushButton( i18n( "Edit Label..." ), page );
  topLayout->addWidget( labelButton, 7, 0, 1, 2 );
  connect( labelButton, SIGNAL( clicked() ), SLOT( editLabel() ) );

  fillCountryCombo();
  label->setBuddy( mCountryCombo );
  topLayout->addWidget( mCountryCombo, 6, 1 );

  mPreferredCheckBox = new QCheckBox( i18nc( "street/postal", "This is the preferred address" ), page );
  topLayout->addWidget( mPreferredCheckBox, 8, 0, 1, 2 );

  KSeparator *sep = new KSeparator( Qt::Horizontal, page );
  topLayout->addWidget( sep, 9, 0, 1, 2 );

  KHBox *buttonBox = new KHBox( page );
  buttonBox->setSpacing( spacingHint() );
  topLayout->addWidget( buttonBox, 10, 0, 1, 2 );

  QPushButton *addButton = new QPushButton( i18nc( "@action:button Add a new address", "New..." ), buttonBox );
  connect( addButton, SIGNAL( clicked() ), SLOT( addAddress() ) );

  mRemoveButton = new QPushButton( i18n( "Remove" ), buttonBox );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( removeAddress() ) );

  mChangeTypeButton = new QPushButton( i18n( "Change Type..." ), buttonBox );
  connect( mChangeTypeButton, SIGNAL( clicked() ), SLOT( changeType() ) );

  mTypeCombo->updateTypes();
  mTypeCombo->setCurrentIndex( selected );

  updateAddressEdits();

  connect( mTypeCombo, SIGNAL( activated( int ) ),
           SLOT( updateAddressEdits() ) );
  connect( mStreetTextEdit, SIGNAL( textChanged() ), SLOT( modified() ) );
  connect( mPOBoxEdit, SIGNAL( textChanged( const QString& ) ), SLOT( modified() ) );
  connect( mLocalityEdit, SIGNAL( textChanged( const QString& ) ), SLOT( modified() ) );
  connect( mRegionEdit, SIGNAL( textChanged( const QString& ) ), SLOT( modified() ) );
  connect( mPostalCodeEdit, SIGNAL( textChanged( const QString& ) ), SLOT( modified() ) );
  connect( mCountryCombo, SIGNAL( textChanged( const QString& ) ), SLOT( modified() ) );
  connect( mPreferredCheckBox, SIGNAL( toggled( bool ) ), SLOT( modified() ) );

  KAcceleratorManager::manage( this );

  mChanged = false;

  mRemoveButton->setEnabled( mAddressList.count() > 1 );
  mChangeTypeButton->setEnabled( mAddressList.count() > 0 );
}

AddressEditDialog::~AddressEditDialog()
{
}

KABC::Address::List AddressEditDialog::addresses()
{
  saveAddress( *(mTypeCombo->selectedElement()) );

  return mAddressList;
}

bool AddressEditDialog::changed() const
{
  return mChanged;
}

void AddressEditDialog::addAddress()
{
  AddressTypeDialog dlg( mTypeCombo->selectedType(), this );
  if ( dlg.exec() ) {
    mAddressList.append( Address( dlg.type() ) );

    mTypeCombo->updateTypes();
    mTypeCombo->setCurrentIndex( mTypeCombo->count() - 1 );
    updateAddressEdits();

    modified();

    mRemoveButton->setEnabled( true );
    mChangeTypeButton->setEnabled( true );
  }
}

void AddressEditDialog::removeAddress()
{
  if ( mAddressList.count() > 0 ) {
    KABC::Address::List::Iterator it = mTypeCombo->selectedElement();
    if ( mPreviousAddress && mPreviousAddress->id() == (*it).id() )
      mPreviousAddress = 0;

    mAddressList.erase( it );
    mTypeCombo->updateTypes();
    updateAddressEdits();

    modified();
  }

  mRemoveButton->setEnabled( mAddressList.count() > 1 );
  mChangeTypeButton->setEnabled( mAddressList.count() > 0 );
}

void AddressEditDialog::changeType()
{
  KABC::Address::List::Iterator a = mTypeCombo->selectedElement();

  AddressTypeDialog dlg( (*a).type(), this );
  if ( dlg.exec() ) {
    (*a).setType( dlg.type() );

    mTypeCombo->updateTypes();

    modified();
  }
}

void AddressEditDialog::editLabel()
{
  bool ok = false;
  QString result = KInputDialog::getMultiLineText( KABC::Address::labelLabel(),
                                                   KABC::Address::labelLabel(),
                                                   mLabel, &ok, this );
  if ( ok ) {
    mLabel = result;
    modified();
  }
}

void AddressEditDialog::updateAddressEdits()
{
  if ( mPreviousAddress )
    saveAddress( *mPreviousAddress );

  if( mTypeCombo->isEmpty())
      return;
  KABC::Address::List::Iterator it = mTypeCombo->selectedElement();
  KABC::Address a = *it;
  mPreviousAddress = &(*it);

  bool tmp = mChanged;

  mStreetTextEdit->setPlainText( a.street() );
  mRegionEdit->setText( a.region() );
  mLocalityEdit->setText( a.locality() );
  mPostalCodeEdit->setText( a.postalCode() );
  mPOBoxEdit->setText( a.postOfficeBox() );
  mCountryCombo->setItemText( mCountryCombo->currentIndex(), a.country() );
  mLabel = a.label();

  mPreferredCheckBox->setChecked( a.type() & KABC::Address::Pref );

  if ( a.isEmpty() )
    mCountryCombo->setItemText( mCountryCombo->currentIndex(),
       KGlobal::locale()->countryCodeToName( KGlobal::locale()->country() ) );

  mStreetTextEdit->setFocus();

  mChanged = tmp;
}

void AddressEditDialog::modified()
{
  mChanged = true;
}

void AddressEditDialog::saveAddress( KABC::Address &addr )
{
  addr.setLocality( mLocalityEdit->text() );
  addr.setRegion( mRegionEdit->text() );
  addr.setPostalCode( mPostalCodeEdit->text() );
  addr.setCountry( mCountryCombo->currentText() );
  addr.setPostOfficeBox( mPOBoxEdit->text() );
  addr.setStreet( mStreetTextEdit->toPlainText() );
  addr.setLabel( mLabel );


  if ( mPreferredCheckBox->isChecked() ) {
    KABC::Address::List::Iterator it;
    for ( it = mAddressList.begin(); it != mAddressList.end(); ++it )
      (*it).setType( (*it).type() & ~( KABC::Address::Pref ) );

    addr.setType( addr.type() | KABC::Address::Pref );
  } else
    addr.setType( addr.type() & ~( KABC::Address::Pref ) );
}

void AddressEditDialog::fillCountryCombo()
{
  QStringList countries;

  foreach( const QString &cc, KGlobal::locale()->allCountriesList() ) {
    countries.append( KGlobal::locale()->countryCodeToName(cc) );
  }

  countries = sortLocaleAware( countries );

  mCountryCombo->addItems( countries );
  mCountryCombo->completionObject()->setItems( countries );
  mCountryCombo->setAutoCompletion( true );
}


AddressTypeDialog::AddressTypeDialog( KABC::Address::Type type, QWidget *parent )
  : KDialog( parent)
{
  setCaption( i18nc( "street/postal", "Edit Address Type" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );

  QWidget *page = new QWidget(this);
  setMainWidget( page );
  QVBoxLayout *layout = new QVBoxLayout( page );
  layout->setSpacing( KDialog::spacingHint() );
  layout->setMargin( 0 );

  QGroupBox *box  = new QGroupBox( i18nc( "street/postal", "Address Types" ), page );
  layout->addWidget( box );
  mGroup = new QButtonGroup( box );
  mGroup->setExclusive ( false );

  QGridLayout *buttonLayout = new QGridLayout( box );

  mTypeList = KABC::Address::typeList();
  mTypeList.removeAll( KABC::Address::Pref );

  KABC::Address::TypeList::ConstIterator it;
  int i = 0;
  int row = 0;
  for ( it = mTypeList.constBegin(); it != mTypeList.constEnd(); ++it, ++i ) {
    QCheckBox *cb = new QCheckBox( KABC::Address::typeLabel( *it ), box );
    cb->setChecked( type & mTypeList[ i ] );
    buttonLayout->addWidget( cb, row, i%3 );
   
    if( i%3 == 2 )
        ++row;
    mGroup->addButton( cb );
  }
}

AddressTypeDialog::~AddressTypeDialog()
{
}

KABC::Address::Type AddressTypeDialog::type() const
{
  KABC::Address::Type type;
  for ( int i = 0; i < mGroup->buttons().count(); ++i ) {
    QCheckBox *box = dynamic_cast<QCheckBox*>( mGroup->buttons().at( i ) );
    if ( box && box->isChecked() )
      type |= mTypeList[ i ];
  }

  return type;
}

/**
  Small helper class, I hope we can remove it as soon as a general solution has
  been committed to kdelibs
 */
class LocaleAwareString : public QString
{
  public:
    LocaleAwareString() : QString()
    {}

    LocaleAwareString( const QString &str ) : QString( str )
    {}
};

static bool operator<( const LocaleAwareString &s1, const LocaleAwareString &s2 )
{
  return ( QString::localeAwareCompare( s1, s2 ) < 0 );
}

QStringList AddressEditDialog::sortLocaleAware( const QStringList &list )
{
  QList<LocaleAwareString> sortedList;

  QStringList::ConstIterator it;
  for ( it = list.constBegin(); it != list.constEnd(); ++it )
    sortedList.append( LocaleAwareString( *it ) );

  qSort( sortedList.begin(), sortedList.end() );

  QStringList retval;
  QList<LocaleAwareString>::ConstIterator retIt;
  for ( retIt = sortedList.constBegin(); retIt != sortedList.constEnd(); ++retIt )
    retval.append( *retIt );

  return retval;
}

#include "addresseditwidget.moc"
