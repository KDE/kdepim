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
#include <kinputdialog.h>
#include <klineedit.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kseparator.h>

#include "addresseditwidget.h"

class TabPressEater : public QObject
{
  public:
    TabPressEater( QObject *parent )
      : QObject( parent, "TabPressEater" )
    {
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
  : QWidget( parent, name )
{
  QBoxLayout *layout = new QVBoxLayout( this, 4, 2 );
  layout->setSpacing( KDialog::spacingHint() );

  mTypeCombo = new AddressTypeCombo( mAddressList, this );
  connect( mTypeCombo, SIGNAL( activated( int ) ),
           SLOT( updateAddressEdit() ) );
  layout->addWidget( mTypeCombo );

  mAddressField = new QLabel( this );
  mAddressField->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  mAddressField->setMinimumHeight( 20 );
  layout->addWidget( mAddressField );

  mEditButton = new QPushButton( i18n( "&Edit Addresses..." ), this );
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

  QValueList<int> defaultTypes;
  defaultTypes << KABC::Address::Home;
  defaultTypes << KABC::Address::Work;

  AddresseeConfig config( mAddressee );
  const QValueList<int> configList = config.noDefaultAddrTypes();
  QValueList<int>::ConstIterator it;
  for ( it = configList.begin(); it != configList.end(); ++it )
    defaultTypes.remove( *it );

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

void AddressEditWidget::edit()
{
  AddressEditDialog dialog( mAddressList, mTypeCombo->currentItem(), this );
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
      QValueList<int> configList;
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
#if KDE_VERSION >= 319
      if ( a.type() & KABC::Address::Work ) {
        mAddressField->setText( a.formattedAddress( mAddressee.realName(),
                                   mAddressee.organization() ) );
      } else {
        mAddressField->setText( a.formattedAddress( mAddressee.realName() ) );
      }
#else
      QString text;
      if ( !a.street().isEmpty() )
        text += a.street() + "\n";

      if ( !a.postOfficeBox().isEmpty() )
        text += a.postOfficeBox() + "\n";

      text += a.locality() + QString(" ") + a.region();

      if ( !a.postalCode().isEmpty() )
        text += QString(", ") + a.postalCode();

      text += "\n";

      if ( !a.country().isEmpty() )
        text += a.country() + "\n";

      text += a.extended();

      mAddressField->setText( text );
#endif
    }
  }

  blockSignals( block );
}

AddressEditDialog::AddressEditDialog( const KABC::Address::List &list,
                                      int selected, QWidget *parent,
                                      const char *name )
  : KDialogBase( Plain, i18n( "Edit Address" ), Ok | Cancel, Ok,
                 parent, name, true, true ),
    mPreviousAddress( 0 )
{
  mAddressList = list;

  QWidget *page = plainPage();

  QGridLayout *topLayout = new QGridLayout( page, 8, 2 );
  topLayout->setSpacing( spacingHint() );

  mTypeCombo = new AddressTypeCombo( mAddressList, page );
  topLayout->addMultiCellWidget( mTypeCombo, 0, 0, 0, 1 );

  QLabel *label = new QLabel( i18n( "<streetLabel>:", "%1:" ).arg( KABC::Address::streetLabel() ), page );
  label->setAlignment( Qt::AlignTop | Qt::AlignLeft );
  topLayout->addWidget( label, 1, 0 );
  mStreetTextEdit = new QTextEdit( page );
  label->setBuddy( mStreetTextEdit );
  topLayout->addWidget( mStreetTextEdit, 1, 1 );

  TabPressEater *eater = new TabPressEater( this );
  mStreetTextEdit->installEventFilter( eater );

  label = new QLabel( i18n( "<postOfficeBoxLabel>:", "%1:" ).arg( KABC::Address::postOfficeBoxLabel() ), page );
  topLayout->addWidget( label, 2 , 0 );
  mPOBoxEdit = new KLineEdit( page );
  label->setBuddy( mPOBoxEdit );
  topLayout->addWidget( mPOBoxEdit, 2, 1 );

  label = new QLabel( i18n( "<localityLabel>:", "%1:" ).arg( KABC::Address::localityLabel() ), page );
  topLayout->addWidget( label, 3, 0 );
  mLocalityEdit = new KLineEdit( page );
  label->setBuddy( mLocalityEdit );
  topLayout->addWidget( mLocalityEdit, 3, 1 );

  label = new QLabel( i18n( "<regionLabel>:", "%1:" ).arg( KABC::Address::regionLabel() ), page );
  topLayout->addWidget( label, 4, 0 );
  mRegionEdit = new KLineEdit( page );
  label->setBuddy( mRegionEdit );
  topLayout->addWidget( mRegionEdit, 4, 1 );

  label = new QLabel( i18n( "<postalCodeLabel>:", "%1:" ).arg( KABC::Address::postalCodeLabel() ), page );
  topLayout->addWidget( label, 5, 0 );
  mPostalCodeEdit = new KLineEdit( page );
  label->setBuddy( mPostalCodeEdit );
  topLayout->addWidget( mPostalCodeEdit, 5, 1 );

  label = new QLabel( i18n( "<countryLabel>:", "%1:" ).arg( KABC::Address::countryLabel() ), page );
  topLayout->addWidget( label, 6, 0 );
  mCountryCombo = new KComboBox( page );
  mCountryCombo->setEditable( true );
  mCountryCombo->setDuplicatesEnabled( false );

#if KDE_IS_VERSION(3,3,0)
  QPushButton *labelButton = new QPushButton( i18n( "Edit Label..." ), page );
  topLayout->addMultiCellWidget( labelButton, 7, 7, 0, 1 );
  connect( labelButton, SIGNAL( clicked() ), SLOT( editLabel() ) );
#endif

  fillCountryCombo();
  label->setBuddy( mCountryCombo );
  topLayout->addWidget( mCountryCombo, 6, 1 );

  mPreferredCheckBox = new QCheckBox( i18n( "This is the preferred address" ), page );
  topLayout->addMultiCellWidget( mPreferredCheckBox, 8, 8, 0, 1 );

  KSeparator *sep = new KSeparator( KSeparator::HLine, page );
  topLayout->addMultiCellWidget( sep, 9, 9, 0, 1 );

  QHBox *buttonBox = new QHBox( page );
  buttonBox->setSpacing( spacingHint() );
  topLayout->addMultiCellWidget( buttonBox, 10, 10, 0, 1 );

  QPushButton *addButton = new QPushButton( i18n( "New..." ), buttonBox );
  connect( addButton, SIGNAL( clicked() ), SLOT( addAddress() ) );

  mRemoveButton = new QPushButton( i18n( "Remove" ), buttonBox );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( removeAddress() ) );

  mChangeTypeButton = new QPushButton( i18n( "Change Type..." ), buttonBox );
  connect( mChangeTypeButton, SIGNAL( clicked() ), SLOT( changeType() ) );

  mTypeCombo->updateTypes();
  mTypeCombo->setCurrentItem( selected );

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
    mTypeCombo->setCurrentItem( mTypeCombo->count() - 1 );
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

    mAddressList.remove( it );
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
#if KDE_IS_VERSION(3,3,0)
  bool ok = false;
  QString result = KInputDialog::getMultiLineText( KABC::Address::labelLabel(),
                                                   KABC::Address::labelLabel(),
                                                   mLabel, &ok, this );
  if ( ok ) {
    mLabel = result;
    modified();
  }
#endif
}

void AddressEditDialog::updateAddressEdits()
{
  if ( mPreviousAddress )
    saveAddress( *mPreviousAddress );

  KABC::Address::List::Iterator it = mTypeCombo->selectedElement();
  KABC::Address a = *it;
  mPreviousAddress = &(*it);

  bool tmp = mChanged;

  mStreetTextEdit->setText( a.street() );
  mRegionEdit->setText( a.region() );
  mLocalityEdit->setText( a.locality() );
  mPostalCodeEdit->setText( a.postalCode() );
  mPOBoxEdit->setText( a.postOfficeBox() );
  mCountryCombo->setCurrentText( a.country() );
  mLabel = a.label();

  mPreferredCheckBox->setChecked( a.type() & KABC::Address::Pref );

  if ( a.isEmpty() )
    mCountryCombo->setCurrentText( KGlobal::locale()->twoAlphaToCountryName( KGlobal::locale()->country() ) );

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
  addr.setStreet( mStreetTextEdit->text() );
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

  mCountryCombo->insertStringList( countries );
  mCountryCombo->completionObject()->setItems( countries );
  mCountryCombo->setAutoCompletion( true );
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

  KABC::Address::TypeList::ConstIterator it;
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
  QValueList<LocaleAwareString> sortedList;

  QStringList::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    sortedList.append( LocaleAwareString( *it ) );

  qHeapSort( sortedList );

  QStringList retval;
  QValueList<LocaleAwareString>::ConstIterator retIt;
  for ( retIt = sortedList.begin(); retIt != sortedList.end(); ++retIt )
    retval.append( *retIt );

  return retval;
}

#include "addresseditwidget.moc"
