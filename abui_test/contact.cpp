/* This file is part of KDE PIM
   Copyright (C) 1999 Don Sanders <dsanders@kde.org>

   License: GNU GPL
*/

#include "contact.h"
#include <qlayout.h>
#include <qvbox.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qgrid.h>
#include <qgroupbox.h>
#include <qtabwidget.h>

#include "namevalue.h"
#include "contactentry.h"
#include "datepickerdialog.h"
#include "klocale.h"
#include "kapp.h" // for kapp->palette()
#include "kglobal.h"

ContactDialog::ContactDialog( QWidget *parent, const char *name, ContactEntry *ce )
  : QTabWidget( parent, name ), vs( 0 ), vp( 0 )
{
    ce ? this->ce = ce : this->ce = new ContactEntry();
    if (ce->find( "N" ))
      curName = *ce->find( "N");
    /*
    QVBoxLayout *vb = new QVBoxLayout( this, 5 );
    tabs = new QTabWidget( this );
    vb->addWidget( tabs );
    tabs->setMargin( 4 );
    */
    tabs = this;
    setMargin( 4 );

    setupTab1();
    setupTab2();
    setupTab3();
    
    ce->touch();
    debug( "sizhint " + QString().setNum( sizeHint().width() ));
    setMinimumSize( sizeHint() );
    resize( sizeHint() );
}

ContactEntry* ContactDialog::entry()
{
  return ce;
}

void ContactDialog::setupTab1()
{
  QFrame *tab1 = new QFrame( 0, "tab1" );
  tab1->setFrameStyle( QFrame::NoFrame );
  QGridLayout *tab1lay = new QGridLayout( tab1, 8, 5 );
  tab1lay->setSpacing( 5 );
  tab1->setMargin( 5 );
  
/////////////////////////////////
// The Name/Job title group

  // First row
  QPushButton *pbFullName = new QPushButton( "&Full Name...", tab1 );
  leFullName = new ContactLineEdit( tab1, ".AUXCONTACT-N", ce );
  leFullName->setText( curName );
  connect( ce, SIGNAL( changed() ), this, SLOT( parseName() ));
  connect( pbFullName, SIGNAL( clicked()), this, SLOT( newNameDialog()));
  tab1lay->addWidget( pbFullName, 0, 0 );
  tab1lay->addWidget( leFullName, 0, 1 );

  QFrame * filler1 = new QFrame( tab1, "filler1" );
  filler1->setFrameStyle( QFrame::NoFrame );
  filler1->setMinimumWidth( 1 );
  tab1lay->addWidget( filler1, 0, 2 );

  QLabel *lJobTitle = new QLabel( "&Job title:", tab1 );
  QLineEdit *leJobTitle = new ContactLineEdit( tab1, "ROLE", ce );
  lJobTitle->setBuddy( leJobTitle );
  tab1lay->addWidget( lJobTitle, 0, 3 );
  tab1lay->addWidget( leJobTitle, 0, 4 );

  // Second row
  QLabel *lCompany = new QLabel( "&Company:", tab1 );
  QLineEdit *leCompany = new ContactLineEdit( tab1, "ORG", ce );
  lCompany->setBuddy( leCompany );
  curCompany = leCompany->text();
  connect( ce, SIGNAL( changed() ), this, SLOT( monitorCompany() ));
  tab1lay->addWidget( lCompany, 1, 0 );
  tab1lay->addWidget( leCompany, 1, 1 );

  QFrame * filler2 = new QFrame( tab1, "filler2" );
  filler2->setFrameStyle( QFrame::NoFrame );
  filler2->setMinimumWidth( 1 );
  tab1lay->addWidget( filler2, 1, 2 );

  QLabel *lFileAs = new QLabel( "F&ile as:", tab1 );
  cbFileAs = new FileAsComboBox( tab1, "X-FileAs", ce );
  updateFileAs();
  if (ce->find( "X-FileAs" ))
    cbFileAs->setEditText( *ce->find( "X-FileAs" ));
  connect( cbFileAs, SIGNAL( textChanged( const QString& ) ), cbFileAs, SLOT( updateContact() ));
  tab1lay->addWidget( lFileAs, 1, 3 );
  tab1lay->addWidget( cbFileAs, 1, 4 );

  lFileAs->setBuddy( cbFileAs );
// End the Name/Job title group
////////////////////////////////
  
  // Horizontal bar (rather verbose)
  QFrame *bar1 = new QFrame( tab1, "horizontal divider 1" );
  bar1->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  bar1->setMinimumHeight( 10 );
  bar1->setMaximumHeight( 10 );
  tab1lay->addMultiCellWidget( bar1, 2, 2, 0, 4 );

////////////////////////////////
// The Email/Webpage group
  ContactComboBox *cbEmail = new ContactComboBox( tab1 );
  cbEmail->insertItem( "E-mail", "EMAIL" );
  cbEmail->insertItem( "E-mail 2", "X-E-mail2" );
  cbEmail->insertItem( "E-mail 3", "X-E-mail3" );
  QLineEdit *leEmail = new ContactLineEdit( tab1, "EMAIL", ce ); 
  cbEmail->setBuddy( leEmail );
  tab1lay->addWidget( cbEmail, 3, 0 );
  tab1lay->addWidget( leEmail, 3, 1 );

  QFrame *filler3 = new QFrame( tab1, "filler3" );
  filler3->setFrameStyle( QFrame::NoFrame );
  filler3->setMinimumWidth( 1 );
  tab1lay->addWidget( filler3, 3, 2 );

  QLabel *lWebPage = new QLabel( "&Web page:", tab1 );  
  QLineEdit *leWebPage = new ContactLineEdit( tab1, "WEBPAGE", ce );
  lWebPage->setBuddy( leWebPage );
  tab1lay->addWidget( lWebPage, 3, 3 );
  tab1lay->addWidget( leWebPage, 3, 4 );

// End the Email/Webpage group
///////////////////////////////

  // Horizontal bar (rather verbose)
  QFrame *bar2 = new QFrame( tab1, "horizontal divider 2" );
  bar2->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  bar2->setMinimumHeight( 10 );
  bar2->setMaximumHeight( 10 );
  tab1lay->addMultiCellWidget( bar2, 4, 4, 0, 4 );

///////////////////////////////
// The Address/Phone group
  
  // Use a box to keep the widgets fixed vertically in place
  QBoxLayout *lay1 = new QBoxLayout( QBoxLayout::Down, 10, "lay1" );

  QPushButton *pbAddress = new QPushButton( "Add&ress...", tab1 );
  connect( pbAddress, SIGNAL( clicked()), this, SLOT( newAddressDialog()));
  lay1->addWidget( pbAddress, 0 );
  cbAddress = new ContactComboBox( tab1 );
  cbAddress->insertItem( "Business", "X-BusinessAddress" );
  cbAddress->insertItem( "Home", "X-HomeAddress" );
  cbAddress->insertItem( "Other", "X-OtherAddress" );
  lay1->addWidget( cbAddress, 0 );
  lay1->addStretch( 1 );  // Fix the other widgets in place
  tab1lay->addLayout( lay1, 5, 0 );

  // Perhaps the address "subfields" (city, postal, country) should be cleared
  // when this control loses focus. They aren't at the moment.
  QMultiLineEdit *mleAddress = new ContactMultiLineEdit( tab1, "X-BusinessAddress", ce );
  cbAddress->setBuddy( mleAddress );
  tab1lay->addWidget( mleAddress, 5, 1 );

  QFrame *filler4 = new QFrame( tab1, "filler4" );
  filler4->setFrameStyle( QFrame::NoFrame );
  filler4->setMinimumWidth( 1 );
  tab1lay->addWidget( filler4, 5, 2 );

  QLabel *lPhone = new QLabel( "Phone:", tab1 );  
  lPhone->setAlignment( QLabel::AlignTop | QLabel::AlignLeft );
  tab1lay->addWidget( lPhone, 5, 3 );

  QBoxLayout *lay2 = new QBoxLayout( QBoxLayout::TopToBottom, 1 );
  lay2->setSpacing( 10 );
  const int numRows = 4;

  QString sPhone[] = {
    "Assistant's", "Business", "Business 2",
    "Business Fax", "Callback", "Car",
    "Company", "Home", "Home 2", "Home Fax",
    "ISDN", "Mobile", "Other", "Other Fax",
    "Pager", "Primary", "Radio", "Telex",
    "TTY/TDD",
    ""
  };

  QString vPhone[] = {
    "X-AssistantsPhone", "X-BusinessPhone", "X-BusinessPhone2",
    "X-BusinessFax", "X-Callback", "X-CarPhone",
    "X-CompanyMainPhone", "X-HomePhone", "X-HomePhone2", "X-HomeFax",
    "X-ISDN", "X-MobilePhone", "X-OtherPhone", "X-OtherFax",
    "X-Pager", "X-PrimaryPhone", "X-RadioPhone", "X-Telex",
    "X-TtyTddPhone",
    ""
  };

  int iPhone[] = { 1, 7, 3, 11 };

  QGridLayout *layhGrid = new QGridLayout( numRows, 2 );
  layhGrid->setSpacing( 10 );
  for ( int row = 0; row < numRows; row++ ) {
    QFrame *hGrid = tab1;

    ContactComboBox *cbPhone = new ContactComboBox( hGrid );
    for (int i =0; sPhone[i] != ""; ++i )
      cbPhone->insertItem( i18n( sPhone[i] ), vPhone[i] );
    cbPhone->setCurrentItem( iPhone[row] );
    cbPhone->setMinimumSize( cbPhone->sizeHint() );
    layhGrid->addWidget( cbPhone, row, 0 );

    QLineEdit *ed = new ContactLineEdit( hGrid, vPhone[iPhone[row]], ce ); 
    ed->setMinimumSize( ed->sizeHint());
    cbPhone->setBuddy( ed );
    layhGrid->addWidget( ed, row ,1 );
  }
  lay2->addLayout( layhGrid, 0 );
  lay2->addStretch( 1 ) ;
  tab1lay->addLayout( lay2, 5, 4 );

// End The Address/Phone group
///////////////////////////////
   
  // Horizontal bar
  QFrame *bar3 = new QFrame( tab1, "horizontal divider 3" );
  bar3->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  bar3->setMinimumHeight( 10 );
  bar3->setMaximumHeight( 10 );
  tab1lay->addMultiCellWidget( bar3, 6, 6, 0, 4 );

//////////////////////
// The Note group
  // Interestingly this doesn't have an equivalent in tab3
  QMultiLineEdit *mleNotes = new ContactMultiLineEdit( tab1, "X-Notes", ce );
  mleNotes->setMinimumSize( mleNotes->sizeHint() );
  mleNotes->resize( mleNotes->sizeHint() );
  tab1lay->addMultiCellWidget( mleNotes, 7, 7, 0, 4 );
// End the Note group
//////////////////////
 
  tab1lay->activate(); // required
  tabs->addTab( tab1, "&General" );
}

void ContactDialog::setupTab2()
{
  // Use a boxlayout to keep the widgets position fixed vertically
  QFrame *v2 = new QFrame( this );
  QBoxLayout *lay2 = new QBoxLayout( v2, QBoxLayout::TopToBottom, 5, 5, "h3BoxLayout" );
  lay2->setSpacing( 10 );

  const int numRows = 9;
  QString sLabel[numRows] = { "D&epartment:", "&Office:", "&Profession:", 
                        "Assistant's &Name:", "&Managers's Name:",
                        "Birthday", "Anniversary:", "Ni&ckname:", 
                        "&Spouse's Name:" };
  QString entryField[numRows] = { 
    "X-Department", "X-Office", "X-Profession", "X-AssistantsName",
    "X-ManagersName", "BDAY", "X-Anniversary", "NICKNAME", "X-SpousesName" 
  };
			     
  QLabel *(label[numRows]);
  QPushButton *(pbDate[2]);
  QSize size = QSize( 0, 0 );
  QSize size2 = QSize( 0, 0 );

  for ( int row = 0; row < 5; row++ ) {
    QGrid *hGrid = new QGrid ( 2, QGrid::Horizontal, v2 );
    hGrid->setSpacing( 10 );
    label[row] = new QLabel( sLabel[row], hGrid );
    size = size.expandedTo( label[row]->sizeHint() );
    QLineEdit *ed = new ContactLineEdit( hGrid, entryField[row], ce ); 
    label[row]->setBuddy( ed );
    lay2->addWidget( hGrid, 0 );
  }

  QFrame *f3 = new QFrame( v2, "horizontal divider 1" );
  f3->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  f3->setMinimumHeight( 2 );
  lay2->addWidget( f3, 0 );

  for ( int row = 5; row < 7; row++ ) {
    QFrame *v3 = new QFrame( v2 );
    QBoxLayout *lay3 = new QBoxLayout( v3, QBoxLayout::LeftToRight, 1, 1, "h3BoxLayout" );
    lay3->setSpacing( 10 );

    label[row] = new QLabel( sLabel[row], v3 );
    lay3->addWidget( label[row] );
    size = size.expandedTo( label[row]->sizeHint() );
    QLineEdit *ed = new ContactLineEdit( v3, entryField[row], ce ); 
    ed->setMaximumSize( ed->sizeHint() );
    lay3->addWidget( ed, 0 );
    label[row]->setBuddy( ed );
    pbDate[row - 5] = new QPushButton( sLabel[row], v3 );
    lay3->addWidget( pbDate[row - 5] );
    size2 = size2.expandedTo( pbDate[row - 5]->sizeHint() );
    lay3->addStretch( 1 ) ;
    lay2->addWidget( v3 );
  }
    connect( pbDate[0], SIGNAL( clicked()), this, SLOT( pickBirthDate() ));  
    connect( pbDate[1], SIGNAL( clicked()), this, SLOT( pickAnniversaryDate() ));  

  QFrame *f5 = new QFrame( v2, "horizontal divider 1" );
  f5->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  f5->setMinimumHeight( 2 );
  lay2->addWidget( f5, 0 );

  for ( int row = 7; row < 9; row++ ) {
    QGrid *hGrid = new QGrid ( 2, QGrid::Horizontal, v2 );
    hGrid->setSpacing( 10 );
    label[row] = new QLabel( sLabel[row], hGrid );
    size = size.expandedTo( label[row]->sizeHint() );
    QLineEdit *ed = new ContactLineEdit( hGrid, entryField[row], ce ); 
    label[row]->setBuddy( ed );
    lay2->addWidget( hGrid, 0 );
  }

  for ( int row = 0; row < numRows; row++ ) {
    label[row]->resize( size );
    label[row]->setMinimumSize( size );
  }
  pbDate[0]->setMinimumSize( size2 );
  pbDate[1]->setMinimumSize( size2 );

  lay2->addStretch( 1 ) ;
  tabs->addTab( v2, "&Details" );
}

void ContactDialog::setupTab3()
{
    QStringList names;
    QStringList fields;
    QString tmp;
    QFrame *tab3 = new QFrame( this );
    QBoxLayout *t3lay = new QBoxLayout( tab3, QBoxLayout::TopToBottom, 5, 5 );
    
    QFrame *row1 = new QFrame( tab3 );
    QBoxLayout *lay1 = new QBoxLayout( row1, QBoxLayout::LeftToRight, 1, 1 );
    lay1->setSpacing( 10 );
    QLabel *lSelectFrom = new QLabel( i18n("&Select from:"), row1 );
    lay1->addWidget( lSelectFrom, 0 );
    cbSelectFrom = new QComboBox( false, row1 );
    lay1->addWidget( cbSelectFrom, 0 );
    lay1->addStretch( 1 );  // Fix the other widgets in place
    lSelectFrom->setBuddy( cbSelectFrom );
    t3lay->addWidget( row1, 0);
 
    QString sFields[] = { 
      "All Contact fields", "Frequently-used fields", "Address fields",
      "E-mail fields", "Fax/Other number fields", "Miscellanous fields",
      "Name fields", "Personal fields", "Phone number fields",
      "User-defined fields in folder", ""
    };

    for (int i =0; tmp = i18n(sFields[i]), tmp != ""; ++i )
      cbSelectFrom->insertItem( tmp );
    cbSelectFrom->setCurrentItem( 9 );
    fields = ce->custom();
    fields.sort();
    for (int i = 0; i < (int)fields.count(); ++i )
      names += fields[i].mid( 9 );

    vs = new NameValueSheet( 0, names.count(), names, fields, ce );
    vp = new NameValueFrame( tab3, vs );
    connect( cbSelectFrom, SIGNAL( activated(int)), this, SLOT( setSheet(int)));
    t3lay->addWidget( vp, 1);

    QFrame *row3 = new QFrame( tab3 );
    QBoxLayout *lay3 = new QBoxLayout( row3, QBoxLayout::LeftToRight, 1, 1 );
    lay3->setSpacing( 10 );
    QPushButton *pbNew = new QPushButton( "&New...", row3 );
    connect( pbNew, SIGNAL( clicked()), this, SLOT( newFieldDialog()));
    lay3->addWidget( pbNew, 0 );
    lay3->addStretch( 1 );  // Fix the other widgets in place
    t3lay->addWidget( row3, 0);
    tabs->addTab( tab3, "&All fields" );
}

void ContactDialog::pickBirthDate() 
{
  DatePickerDialog* datePicker=new DatePickerDialog( "Select Birthday", this);
  datePicker->setDate(QDate::currentDate());
  if(datePicker->exec())
    ce->replace( "BDAY", new QString( datePicker->getDate().toString()));
    // ce autoDelete will clean it up
  delete datePicker;
}

void ContactDialog::pickAnniversaryDate() 
{
  DatePickerDialog* datePicker=new DatePickerDialog( "Select Birthday", this);
  datePicker->setDate(QDate::currentDate());
  if(datePicker->exec())
    ce->replace( "X-Anniversary", new QString( datePicker->getDate().toString()));
    // ce autoDelete will clean it up
  delete datePicker;
}

void ContactDialog::newAddressDialog()
{
  QDialog *ad = new AddressDialog( this, cbAddress->currentEntryField(), ce, true );
  ad->exec();
}

void ContactDialog::newFieldDialog()
{
  NewFieldDialog *fd = new NewFieldDialog( this, true );
  if (fd->exec()) {
    ce->replace( fd->field(), new QString( fd->value() ));
    cbSelectFrom->setCurrentItem( 9 );
    setSheet( 9 );
  }
}

void ContactDialog::newNameDialog()
{
  QDialog *nd = new NameDialog( this, ce, true );
  if (nd->exec()) {
    if (ce->find( "N" )) {
      curName = *ce->find( "N" );
      leFullName->setText( *ce->find( "N" ));
    }
    else {
      curName = "";
      leFullName->setText( "");
    }
    updateFileAs();
  }
}

void ContactDialog::monitorCompany()
{
  const QString *org = ce->find( "ORG" );
  if (org)
    if (*org != curCompany) {
      curCompany = *org;
      updateFileAs();
    }
}

void ContactDialog::updateFileAs()
{
  cbFileAs->clear();
  QString surnameFirst;
  if (ce->find( "N" )) {
    cbFileAs->insertItem( *ce->find( "N" ) );
    cbFileAs->setCurrentItem( 0 );
    if (ce->find( "X-LastName" )) {
      surnameFirst += *ce->find( "X-LastName" );
      if ((ce->find( "X-FirstName" )) || (ce->find( "X-MiddleName" )))
	surnameFirst += ", ";
      if (ce->find( "X-FirstName" ))
	surnameFirst += *ce->find( "X-FirstName" )  + " ";
      if (ce->find( "X-MiddleName" ))
	surnameFirst += *ce->find( "X-MiddleName" );
      surnameFirst = surnameFirst.simplifyWhiteSpace();
      if (surnameFirst != *ce->find( "N" ))
	cbFileAs->insertItem( surnameFirst );
    } else
      surnameFirst = *ce->find( "N" );
    if (ce->find( "ORG" )) {
      cbFileAs->insertItem( *ce->find( "ORG" ) + " (" + surnameFirst + ")");
      cbFileAs->insertItem( surnameFirst + " (" + *ce->find( "ORG" ) + ")");
    }
  }
  else if (ce->find( "ORG" )) {
    cbFileAs->insertItem( *ce->find( "ORG" ) );
    cbFileAs->setCurrentItem( 0 );
  }
}

void ContactDialog::parseName()
{
  if (!ce->find( ".AUXCONTACT-N" ))
    return;
  if (*ce->find( ".AUXCONTACT-N" ) == curName)
    return;
  curName = (*ce->find( ".AUXCONTACT-N" )).simplifyWhiteSpace();
  ce->replace( ".AUXCONTACT-N", new QString( curName ));
  QString name = curName;
  QString prefix;
  QString suffix;
  QString first;
  QString middle;
  QString last;
  
  name = name.simplifyWhiteSpace();
  if (name.find( i18n( "the" ), 0, false ) != 0) {
    QString sTitle[] = {
      "Doctor", "Dr", "Dr.", "Miss", "Mr", "Mr.", 
      "Mrs", "Mrs.", "Ms", "Ms.", "Professor", "Prof.",
      ""
     };
    QString sSuffix[] = {
      "I", "II", "III", "Junior", "Jr.", "Senior", "Sr.",
      ""
    };
    
    for (int i =0; sTitle[i] != ""; ++i )
      if (name.find( i18n( sTitle[i] ), 0, false ) == 0) {
	prefix = i18n( sTitle[i] );
	name = name.right( name.length() - prefix.length() - 1 );
	name = name.simplifyWhiteSpace();
	break;
      }
    
    for (int i =0; sSuffix[i] != ""; ++i ) {
      QString tSuffix = i18n( sSuffix[i] );
      int pos = name.length() - tSuffix.length();
      if ((pos > 0) && (name.findRev( tSuffix, -1, false ) == pos)) {
	suffix = tSuffix;
	name = name.left( pos - 1 );
	name = name.simplifyWhiteSpace();
	break;
      }
    }
    if (name.find( " ", 0 ) > 0 ) {
      int pos = name.findRev( " ", -1 );
      debug( QString().setNum( pos ));
      last = name.mid( pos + 1);
      name = name.left( pos );
      name = name.simplifyWhiteSpace();
      if (name.find( " ", 0 ) > 0 ) {
	pos = name.find( " ", 0 );
	first = name.left( pos );
	name = name.mid( pos + 1 );
	name = name.simplifyWhiteSpace(); 
	middle = name;
      }
      else
	first = name;
    }
    else
      last = name; 
  }    
  ce->replace( "N", new QString( curName ));
  ce->replace( "X-Title", new QString( prefix ) );
  ce->replace( "X-FirstName", new QString( first ) );
  ce->replace( "X-MiddleName", new QString( middle ) );
  ce->replace( "X-LastName", new QString( last ) );
  ce->replace( "X-Suffix", new QString( suffix ) );

  updateFileAs();
}


AddressDialog::AddressDialog( QWidget *parent, 
			      QString entryField, 
			      ContactEntry *ce, 
			      bool modal )
 : QDialog( parent, "", modal ), entryField( entryField), ce( ce )
{
  QString sCountry[] = {
    "Afghanistan", "Albania", "Algeria",
    "American Samoa", "Andorra", "Angola",
    "Anguilla", "Antarctica", "Antigua and Barbuda",
    "Argentina", "Armenia", "Aruba",
    "Ashmore and Cartier Islands", "Australia", "Austria",
    "Azerbaijan", "Bahama", "Bahrain",
    "Bangladesh", "Barbados",
    "Belarus", "Belgium", "Belize",
    "Benin", "Bermuda", "Bhutan",
    "Bolivia", "Bosnia and Herzegovina", "Botswana",
    "Brazil", "Brunei", "Bulgaria",
    "Burkina Faso", "Burundi", "Cambodia",
    "Cameroon", "Canada", "Cape Verde",
    "Cayman Islands", "Central African Republic", "Chad",
    "Chile", "China", "Colombia",
    "Comoros", "Congo", "Congo, Dem. Rep.",
    "Costa Rica", "Côte d'Ivoire", "Croatia",
    "Cuba", "Cyprus", "Czech Republic",
    "Denmark", "Deutschland", "Djibouti",
    "Dominica", "Dominican Republic", "Ecuador",
    "Egypt", "El Salvador", "Equatorial Guinea",
    "Eritrea", "Estonia", "England",
    "Ethiopia", "European Union", "Faroe Islands",
    "Fiji", "Finland", "France",
    "French Polynesia", "Gabon", "Gambia",
    "Georgia", "Germany", "Ghana",
    "Greece", "Greenland", "Grenada",
    "Guam", "Guatemala", "Guinea",
    "Guinea-Bissau", "Guyana", "Haiti", "Holland",
    "Honduras", "Hong Kong", "Hungary",
    "Iceland", "India", "Indonesia",
    "Iran", "Iraq", "Ireland",
    "Israel", "Italy", "Ivory Coast",
    "Jamaica", "Japan", "Jordan",
    "Kazakhstan", "Kenya", "Kiribati",
    "Korea, North", "Korea, South",
    "Kuwait", "Kyrgyzstan", "Laos",
    "Latvia", "Lebanon", "Lesotho",
    "Liberia", "Libya", "Liechtenstein",
    "Lithuania", "Luxembourg", "Macau",
    "Madagascar", "Malawi", "Malaysia",
    "Maldives", "Mali", "Malta",
    "Marshall Islands", "Martinique", "Mauritania",
    "Mauritius", "Mexico", "Micronesia, Federated States Of",
    "Moldova", "Monaco", "Mongolia",
    "Montserrat", "Morocco", "Mozambique",
    "Myanmar", "Nagorno-Karabakh / Artsakh", "Namibia",
    "Nauru", "Nepal", "Netherlands",
    "Netherlands Antilles", "New Caledonia", "New Zealand",
    "Nicaragua", "Niger", "Nigeria",
    "Niue", "North Korea", "Northern Ireland",
    "Northern Mariana Islands", "Norway", "Oman",
    "Pakistan", "Palau", "Palestinian",
    "Panama", "Papua New Guinea", "Paraguay",
    "Perú", "Philippines", "Poland",
    "Portugal", "Puerto Rico", "Qatar",
    "Romania", "Russia", "Rwanda",
    "St. Kitts and Nevis", "St. Lucia", "St. Vincent and the Grenadines",
    "San Marino", "Sao Tome and Principe", "Saudi Arabia",
    "Senegal", "Serbia & Montenegro", "Seychelles",
    "Sierra Leone", "Singapore", "Slovakia",
    "Slovenia", "Solomon Islands", "Somalia",
    "South Africa", "South Korea", "Spain",
    "Sri Lanka", "St. Kitts and Nevis", "Sudan",
    "Suriname", "Swaziland", "Sweden",
    "Switzerland", "Syria", "Taiwan",
    "Tajikistan", "Tanzania", "Thailand",
    "Tibet", "Togo", "Tonga",
    "Trinidad and Tobago", "Tunisia", "Turkey",
    "Turkmenistan", "Turks and Caicos Islands", "Tuvalu",
    "Uganda ", "Ukraine", "United Arab Emirates",
    "United Kingdom", "United States",
    "Uruguay", "Uzbekistan", "Vanuatu",
    "Vatican City", "Venezuela", "Vietnam",
    "Western Samoa", "Yemen",
    "Yugoslavia", "Zaire", "Zambia",
    "Zimbabwe",
    ""
  };

  setCaption( "Address" );
  QGridLayout *hb = new QGridLayout( this, 1, 2, 10 );
  hb->setSpacing( 5 );
  
  QGroupBox *gb = new QGroupBox( this );
  gb->setTitle( i18n( "Address details" ));
  QGridLayout *lay = new QGridLayout( gb, 6, 2, 12 );
  lay->setSpacing( 5 );
  lay->addWidget( new QFrame( gb ), 0, 0 );
  lay->addWidget( new QFrame( gb ), 0, 1 );

  QLabel *lStreet = new QLabel( i18n( "Street" ), gb );
  lStreet->setAlignment( QLabel::AlignTop | QLabel::AlignLeft );
  lay->addWidget( lStreet, 1, 0 );
  mleStreet = new QMultiLineEdit( gb );
  lay->addWidget( mleStreet, 1, 1 );
  if (ce->find( entryField + "Street" ))
    mleStreet->setText( *ce->find( entryField + "Street" ));
  mleStreet->setMinimumSize( mleStreet->sizeHint() );
  lay->addWidget( new QLabel( i18n( "City" ), gb ), 2, 0 );
  leCity = new QLineEdit( gb );
  if (ce->find( entryField + "City" ))
    leCity->setText( *ce->find( entryField + "City" ));
  lay->addWidget( leCity, 2, 1 );
  lay->addWidget( new QLabel( i18n( "State/Province" ), gb ), 3, 0 );
  leState = new QLineEdit( gb );
  if (ce->find( entryField + "State" ))
    leState->setText( *ce->find( entryField + "State" ));
  lay->addWidget( leState, 3, 1 );
  lay->addWidget( new QLabel( i18n( "Zip/Postal Code" ), gb ), 4, 0 );
  lePostal = new QLineEdit( gb );
  if (ce->find( entryField + "PostalCode" ))
    lePostal->setText( *ce->find( entryField + "PostalCode" ));
  lay->addWidget( lePostal, 4, 1 );

  lay->addWidget( new QLabel( i18n("Country"), gb ), 5, 0 );
  cbCountry = new QComboBox( true, gb );
  QString curCountry;
  int cbNum = -1;
  if (ce->find( entryField + "Country" ))
    curCountry = *ce->find( entryField + "Country" );
  for (int i =0; sCountry[i] != ""; ++i )
    cbCountry->insertItem( i18n( sCountry[i] ));
  for (int i =0; sCountry[i] != ""; ++i )
    if (i18n( sCountry[i] ) == curCountry)
      cbNum = i;
  cbCountry->setAutoCompletion( true );
  lay->addWidget( cbCountry, 5, 1 );

  QString language = KGlobal::locale()->language();
  // Try to guess the country the user is in depending
  // on their preferred language.
  // Imperfect but the best I could do.

  QString GuessLanguage[] = {
    "C", "en", "en_AU", "en_UK", "en_NZ", "en_ZA", "da",
    "de", "el", "es", "fi", "fr", "he",
    "hu", "hr", "is", "it", "ko", "nl",
    "no", "pl", "pt", "pt_BR", "ro", "ru",
    "sv", "tr", "zh_CN.GB2312", "zh_TW.Big5", "et", 
    ""
  };
  QString GuessCountry[] = {
    "United States", "United States", "Australia", "United Kingdom",
    "New Zealand", "South Africa", "Denmark",
    "Germany", "Greece", "Spain", "Finland", "French", "Israel",
    "Hungary", "Croatia", "Iceland", "Italy", "South Korea", "Holland",
    "Norway", "Poland", "Portugal", "Brazil", "Romania", "Russia",
    "Sweden", "Turkey", "China", "Taiwan", "Estonia", 
    ""
  };

  int langNum = -1;
  if (cbNum == -1) {
    for (langNum =0; language != GuessLanguage[langNum]; ++langNum )
      if (GuessLanguage[langNum] == "")
	break;
    if (GuessLanguage[langNum] != "")
      for (cbNum =0; sCountry[cbNum] != GuessCountry[langNum]; ++cbNum )
	if (sCountry[cbNum] == "")
	  break;
    if (sCountry[cbNum] == "")
      cbNum = -1;
  }
  if (cbNum != -1)
    cbCountry->setCurrentItem( cbNum );
  if (curCountry != "")
    cbCountry->setEditText( curCountry );    
  hb->addWidget( gb, 0, 0 );

  QFrame *tf = new QFrame( this );
  QVBoxLayout *lay1 = new QVBoxLayout( tf, 10 );
  QPushButton *pbOk = new QPushButton( i18n("OK"), tf );
  lay1->addWidget( pbOk, 0 );
  QPushButton *pbCancel = new QPushButton( i18n("Cancel"), tf );
  lay1->addWidget( pbCancel, 0 );
  lay1->addStretch( 1 );  // Fix the other widgets in place

  hb->addWidget( tf, 0, 1 );
  hb->activate();
  connect( pbOk, SIGNAL( clicked() ), this, SLOT( AddressOk()));
  connect( pbCancel, SIGNAL( clicked() ), this, SLOT( reject()));
}

void AddressDialog::AddressOk() 
{
  QString newAddress = mleStreet->text() + "\n" + leCity->text() + "\n" + leState->text() + "\n" + lePostal->text() + "\n" + cbCountry->currentText();

  ce->replace( entryField, new QString( newAddress ));
  ce->replace( entryField + "City", new QString( leCity->text() ));
  ce->replace( entryField + "Country", new QString( cbCountry->currentText() ));
  ce->replace( entryField + "PostalCode", new QString( lePostal->text() ));
  ce->replace( entryField + "State", new QString( leState->text() ));
  ce->replace( entryField + "Street", new QString( mleStreet->text() ));

  accept();
}

NameDialog::NameDialog( QWidget *parent, ContactEntry *ce, bool modal )
  : QDialog( parent, "", modal ), ce( ce )
{
  QString sTitle[] = {
    "Dr.", "Miss", "Mr.", "Mrs.", "Ms.", "Prof.",
    ""
  };
  QString sSuffix[] = {
    "I", "II", "III", "Jr.", "Sr.",
    ""
  };

  setCaption( "Full Name" );
  QGridLayout *hb = new QGridLayout( this, 1, 2, 10 );
  hb->setSpacing( 5 );
  
  QGroupBox *gb = new QGroupBox( this );
  gb->setTitle( i18n("Name details") );
  QGridLayout *lay = new QGridLayout( gb, 6, 2, 12 );
  lay->setSpacing( 5 );
  lay->addWidget( new QFrame( gb ),0,0);
  lay->addWidget( new QFrame( gb ),0,1);

  lay->addWidget( new QLabel( i18n("Title"), gb ),1,0);
  cbTitle = new QComboBox( true, gb );
  for (int i =0; sTitle[i] != ""; ++i )
    cbTitle->insertItem( i18n( sTitle[i] ));
  if (ce->find( "X-Title" ))
    cbTitle->setEditText( *ce->find( "X-Title" ));
  else
    cbTitle->setEditText( "" );
  lay->addWidget( cbTitle,1,1 );

  lay->addWidget( new QLabel( i18n("First"), gb ), 2,0);
  leFirst = new QLineEdit( gb );
  if (ce->find( "X-FirstName" ))
    leFirst->setText( *ce->find( "X-FirstName" ));
  else
    leFirst->setText( "" );
  lay->addWidget( leFirst, 2, 1 );
  leFirst->setMinimumSize( leFirst->sizeHint() );

  lay->addWidget( new QLabel( i18n("Middle"), gb ), 3, 0 );
  leMiddle = new QLineEdit( gb );
  if (ce->find( "X-MiddleName" ))
    leMiddle->setText( *ce->find( "X-MiddleName" ));
  else
    leMiddle->setText( "" );
  lay->addWidget( leMiddle,3 ,1 );

  lay->addWidget( new QLabel( i18n("Last"), gb ), 4, 0 );
  leLast = new QLineEdit( gb );
  if (ce->find( "X-LastName" ))
    leLast->setText( *ce->find( "X-LastName" ));
  else
    leLast->setText( "" );
  lay->addWidget( leLast, 4, 1 );

  lay->addWidget( new QLabel( i18n("Suffix"), gb ), 5, 0 );
  cbSuffix = new QComboBox( true, gb );
  for (int i =0; sSuffix[i] != ""; ++i )
    cbSuffix->insertItem( i18n( sSuffix[i] ));
  if (ce->find( "X-Suffix" ))
    cbSuffix->setEditText( *ce->find( "X-Suffix" ));
  else
    cbSuffix->setEditText( "" );
  lay->addWidget( cbSuffix, 5, 1 );

  hb->addWidget( gb, 0, 0 );

  QFrame *tf = new QFrame( this );
  QVBoxLayout *lay1 = new QVBoxLayout( tf, 10 );
  QPushButton *pbOk = new QPushButton( i18n("OK"), tf );
  lay1->addWidget( pbOk, 0 );
  QPushButton *pbCancel = new QPushButton( i18n("Cancel"), tf );
  lay1->addWidget( pbCancel, 0 );
  lay1->addStretch( 1 );  // Fix the other widgets in place

  hb->addWidget( tf, 0, 1 );
  hb->activate();
  connect( pbOk, SIGNAL( clicked() ), this, SLOT( NameOk() ));
  connect( pbCancel, SIGNAL( clicked()), this, SLOT( reject() ));
}

void NameDialog::polish()
{
  setMaximumHeight( height() );
}

void NameDialog::NameOk() 
{
  QString name = cbTitle->currentText() + " " + 
    leFirst->text() + " " + 
    leMiddle->text() + " " + 
    leLast->text() + " " 
    + cbSuffix->currentText();
  ce->replace( "N", new QString( name.simplifyWhiteSpace() ));
  ce->replace( "X-Title", new QString( cbTitle->currentText() ));
  ce->replace( "X-FirstName", new QString( leFirst->text() ));
  ce->replace( "X-MiddleName", new QString( leMiddle->text() ));
  ce->replace( "X-LastName", new QString( leLast->text() ));
  ce->replace( "X-Suffix", new QString( cbSuffix->currentText() ));
  accept();
}

void ContactDialog::setSheet(int sheet)
{
  QString allNames[] = {
    "Account", "Anniversary", "Assistant's Name", "Assistant's Phone",
    "Attachment", "Billing Information", "Birthday", "Business Address",
    "Business Address City", "Business Address Country", 
    "Business Address PO Box", "Business Address Postal Code",
    "Business Address State", "Business Address Street", "Business Fax", 
    "Business Home Page", "Business Phone", "Business Phone 2", 
    "Callback", "Car Phone", "Categories", "Children", "City", "Company",
    "Company Main Phone", "Computer Network Name", "Country", "Created",
    "Customer ID", "Department", "E-mail", "E-mail 2",
    "E-mail 3", "File As", "First Name", "FTP Site",
    "Full Name", "Gender", "Government ID Number", "Hobbies",
    "Home Address", "Home Address City", "Home Address Country", 
    "Home Address PO Box", "Home Address Postal Code", "Home Address State", 
    "Home Address Street", "Home Fax", "Home Phone", "Home Phone 2", 
    "Icon", "In Folder", "Initials", "ISDN", "Job Title", "Journal",
    "Language", "Last Name", "Location", "Mailing Address",
    "Manager's Name", "Message Class", "Middle Name", "Mileage",
    "Mobile Phone", "Modified", "Nickname", "Office Location",
    "Organizational ID Number", "Other Address", "Other Address City", 
    "Other Address Country", "Other Address PO Box", 
    "Other Address Postal Code", "Other Address State", 
    "Other Address Street", "Other Fax", "Other Phone",
    "Pager", "Personal Home Page", "PO Box", "Primary Phone",
    "Profession", "Radio Phone", "Read", "Referred By",
    "Sensitivity", "Size", "Spouse", "State",
    "Street Address", "Subject", "Suffix", "Telex",
    "Title", "TTY/TDD Phone", "User Field 1", "User Field 2",
    "User Field 3", "User Field 4", "Web Page", "ZIP/Postal Code",
    ""
  };
  QString allFields[] = {
    "X-Account", "X-Anniversary", "X-AssistantsName", "X-AssistantsPhone",
    "X-Attachment", "X-BillingInformation", "BDAY", "X-BusinessAddress",
    "X-BusinessAddressCity", "X-BusinessAddressCountry", 
    "X-BusinessAddressPOBox", "X-BusinessAddressPostalCode",
    "X-BusinessAddressState", "X-BusinessAddressStreet", "X-BusinessFax", 
    "X-BusinessHomePage", "X-BusinessPhone", "X-BusinessPhone2", 
    "X-Callback", "X-CarPhone", "X-Categories", "X-Children", "X-City", "ORG",
    "X-CompanyMainPhone", "X-ComputerNetworkName", "X-Country", "X-Created",
    "X-CustomerID", "X-Department", "EMAIL", "X-E-mail2",
    "X-E-mail3", "X-FileAs", "X-FirstName", "X-FTPSite",
    "N", "X-Gender", "X-GovernmentIDNumber", "X-Hobbies",
    "X-HomeAddress", "X-HomeAddressCity", "X-HomeAddressCountry", 
    "X-HomeAddressPOBox", "X-HomeAddressPostalCode", "X-HomeAddressState", 
    "X-HomeAddressStreet", "X-HomeFax", "X-HomePhone", "X-HomePhone2", 
    "X-Icon", "X-InFolder", "X-Initials", "X-ISDN", "ROLE", "X-Journal",
    "X-Language", "X-LastName", "X-Location", "X-MailingAddress",
    "X-ManagersName", "X-MessageClass", "X-MiddleName", "X-Mileage",
    "X-MobilePhone", "X-Modified", "X-Nickname", "X-OfficeLocation",
    "X-OrganizationalIDNumber", "X-OtherAddress", "X-OtherAddressCity", 
    "X-OtherAddressCountry", "X-OtherAddressPOBox", 
    "X-OtherAddressPostalCode", "X-OtherAddressState", 
    "X-OtherAddressStreet", "X-OtherFax", "X-OtherPhone",
    "X-Pager", "X-PersonalHomePage", "X-POBox", "X-PrimaryPhone",
    "X-Profession", "X-RadioPhone", "X-Read", "X-ReferredBy",
    "X-Sensitivity", "X-Size", "X-Spouse", "X-State",
    "X-StreetAddress", "X-Subject", "X-Suffix", "X-Telex",
    "X-Title", "X-TtyTddPhone", "X-UserField1", "X-UserField2",
    "X-UserField3", "X-UserField4", "WEBPAGE", "X-ZIPPostalCode",
    "" 
  };
    
  QString frequentNames[] = {
    "Assistant's Phone", "Attachment", "Business Address",
    "Business Fax", "Business Home Page", "Business Phone", 
    "Business Phone2", "Callback", "Car Phone", "Categories",
    "Company", "Company Main Phone", "Country",
    "Department", "E-mail", "E-mail 2", "E-mail 3", 
    "File As", "First Name", "Full Name", "Home Address", 
    "Home Fax", "Home Phone", "Home Phone 2", 
    "Icon", "ISDN", "Job Title", "Journal",
    "Last Name", "Mailing Address", "Mobile Phone", 
    "Modified", "Office Location", "Other Address", 
    "Other Address City", "OtherFax", "OtherPhone",
    "Pager", "Personal Home Page", "Primary Phone",
    "Radio Phone", "Sensitivity", "State", "Telex",
    "TTY/TDD Phone", "Web Page",
    "" 
  };
  QString frequentFields[] = {
    "X-AssistantsPhone", "X-Attachment", "X-BusinessAddress",
    "X-BusinessFax", "X-BusinessHomePage", "X-BusinessPhone", 
    "X-BusinessPhone2", "X-Callback", "X-CarPhone", "X-Categories",
    "ORG", "X-CompanyMainPhone", "X-Country",
    "X-Department", "EMAIL", "X-E-mail2", "X-E-mail3", 
    "X-FileAs", "X-FirstName", "N", "X-HomeAddress", 
    "X-HomeFax", "X-HomePhone", "X-HomePhone2", 
    "X-Icon", "X-ISDN", "ROLE", "X-Journal",
    "X-LastName", "X-MailingAddress", "X-MobilePhone", 
    "X-Modified", "X-OfficeLocation", "X-OtherAddress", 
    "X-OtherAddressCity", "X-OtherFax", "X-OtherPhone",
    "X-Pager", "X-PersonalHomePage", "X-PrimaryPhone",
    "X-RadioPhone", "X-Sensitivity", "X-State", "X-Telex",
    "X-TtyTddPhone", "WEBPAGE",
    "" 
  };


  QString addressNames[] = { 
    "City", "Country", "Department",
    "Home Address", "Home Address City", "Home Address Country",
    "Home Address PO Box", "Home Address Postal Code", "Home Address State",
    "Home Address Street", "Location", "Mailing Address",
    "Office Location", "Other Address", "Other Address City",
    "Other Address Country", "Other Address PO Box", "Other Address Postal Code",
    "Other Address State", "Other Address Street", "PO Box",
    "State", "Street Address", "ZIP/Postal Code",
    ""
  };
  QString addressFields[] = { 
    "X-City", "X-Country", "X-Department",
    "X-HomeAddress", "X-HomeAddressCity", "X-HomeAddressCountry",
    "X-HomeAddressPOBox", "X-HomeAddressPostalCode", "X-HomeAddressState",
    "X-HomeAddressStreet", "X-Location", "X-MailingAddress",
    "X-OfficeLocation", "X-OtherAddress", "X-OtherAddressCity",
    "X-OtherAddressCountry", "X-OtherAddressPOBox", "X-OtherAddressPostalCode",
    "X-OtherAddressState", "X-OtherAddressStreet", "X-POBox",
    "X-State", "X-StreetAddress", "X-ZIPPostalCode",
    ""
  };

  QString emailNames[] = { "E-mail", "E-mail 2", "E-mail 3", "" };
  QString emailFields[] = { "EMAIL", "X-E-mail2", "X-E-mail3", "" };

  QString faxNames[] = { 
    "Business Fax", "Computer Network Name", "FTP Site",		
    "Home Fax", "ISDN",	"Other Fax", "Telex",
    ""
  };
  QString faxFields[] = {
    "X-BusinessFax", "X-ComputerNetworkName", "X-FTPSite",		
    "X-HomeFax", "X-ISDN", "X-OtherFax", "X-Telex",
    ""
  };

  QString miscNames[] = { 
    "Account", "Assistant's Name", "Assistant's Phone",
    "Computer Network Name", "Customer ID", "Department",
    "FTP Site", "Government ID Number", "Organizational ID Number",
    "User Field 1", "User Field 2", "User Field 3", "User Field 4",
    ""
  };
  QString miscFields[] = { 
    "X-Account", "X-AssistantsName", "X-AssistantsPhone",
    "X-ComputerNetworkName", "X-CustomerID", "X-Department",
    "X-FTPSite", "X-GovernmentIDNumber", "X-OrganizationalIDNumber",
    "X-UserField1", "X-UserField2", "X-UserField3", "X-UserField4",
    ""
  };

  QString nameNames[] = {
    "Assistant's Name", "Children", "File As",
    "First Name", "Full Name", "Initials",	
    "Job Title", "Last Name", "Manager's Name",
    "Middle Name", "Nickname", "Referred By",
    "Spouse", "Suffix", "Title",
    ""
  };
  QString nameFields[] = {
    "X-AssistantsName", "X-Children", "X-FileAs",
    "X-FirstName", "N", "X-Initials",	
    "ROLE", "X-LastName", "X-ManagersName",
    "X-MiddleName", "X-Nickname", "X-ReferredBy",	
    "X-Spouse", "X-Suffix", "X-Title",
    ""
  };

  QString personalNames[] = {
    "Anniversary", "Birthday", "Children",
    "Gender", "Hobbies", "Language",
    "Profession", "Referred By", "Spouse",
    "Web Page",
    ""
  };
  QString personalFields[] = {
    "X-Anniversary", "BDAY", "X-Children",
    "X-Gender", "X-Hobbies", "X-Language",
    "X-Profession", "X-ReferredBy", "X-Spouse",
    "WEBPAGE",
    ""
  };
  QString phoneNames[] = {
    "Assistant's Phone", "Business Phone", "Business Phone 2",
    "Callback", "Car Phone", "Company Main Phone",
    "Home Phone", "Home Phone 2", "Mobile Phone",
    "Other Phone", "Pager", "Primary Phone",
    "Radio Phone", "TTY/TDD Phone",
    ""
  };
  QString phoneFields[] = {
    "X-AssistantsPhone", "X-BusinessPhone", "X-BusinessPhone2",
    "X-Callback", "X-CarPhone", "X-CompanyMainPhone",
    "X-HomePhone", "X-HomePhone2", "X-MobilePhone",
    "X-OtherPhone", "X-Pager", "X-PrimaryPhone",
    "X-RadioPhone", "X-TttyTddPhone",
    ""
  };

  QString *sNames = 0, *sFields = 0;
  QStringList names;
  QStringList fields;
  bool custom = false;

  switch (sheet) {
  case 0:
    sNames = allNames;
    sFields = allFields;
    break;
  case 1:
    sNames = frequentNames;
    sFields = frequentFields;
    break;
  case 2:
    sNames = addressNames;
    sFields = addressFields;
    break;
  case 3:
    sNames = emailNames;
    sFields = emailFields;
    break;
  case 4:
    sNames = faxNames;
    sFields = faxFields;
    break;  
  case 5:
    sNames = miscNames;
    sFields = miscFields;
    break;  
  case 6:
    sNames = nameNames;
    sFields = nameFields;
    break;  
  case 7:
    sNames = personalNames;
    sFields = personalFields;
    break;  
  case 8:
    sNames =  phoneNames;
    sFields = phoneFields;
    break;  
  default:
    custom = true;
    fields = ce->custom();
    fields.sort();
    for (int i = 0; i < (int)fields.count(); ++i )
      names += fields[i].mid( 9 );
    break;
  }

  QString tmp;
  if (!custom) {
    for (int i = 0; tmp = sNames[i], tmp != ""; ++i )
      names += i18n( tmp );
    for (int i = 0; tmp = sFields[i], tmp != ""; ++i )
      fields += i18n( tmp );
  }
  
  delete vs;
  vs = new NameValueSheet( 0, (int)names.count(), names, fields, ce );
  vp->setSheet( vs );
}

NewFieldDialog::NewFieldDialog( QWidget *parent, bool modal = false )
  : QDialog( parent, "", modal )
    {
  setCaption( "Create Custom Field" );

  QGridLayout *hbl = new QGridLayout( this, 3, 2, 10 );
  hbl->setSpacing( 5 );
  
  QLabel *lField = new QLabel( i18n( "Field name" ), this );
  hbl->addWidget( lField, 0, 0 );
  lField->setAlignment( QLabel::AlignTop | QLabel::AlignLeft );
  leField = new QLineEdit( this );
  hbl->addWidget( leField, 0, 1 );
  QLabel *lValue = new QLabel( i18n( "Value" ), this );
  hbl->addWidget( lValue, 1, 0 );
  lValue->setAlignment( QLabel::AlignTop | QLabel::AlignLeft );
  leValue = new QLineEdit( this );
  hbl->addWidget( leValue, 1, 1 );

  QHBox *tf = new QHBox( this );
  tf->setSpacing( 10 );
  QPushButton *pbOk = new QPushButton( i18n("OK"), tf );
  QPushButton *pbCancel = new QPushButton( i18n("Cancel"), tf );

  hbl->addMultiCellWidget( tf, 2, 2, 0, 1, QGridLayout::AlignRight );

  hbl->activate();
  setMinimumSize( sizeHint() );
  resize( sizeHint() );
  setMaximumHeight( height() );

  connect( pbOk, SIGNAL( clicked() ), this, SLOT( accept() ));
  connect( pbCancel, SIGNAL( clicked()), this, SLOT( reject() ));
};

QString NewFieldDialog::field() const
{
  return "X-CUSTOM-" + leField->text();
}

QString NewFieldDialog::value() const
{
  return leValue->text();
}
