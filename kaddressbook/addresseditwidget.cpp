/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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
#include <qcheckbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qsignal.h>
#include <qstring.h>
#include <qtextedit.h>
#include <qtoolbutton.h>
#include <qtooltip.h>

#include <kaccelmanager.h>
#include <kapplication.h>
#include <kbuttonbox.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kseparator.h>

#include "addresseditwidget.h"


AddressEditWidget::AddressEditWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QGridLayout *layout = new QGridLayout( this, 4, 2, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  mTypeCombo = new KComboBox( this );
  connect( mTypeCombo, SIGNAL( activated( int ) ),
           SLOT( updateView() ) );
  layout->addWidget( mTypeCombo, 0, 0 );

  mAddressView = new QTextEdit( this );
  mAddressView->setReadOnly( true );
  mAddressView->setMinimumHeight( 20 );
  layout->addMultiCellWidget( mAddressView, 1, 3, 0, 0 );

  mAddButton = new QPushButton( i18n( "Add" ), this );
  layout->addWidget( mAddButton, 0, 1 );

  mEditButton = new QPushButton( i18n( "Edit..." ), this );
  layout->addWidget( mEditButton, 1, 1 );

  mRemoveButton = new QPushButton( i18n( "Remove..." ), this );
  layout->addWidget( mRemoveButton, 2, 1 );

  mPreferredButton = new QPushButton( i18n( "Set Preferred" ), this );
  layout->addWidget( mPreferredButton, 3, 1 );

  connect( mAddButton, SIGNAL( clicked() ), SLOT( add() ) );
  connect( mEditButton, SIGNAL( clicked() ), SLOT( edit() ) );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( remove() ) );
  connect( mPreferredButton, SIGNAL( clicked() ), SLOT( setPreferred() ) );
}

AddressEditWidget::~AddressEditWidget()
{
}

KABC::Address::List AddressEditWidget::addresses() const
{
  return mAddressList;
}

void AddressEditWidget::setAddresses( const KABC::Addressee &addr,
                                      const KABC::Address::List &list )
{
  mAddressee = addr;
  mAddressList = list;

  updateTypeCombo();
}

void AddressEditWidget::updateTypeCombo()
{
  int pos = mTypeCombo->currentItem();

  mTypeCombo->clear();
  mAddressMap.clear();

  QMap<int,int> labelCount;
  QStringList items;

  KABC::Address::List::Iterator it;
  bool hasPreferred = false;
  for ( it = mAddressList.begin(); it != mAddressList.end(); ++it ) {
    if ( (*it).type() & KABC::Address::Pref ) {
      hasPreferred = true;
      break;
    }
  }

  int addrPos, currAddr = 0;
  if ( hasPreferred )
    addrPos = 1;
  else
    addrPos = 0;
  for ( it = mAddressList.begin(); it != mAddressList.end(); ++it, ++addrPos, ++currAddr ) {
    int type = (*it).type() & ~( KABC::Address::Pref );

    int count = 1;
    if ( labelCount.contains( type ) )
      count = labelCount[ type ] + 1;
    labelCount[ type ] = count;

    QString label = KABC::Address::typeLabel( type );
    if ( count > 1 )
      label = i18n("label (number)", "%1 (%2)").arg( label )
                                           .arg( QString::number( count ) );

    if ( (*it).type() & KABC::Address::Pref ) { // is preferred address
      items.prepend( label );
      mAddressMap.insert( 0, currAddr );
      addrPos--;
    } else {
      items.append( label );
      mAddressMap.insert( addrPos, currAddr );
    }
  }
  
  mTypeCombo->insertStringList( items );
  mTypeCombo->setCurrentItem( pos );

  bool state = ( mAddressList.count() != 0 );
  mEditButton->setEnabled( state );
  mRemoveButton->setEnabled( state );
  mPreferredButton->setEnabled( state );

  updateView();
}

void AddressEditWidget::updateView()
{
  mAddressView->setText( "" );

  KABC::Address addr = mAddressList[ mAddressMap[ mTypeCombo->currentItem() ] ];
  if ( !addr.isEmpty() ) {
#if KDE_VERSION >= 319
    if ( addr.type() & KABC::Address::Work ) {
      mAddressView->setText( addr.formattedAddress( mAddressee.realName(),
                             mAddressee.organization() ) );
    } else
      mAddressView->setText( addr.formattedAddress( mAddressee.realName() ) );
#else
    QString text;
    if ( !addr.street().isEmpty() )
      text += addr.street() + "\n";

    if ( !addr.postOfficeBox().isEmpty() )
      text += addr.postOfficeBox() + "\n";

    text += addr.locality() + QString(" ") + addr.region();

    if ( !addr.postalCode().isEmpty() )
      text += QString(", ") + addr.postalCode();

    text += "\n";

    if ( !addr.country().isEmpty() )
      text += addr.country() + "\n";

    text += addr.extended();

    mAddressView->setText( text );      
#endif
  }
}

void AddressEditWidget::add()
{
  KABC::Address addr;
  addr.setType( KABC::Address::Home );
  
  AddressEditDialog dlg( addr, this );
  if ( dlg.exec() ) {
    mAddressList.append( dlg.address() );
    updateTypeCombo();
    emit modified();
  }
}

void AddressEditWidget::edit()
{
  KABC::Address addr = mAddressList[ mAddressMap[ mTypeCombo->currentItem() ] ];
  AddressEditDialog dlg( addr, this );
  if ( dlg.exec() ) {
    mAddressList.remove( addr );
    mAddressList.append( dlg.address() );
    updateTypeCombo();
    emit modified();
  }
}

void AddressEditWidget::remove()
{
  KABC::Address addr = mAddressList[ mAddressMap[ mTypeCombo->currentItem() ] ];
  mAddressList.remove( addr );
  updateTypeCombo();

  emit modified();
}

void AddressEditWidget::setPreferred()
{
  KABC::Address &addr = mAddressList[ mAddressMap[ mTypeCombo->currentItem() ] ];
  
  KABC::Address::List::Iterator it;
  for ( it = mAddressList.begin(); it != mAddressList.end(); ++it ) {
    if ( (*it).type() & KABC::Address::Pref )
      (*it).setType( (*it).type() & ~(KABC::Address::Pref) );
  }

  int type = addr.type() | KABC::Address::Pref;
  addr.setType( type );

  updateTypeCombo();
  emit modified();
}

AddressEditDialog::AddressEditDialog( const KABC::Address &addr,
                                      QWidget *parent, const char *name )
  : KDialogBase( Plain, i18n( "Edit Address" ), Ok | Cancel, Ok,
                 parent, name, true, true ), mAddress( addr )
{
  QWidget *page = plainPage();

  QGridLayout *topLayout = new QGridLayout( page, 7, 2 );
  topLayout->setSpacing( spacingHint() );

  QLabel *label = new QLabel( i18n( "Street:" ), page );
  label->setAlignment( Qt::AlignTop | Qt::AlignLeft );
  topLayout->addWidget( label, 0, 0 );
  mStreetTextEdit = new QTextEdit( page );
  label->setBuddy( mStreetTextEdit );
  topLayout->addWidget( mStreetTextEdit, 0, 1 );

  label = new QLabel( i18n( "Post office box:" ), page );
  topLayout->addWidget( label, 1 , 0 );
  mPOBoxEdit = new KLineEdit( page );
  label->setBuddy( mPOBoxEdit );
  topLayout->addWidget( mPOBoxEdit, 1, 1 );

  label = new QLabel( i18n( "Locality:" ), page );
  topLayout->addWidget( label, 2, 0 );
  mLocalityEdit = new KLineEdit( page, "mLocalityEdit" );
  label->setBuddy( mLocalityEdit );
  topLayout->addWidget( mLocalityEdit, 2, 1 );

  label = new QLabel( i18n( "Region:" ), page );
  topLayout->addWidget( label, 3, 0 );
  mRegionEdit = new KLineEdit( page );
  label->setBuddy( mRegionEdit );
  topLayout->addWidget( mRegionEdit, 3, 1 );

  label = new QLabel( i18n( "Postal code:" ), page );
  topLayout->addWidget( label, 4, 0 );
  mPostalCodeEdit = new KLineEdit( page );
  label->setBuddy( mPostalCodeEdit );
  topLayout->addWidget( mPostalCodeEdit, 4, 1 );

  label = new QLabel( i18n( "Country:" ), page );
  topLayout->addWidget( label, 5, 0 );
  mCountryCombo = new KComboBox( page );
  mCountryCombo->setEditable( true );
  mCountryCombo->setDuplicatesEnabled( false );
  mCountryCombo->setAutoCompletion( true );
  label->setBuddy( mCountryCombo );
  topLayout->addWidget( mCountryCombo, 5, 1 );

  mTypeButton = new QPushButton( i18n( "Change Type" ), page );
  topLayout->addMultiCellWidget( mTypeButton, 6, 6, 0, 1 );  

  fillCountryCombo();
  KAcceleratorManager::manage( this );

  connect( mTypeButton, SIGNAL( clicked() ), SLOT( changeType() ) );

  // Fill GUI
  mStreetTextEdit->setText( mAddress.street() );
  mRegionEdit->setText( mAddress.region() );
  mLocalityEdit->setText( mAddress.locality() );
  mPostalCodeEdit->setText( mAddress.postalCode() );
  mPOBoxEdit->setText( mAddress.postOfficeBox() );
  mCountryCombo->setCurrentText( mAddress.country() );

  mStreetTextEdit->setFocus();
}

AddressEditDialog::~AddressEditDialog()
{
}

KABC::Address AddressEditDialog::address()
{
  mAddress.setLocality( mLocalityEdit->text() );
  mAddress.setRegion( mRegionEdit->text() );
  mAddress.setPostalCode( mPostalCodeEdit->text() );
  mAddress.setCountry( mCountryCombo->currentText() );
  mAddress.setPostOfficeBox( mPOBoxEdit->text() );
  mAddress.setStreet( mStreetTextEdit->text() );

  return mAddress;
}

void AddressEditDialog::changeType()
{
  AddressTypeDialog dlg( mAddress.type(), this );
  if ( dlg.exec() )
    mAddress.setType( dlg.type() );
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
    i18n( "Tuvalu" ), i18n( "Uganda " ), i18n( "Ukraine" ),
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

  countries.sort();

  mCountryCombo->insertStringList( countries );
}


AddressTypeDialog::AddressTypeDialog( int type, QWidget *parent )
  : KDialogBase( Plain, i18n( "Edit Address Type" ), Ok | Cancel, Ok,
                 parent, "AddressTypeDialog" )
{
  QWidget *page = plainPage();
  QVBoxLayout *layout = new QVBoxLayout( page );

  mGroup = new QButtonGroup( 2, Horizontal, i18n( "Address Types" ), page );
  layout->addWidget( mGroup );

  mTypeList = KABC::Address::typeList();
  mTypeList.remove( KABC::Address::Pref );

  KABC::Address::TypeList::Iterator it;
  for ( it = mTypeList.begin(); it != mTypeList.end(); ++it )
    new QCheckBox( KABC::Address::typeLabel( *it ), mGroup );

  for ( int i = 0; i < mGroup->count(); ++i ) {
    QCheckBox *box = (QCheckBox*)mGroup->find( i );
    box->setChecked( type & mTypeList[ i ] );
  }
}

AddressTypeDialog::~AddressTypeDialog()
{
}

int AddressTypeDialog::type() const
{
  int type = 0;
  for ( int i = 0; i < mGroup->count(); ++i ) {
    QCheckBox *box = (QCheckBox*)mGroup->find( i );
    if ( box->isChecked() )
      type += mTypeList[ i ];
  }

  return type;
}

#include "addresseditwidget.moc"
