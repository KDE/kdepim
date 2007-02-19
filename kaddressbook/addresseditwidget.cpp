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

#include <q3buttongroup.h>
#include <QCheckBox>

#include <QLabel>
#include <QLayout>
#include <q3listbox.h>
#include <q3listview.h>
#include <QPushButton>
#include <q3signal.h>
#include <QString>
#include <QTextEdit>
#include <QToolButton>
#include <QToolTip>
//Added by qt3to4:
#include <QGridLayout>
#include <QKeyEvent>
#include <QEvent>
#include <QFrame>
#include <QBoxLayout>
#include <QList>
#include <QVBoxLayout>

#include <kacceleratormanager.h>
#include <k3activelabel.h>
#include <kapplication.h>
#include <kbuttonbox.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <k3listview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kseparator.h>
#include <kvbox.h>

#include "addresseditwidget.h"

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

  mAddressField = new K3ActiveLabel( this );
  mAddressField->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  mAddressField->setMinimumHeight( 20 );
  mAddressField->setAlignment( Qt::AlignTop );
  mAddressField->setAcceptRichText( false );
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
  for ( it = mAddressList.begin(); it != mAddressList.end(); ++it )
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

  QList<int> defaultTypes;
  defaultTypes << KABC::Address::Home;
  defaultTypes << KABC::Address::Work;

  AddresseeConfig config( mAddressee );
  const QList<int> configList = config.noDefaultAddrTypes();
  QList<int>::ConstIterator it;
  for ( it = configList.begin(); it != configList.end(); ++it )
    defaultTypes.removeAll( *it );

  // Insert default types.
  // Doing this for mPrefCombo is enough because the list is shared by all
  // combos.
  for ( it = defaultTypes.begin(); it != defaultTypes.end(); ++it ) {
    if ( !mTypeCombo->hasType( *it ) )
      mTypeCombo->insertType( list, *it, Address( *it ) );
  }

  mTypeCombo->updateTypes();

  // find preferred address which will be shown
  int preferred = KABC::Address::Home;  // default if no preferred address set
  KABC::Address::List::ConstIterator addrIt;
  for ( addrIt = list.begin(); addrIt != list.end(); ++addrIt )
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
      for ( it = mAddressList.begin(); it != mAddressList.end(); ++it ) {
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
      QList<int> configList;
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

  mAddressField->setPlainText( "" );

  if ( it != mAddressList.end() ) {
    KABC::Address a = *it;
    if ( !a.isEmpty() ) {
      if ( a.type() & KABC::Address::Work && mAddressee.realName() != mAddressee.organization() ) {
        mAddressField->setPlainText( a.formattedAddress( mAddressee.realName(),
                                   mAddressee.organization() ) );
      } else {
        mAddressField->setPlainText( a.formattedAddress( mAddressee.realName() ) );
      }
    }
  }

  blockSignals( block );
}

AddressEditDialog::AddressEditDialog( const KABC::Address::List &list,
                                      int selected, QWidget *parent,
                                      const char *name )
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

  mTypeCombo = new AddressTypeCombo( mAddressList, page );
  topLayout->addWidget( mTypeCombo, 0, 0, 1, 2 );

  QLabel *label = new QLabel( i18nc( "<streetLabel>:", "%1:", KABC::Address::streetLabel() ), page );
  label->setAlignment( Qt::AlignTop | Qt::AlignLeft );
  topLayout->addWidget( label, 1, 0 );
  mStreetTextEdit = new QTextEdit( page );
  mStreetTextEdit->setTextFormat( Qt::PlainText );
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

  QPushButton *addButton = new QPushButton( i18n( "New..." ), buttonBox );
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

  bool state = (mAddressList.count() > 0);
  mRemoveButton->setEnabled( state );
  mChangeTypeButton->setEnabled( state );
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

  bool state = ( mAddressList.count() > 0 );
  mRemoveButton->setEnabled( state );
  mChangeTypeButton->setEnabled( state );
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
       KGlobal::locale()->twoAlphaToCountryName( KGlobal::locale()->country() ) );

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
  QString country[] = {
    i18n( "Afghanistan" ), i18n( "Albania" ), i18n( "Algeria" ),
    i18n( "American Samoa" ), i18n( "Andorra" ), i18n( "Angola" ),
    i18n( "Anguilla" ), i18n( "Antarctica" ), i18n( "Antigua and Barbuda" ),
    i18n( "Argentina" ), i18n( "Armenia" ), i18n( "Aruba" ),
    i18n( "Ashmore and Cartier Islands" ), i18n( "Australia" ),
    i18n( "Austria" ), i18n( "Azerbaijan" ), i18n( "Bahamas" ),
    i18n( "Bahrain" ), i18n( "Bangladesh" ), i18n( "Barbados" ),
    i18n( "Belarus" ), i18n( "Belgium" ), i18n( "Belize" ),
    i18n( "Benin" ), i18n( "Bermuda" ), i18n( "Bhutan" ),
    i18n( "Bolivia" ), i18n( "Bosnia and Herzegovina" ), i18n( "Botswana" ),
    i18n( "Brazil" ), i18n( "Brunei" ), i18n( "Bulgaria" ),
    i18n( "Burkina Faso" ), i18n( "Burundi" ), i18n( "Cambodia" ),
    i18n( "Cameroon" ), i18n( "Canada" ), i18n( "Cape Verde" ),
    i18n( "Cayman Islands" ), i18n( "Central African Republic" ),
    i18n( "Chad" ), i18n( "Chile" ), i18n( "China" ), i18n( "Colombia" ),
    i18n( "Comoros" ), i18n( "Congo" ), i18n( "Congo, Dem. Rep." ),
    i18n( "Costa Rica" ), i18n( "Croatia" ),
    i18n( "Cuba" ), i18n( "Cyprus" ), i18n( "Czech Republic" ),
    i18n( "Denmark" ), i18n( "Djibouti" ),
    i18n( "Dominica" ), i18n( "Dominican Republic" ), i18n( "Ecuador" ),
    i18n( "Egypt" ), i18n( "El Salvador" ), i18n( "Equatorial Guinea" ),
    i18n( "Eritrea" ), i18n( "Estonia" ), i18n( "England" ),
    i18n( "Ethiopia" ), i18n( "European Union" ), i18n( "Faroe Islands" ),
    i18n( "Fiji" ), i18n( "Finland" ), i18n( "France" ),
    i18n( "French Polynesia" ), i18n( "Gabon" ), i18n( "Gambia" ),
    i18n( "Georgia" ), i18n( "Germany" ), i18n( "Ghana" ),
    i18n( "Greece" ), i18n( "Greenland" ), i18n( "Grenada" ),
    i18n( "Guam" ), i18n( "Guatemala" ), i18n( "Guinea" ),
    i18n( "Guinea-Bissau" ), i18n( "Guyana" ), i18n( "Haiti" ),
    i18n( "Honduras" ), i18n( "Hong Kong" ), i18n( "Hungary" ),
    i18n( "Iceland" ), i18n( "India" ), i18n( "Indonesia" ),
    i18n( "Iran" ), i18n( "Iraq" ), i18n( "Ireland" ),
    i18n( "Israel" ), i18n( "Italy" ), i18n( "Ivory Coast" ),
    i18n( "Jamaica" ), i18n( "Japan" ), i18n( "Jordan" ),
    i18n( "Kazakhstan" ), i18n( "Kenya" ), i18n( "Kiribati" ),
    i18n( "Korea, North" ), i18n( "Korea, South" ),
    i18n( "Kuwait" ), i18n( "Kyrgyzstan" ), i18n( "Laos" ),
    i18n( "Latvia" ), i18n( "Lebanon" ), i18n( "Lesotho" ),
    i18n( "Liberia" ), i18n( "Libya" ), i18n( "Liechtenstein" ),
    i18n( "Lithuania" ), i18n( "Luxembourg" ), i18n( "Macau" ),
    i18n( "Madagascar" ), i18n( "Malawi" ), i18n( "Malaysia" ),
    i18n( "Maldives" ), i18n( "Mali" ), i18n( "Malta" ),
    i18n( "Marshall Islands" ), i18n( "Martinique" ), i18n( "Mauritania" ),
    i18n( "Mauritius" ), i18n( "Mexico" ),
    i18n( "Micronesia, Federated States Of" ), i18n( "Moldova" ),
    i18n( "Monaco" ), i18n( "Mongolia" ), i18n( "Montserrat" ),
    i18n( "Morocco" ), i18n( "Mozambique" ), i18n( "Myanmar" ),
    i18n( "Namibia" ),
    i18n( "Nauru" ), i18n( "Nepal" ), i18n( "Netherlands" ),
    i18n( "Netherlands Antilles" ), i18n( "New Caledonia" ),
    i18n( "New Zealand" ), i18n( "Nicaragua" ), i18n( "Niger" ),
    i18n( "Nigeria" ), i18n( "Niue" ), i18n( "North Korea" ),
    i18n( "Northern Ireland" ), i18n( "Northern Mariana Islands" ),
    i18n( "Norway" ), i18n( "Oman" ), i18n( "Pakistan" ), i18n( "Palau" ),
    i18n( "Palestinian" ), i18n( "Panama" ), i18n( "Papua New Guinea" ),
    i18n( "Paraguay" ), i18n( "Peru" ), i18n( "Philippines" ),
    i18n( "Poland" ), i18n( "Portugal" ), i18n( "Puerto Rico" ),
    i18n( "Qatar" ), i18n( "Romania" ), i18n( "Russia" ), i18n( "Rwanda" ),
    i18n( "St. Kitts and Nevis" ), i18n( "St. Lucia" ),
    i18n( "St. Vincent and the Grenadines" ), i18n( "San Marino" ),
    i18n( "Sao Tome and Principe" ), i18n( "Saudi Arabia" ),
    i18n( "Senegal" ), i18n( "Serbia & Montenegro" ), i18n( "Seychelles" ),
    i18n( "Sierra Leone" ), i18n( "Singapore" ), i18n( "Slovakia" ),
    i18n( "Slovenia" ), i18n( "Solomon Islands" ), i18n( "Somalia" ),
    i18n( "South Africa" ), i18n( "South Korea" ), i18n( "Spain" ),
    i18n( "Sri Lanka" ), i18n( "St. Kitts and Nevis" ), i18n( "Sudan" ),
    i18n( "Suriname" ), i18n( "Swaziland" ), i18n( "Sweden" ),
    i18n( "Switzerland" ), i18n( "Syria" ), i18n( "Taiwan" ),
    i18n( "Tajikistan" ), i18n( "Tanzania" ), i18n( "Thailand" ),
    i18n( "Tibet" ), i18n( "Togo" ), i18n( "Tonga" ),
    i18n( "Trinidad and Tobago" ), i18n( "Tunisia" ), i18n( "Turkey" ),
    i18n( "Turkmenistan" ), i18n( "Turks and Caicos Islands" ),
    i18n( "Tuvalu" ), i18n( "Uganda" ), i18n( "Ukraine" ),
    i18n( "United Arab Emirates" ), i18n( "United Kingdom" ),
    i18n( "United States" ), i18n( "Uruguay" ), i18n( "Uzbekistan" ),
    i18n( "Vanuatu" ), i18n( "Vatican City" ), i18n( "Venezuela" ),
    i18n( "Vietnam" ), i18n( "Western Samoa" ), i18n( "Yemen" ),
    i18n( "Yugoslavia" ), i18n( "Zaire" ), i18n( "Zambia" ),
    i18n( "Zimbabwe" ),
    ""
  };

  QStringList countries;
  for ( int i = 0; !country[ i ].isEmpty(); ++i )
    countries.append( country[ i ] );

  countries = sortLocaleAware( countries );

  mCountryCombo->addItems( countries );
  mCountryCombo->completionObject()->setItems( countries );
  mCountryCombo->setAutoCompletion( true );
}


AddressTypeDialog::AddressTypeDialog( int type, QWidget *parent )
  : KDialog( parent)
{
  setCaption( i18nc( "street/postal", "Edit Address Type" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );

  QWidget *page = new QWidget(this);
  setMainWidget( page );
  QVBoxLayout *layout = new QVBoxLayout( page );

  QGroupBox *box  = new QGroupBox( i18nc( "street/postal", "Address Types" ), page );
  layout->addWidget( box );
  mGroup = new QButtonGroup( box );
  mGroup->setExclusive ( false );

  QHBoxLayout *buttonLayout = new QHBoxLayout( box );

  mTypeList = KABC::Address::typeList();
  mTypeList.removeAll( KABC::Address::Pref );

  KABC::Address::TypeList::ConstIterator it;
  int i = 0;
  for ( it = mTypeList.begin(); it != mTypeList.end(); ++it, ++i ) {
    QCheckBox *cb = new QCheckBox( KABC::Address::typeLabel( *it ), box );
    cb->setChecked( type & mTypeList[ i ] );
    buttonLayout->addWidget( cb );
    mGroup->addButton( cb );
  }
}

AddressTypeDialog::~AddressTypeDialog()
{
}

int AddressTypeDialog::type() const
{
  int type = 0;
  for ( int i = 0; i < mGroup->buttons().count(); ++i ) {
    QCheckBox *box = dynamic_cast<QCheckBox*>( mGroup->buttons().at( i ) );
    if ( box && box->isChecked() )
      type += mTypeList[ i ];
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
  for ( it = list.begin(); it != list.end(); ++it )
    sortedList.append( LocaleAwareString( *it ) );

  qSort( sortedList.begin(), sortedList.end() );

  QStringList retval;
  QList<LocaleAwareString>::ConstIterator retIt;
  for ( retIt = sortedList.begin(); retIt != sortedList.end(); ++retIt )
    retval.append( *retIt );

  return retval;
}

#include "addresseditwidget.moc"
