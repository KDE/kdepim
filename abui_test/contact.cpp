/* This file is part of KDE PIM
   Copyright (C) 1999 Don Sanders <dsanders@kde.org>

   License: GNU GPL
*/

#include "contact.h"
#include <qtabwidget.h>
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

#include "namevalue.h"
#include "contactentry.h"
#include "datepickerdialog.h"
#include "klocale.h"
#include "kapp.h" // for kapp->palette()
#include "kglobal.h"

ContactDialog::ContactDialog(QWidget *parent, const char *name, ContactEntry *ce)
  : QTabWidget(parent, name), vs( 0 ), vp( 0 )
{
    ce ? this->ce = ce : this->ce = new ContactEntry();
    if (ce->find( "N" ))
      curName = *ce->find( "N");
    setupTab1();
    setupTab2();
    setupTab3();
    ce->touch();
    setMinimumSize(minimumSizeHint());
}

ContactEntry* ContactDialog::entry()
{
  return ce;
}

void ContactDialog::setupTab1()
{
  QHBox *tabAux = new QHBox( this );
  QVBox *tab1 = new QVBox( tabAux );
  tab1->setMargin( 5 );
  
/////////////////////////////////
// The Name/Job title group
  QFrame *h1 = new QFrame( tab1 );
  QGridLayout *lay0 = new QGridLayout( h1, 2, 5 );
  lay0->setSpacing( 5 );
  lay0->setAutoAdd( true );

  // First row
  QPushButton *pbFullName = new QPushButton( "&Full Name...", h1 );
  leFullName = new ContactLineEdit( h1, ".AUXCONTACT-N", ce );
  leFullName->setText( curName );
  connect( ce, SIGNAL( changed() ), this, SLOT( parseName() ));
  connect( pbFullName, SIGNAL( clicked()), this, SLOT( newNameDialog()));

  QFrame * filler1 = new QFrame( h1, "filler1" );
  filler1->setFrameStyle( QFrame::NoFrame );
  filler1->setMinimumWidth( 1 );

  QLabel *lJobTitle = new QLabel( "&Job title:", h1 );
  QLineEdit *leJobTitle = new ContactLineEdit( h1, "ROLE", ce );
  lJobTitle->setBuddy( leJobTitle );

  // Second row
  QLabel *lCompany = new QLabel( "&Company:", h1 );
  QLineEdit *leCompany = new ContactLineEdit( h1, "ORG", ce );
  lCompany->setBuddy( leCompany );
  curCompany = leCompany->text();
  connect( ce, SIGNAL( changed() ), this, SLOT( monitorCompany() ));

  QFrame * filler2 = new QFrame( h1, "filler2" );
  filler2->setFrameStyle( QFrame::NoFrame );
  filler2->setMinimumWidth( 1 );

  QLabel *lFileAs = new QLabel( "F&ile as:", h1 );
  cbFileAs = new QComboBox( true, h1, "cbFileAs" );
  cbFileAs->insertItem( "Sanders, Don" );

  lFileAs->setBuddy( cbFileAs );
// End the Name/Job title group
////////////////////////////////

  // Horizontal bar (rather verbose)
  QFrame *bar1 = new QFrame( tab1, "horizontal divider 1" );
  bar1->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  bar1->setMinimumHeight( 10 );
  QFrame *bar2 = new QFrame( tab1, "horizontal blank 1" );
  bar2->setFrameStyle( QFrame::NoFrame );  
  bar2->setMinimumHeight( 6 );

////////////////////////////////
// The Email/Webpage group
  QHBox *h4 = new QHBox( tab1 );
  h4->setSpacing( 5 );

  ContactComboBox *cbEmail = new ContactComboBox( h4 );
  cbEmail->insertItem( "E-mail", "EMAIL" );
  cbEmail->insertItem( "E-mail 2", "X-E-mail2" );
  cbEmail->insertItem( "E-mail 3", "X-E-mail3" );
  QLineEdit *leEmail = new ContactLineEdit( h4, "EMAIL", ce ); 
  cbEmail->setBuddy( leEmail );

  QFrame * filler4 = new QFrame( h4, "filler4" );
  filler4->setFrameStyle( QFrame::NoFrame );
  filler4->setMinimumWidth( 1 );

  QLabel *lWebPage = new QLabel( "&Web page:", h4 );  
  QLineEdit *leWebPage = new ContactLineEdit( h4, "WEBPAGE", ce );
  lWebPage->setBuddy( leWebPage );
// End the Email/Webpage group
///////////////////////////////

  // Horizontal bar (rather verbose)
  QFrame *f5 = new QFrame( tab1, "horizontal divider 2" );
  f5->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  f5->setMinimumHeight( 10 );
  QFrame *f6 = new QFrame( tab1, "horizontal blank 2" );
  f6->setFrameStyle( QFrame::NoFrame );  
  f6->setMinimumHeight( 6 );

///////////////////////////////
// The Address/Phone group
  QHBox *h3 = new QHBox( tab1 );
  h3->setSpacing( 5 );

  // Use a box to keep the widgets fixed vertically in place
  QFrame *v1 = new QFrame( h3 );
  QBoxLayout *lay1 = new QBoxLayout( v1, QBoxLayout::Down, 1, 1, "h3BoxLayout" );
  lay1->setSpacing( 10 );
  QPushButton *pbAddress = new QPushButton( "Add&ress...", v1 );
  connect( pbAddress, SIGNAL( clicked()), this, SLOT( newAddressDialog()));
  lay1->addWidget( pbAddress, 0 );
  cbAddress = new ContactComboBox( v1 );
  cbAddress->insertItem( "Business", "X-BusinessAddress" );
  cbAddress->insertItem( "Home", "X-HomeAddress" );
  cbAddress->insertItem( "Other", "X-OtherAddress" );
  lay1->addWidget( cbAddress, 0 );
  lay1->addStretch( 1 );  // Fix the other widgets in place

  // Perhaps the address "subfields" (city, postal, country) should be cleared
  // when this control loses focus. They aren't at the moment.
  QMultiLineEdit *mleAddress = new ContactMultiLineEdit( h3, "X-BusinessAddress", ce );
  cbAddress->setBuddy( mleAddress );

  QFrame * filler3 = new QFrame( h3, "filler3" );
  filler3->setFrameStyle( QFrame::NoFrame );
  filler3->setMinimumWidth( 1 );

  QLabel *lPhone = new QLabel( "Phone:", h3 );  
  lPhone->setAlignment( QLabel::AlignTop | QLabel::AlignLeft );

  // Use a boxlayout to keep the widgets position fixed vertically
  QFrame *v2 = new QFrame( h3 );
  QBoxLayout *lay2 = new QBoxLayout( v2, QBoxLayout::TopToBottom, 1, 1, "h3BoxLayout" );
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

  int v2Width = 0;
  for ( int row = 0; row < numRows; row++ ) {
    QGrid *hGrid = new QGrid ( 2, QGrid::Horizontal, v2 );
    hGrid->setSpacing( 10 );

    ContactComboBox *cbPhone = new ContactComboBox( hGrid );
    for (int i =0; sPhone[i] != ""; ++i )
      cbPhone->insertItem( i18n( sPhone[i] ), vPhone[i] );
    cbPhone->setCurrentItem( iPhone[row] );
    cbPhone->setMinimumSize( cbPhone->sizeHint() );

    QLineEdit *ed = new ContactLineEdit( hGrid, vPhone[iPhone[row]], ce ); 
    ed->setMinimumSize( ed->sizeHint());
    cbPhone->setBuddy( ed );

    v2Width = cbPhone->sizeHint().width() + 10 + ed->sizeHint().width();
    lay2->addWidget( hGrid, 0 );
  }
  lay2->addStretch( 1 ) ;

  // Rather awkward horizontal bar
  QFrame *f3 = new QFrame( tab1, "horizontal divider 3" );
  f3->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  f3->setMinimumHeight( 10 );
  QFrame *f4 = new QFrame( tab1, "horizontal blank 3" );
  f4->setFrameStyle( QFrame::NoFrame );  
  f4->setMinimumHeight( 6 );
// End The Address/Phone group
///////////////////////////////

//////////////////////
// The Note group
  // Interestingly this doesn't have an equivalent in tab3
  QMultiLineEdit *mleNotes = new ContactMultiLineEdit( tab1, "X-Notes", ce );
  mleNotes->setMinimumSize( mleNotes->sizeHint() );
// End the Note group
//////////////////////

  // Make widgets in first "column" same size
  QSize tSz1 = pbFullName->sizeHint().expandedTo( lCompany->sizeHint()).expandedTo( pbAddress->sizeHint()).expandedTo( cbAddress->sizeHint() ).expandedTo( cbEmail->sizeHint() );;
  pbFullName->setMinimumSize( tSz1 );
  lCompany->setMinimumSize( tSz1 );
  pbAddress->setMinimumSize( tSz1 );
  cbAddress->setMinimumSize( tSz1 );
  cbEmail->setMinimumSize( tSz1 );

  // Make widgets in second "column" same size
  QSize tSz2 = leFullName->sizeHint().expandedTo( leCompany->sizeHint()).expandedTo( mleAddress->sizeHint()).expandedTo( leEmail->sizeHint());
  QSize tSz4 = leJobTitle->sizeHint().expandedTo( cbFileAs->sizeHint()).expandedTo( leWebPage->sizeHint()).expandedTo( QSize( v2Width, 0 ));
  tSz2 = tSz2.expandedTo( tSz4 );
  tSz2.setHeight( 0 ); // Only expand horizontally

  leFullName->setMinimumSize( leFullName->sizeHint().expandedTo( tSz2 ));
  leCompany->setMinimumSize( leCompany->sizeHint().expandedTo( tSz2 ));
  // For some reason the mleAddress widget is a bit smaller than
  // the others even after this correction
  mleAddress->setMinimumSize( mleAddress->sizeHint().expandedTo( tSz2 ));
  leEmail->setMinimumSize( leCompany->sizeHint().expandedTo( tSz2 ));

  // Make widgets in third "column" same size
  QSize tSz3 = lJobTitle->sizeHint().expandedTo( lFileAs->sizeHint()).expandedTo( lPhone->sizeHint()).expandedTo( lWebPage->sizeHint());
  lJobTitle->setMinimumSize( tSz3 );
  lFileAs->setMinimumSize( tSz3 );
  lPhone->setMinimumSize( tSz3 );
  lWebPage->setMinimumSize( tSz3 );

  // Make widgets in fourth "column" same size
  leJobTitle->setMinimumSize( tSz2 );
  cbFileAs->setMinimumSize( tSz2 );
  leWebPage->setMinimumSize( tSz2 );

  // Use blank and tabAux widgets to force the dialog to be big enough to 
  // show the entire tab (it's a cludge)
  QFrame *blank  = new QFrame( tabAux );
  blank->setFrameStyle( QFrame::NoFrame );
  blank->setMinimumWidth( 5 );
  lay0->activate();

  addTab( tabAux, "&General" );
}

void ContactDialog::setupTab2()
{
  // Use a boxlayout to keep the widgets position fixed vertically
  QFrame *v2 = new QFrame( this );
  QBoxLayout *lay2 = new QBoxLayout( v2, QBoxLayout::TopToBottom, 5, 5, "h3BoxLayout" );
  lay2->setSpacing( 10 );

  const int numRows = 9;
  QString sLabel[numRows] = { "De&partment:", "&Office:", "&Profession:", 
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
    new ContactLineEdit( hGrid, entryField[row], ce ); 
    lay2->addWidget( hGrid, 0 );
  }

  for ( int row = 0; row < numRows; row++ ) {
    label[row]->resize( size );
    label[row]->setMinimumSize( size );
  }
  pbDate[0]->setMinimumSize( size2 );
  pbDate[1]->setMinimumSize( size2 );

  lay2->addStretch( 1 ) ;
  addTab( v2, "&Details" );

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
    addTab( tab3, "&All fields" );
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
  gb->setTitle( i18n( "Address details" ) );
  QGridLayout *lay = new QGridLayout( gb, 4, 2, 12 );
  lay->setSpacing( 5 );
  lay->setAutoAdd( true );
  new QFrame( gb );
  new QFrame( gb );

  QLabel *lStreet = new QLabel( i18n( "Street" ), gb );
  lStreet->setAlignment( QLabel::AlignTop | QLabel::AlignLeft );
  mleStreet = new QMultiLineEdit( gb );
  if (ce->find( entryField + "Street" ))
    mleStreet->setText( *ce->find( entryField + "Street" ));
  mleStreet->setMinimumSize( mleStreet->sizeHint() );
  new QLabel( i18n( "City" ), gb );
  leCity = new QLineEdit( gb );
  if (ce->find( entryField + "City" ))
    leCity->setText( *ce->find( entryField + "City" ));
  new QLabel( i18n( "State/Province" ), gb );
  leState = new QLineEdit( gb );
  if (ce->find( entryField + "State" ))
    leState->setText( *ce->find( entryField + "State" ));
  new QLabel( i18n( "Zip/Postal Code" ), gb );
  lePostal = new QLineEdit( gb );
  if (ce->find( entryField + "PostalCode" ))
    lePostal->setText( *ce->find( entryField + "PostalCode" ));

  new QLabel( i18n("Country"), gb );
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

  //  lay->activate();
  hb->addWidget( gb, 0, 0 );

  QFrame *tf = new QFrame( this );
  QVBoxLayout *lay1 = new QVBoxLayout( tf, 10 );
  QPushButton *pbOk = new QPushButton( i18n("OK"), tf );
  lay1->addWidget( pbOk, 0 );
  QPushButton *pbCancel = new QPushButton( i18n("Cancel"), tf );
  lay1->addWidget( pbCancel, 0 );
  lay1->addStretch( 1 );  // Fix the other widgets in place

  //  lay1->activate();
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
  QGridLayout *lay = new QGridLayout( gb, 5, 2, 12 );
  lay->setSpacing( 5 );
  lay->setAutoAdd( true );
  new QFrame( gb );
  new QFrame( gb );

  new QLabel( i18n("Title"), gb );
  cbTitle = new QComboBox( true, gb );
  for (int i =0; sTitle[i] != ""; ++i )
    cbTitle->insertItem( i18n( sTitle[i] ));
  if (ce->find( "X-Title" ))
    cbTitle->setEditText( *ce->find( "X-Title" ));
  else
    cbTitle->setEditText( "" );

  new QLabel( i18n("First"), gb );
  leFirst = new QLineEdit( gb );
  if (ce->find( "X-FirstName" ))
    leFirst->setText( *ce->find( "X-FirstName" ));
  else
    leFirst->setText( "" );

  leFirst->setMinimumSize( leFirst->sizeHint() );
  new QLabel( i18n("Middle"), gb );
  leMiddle = new QLineEdit( gb );
  if (ce->find( "X-MiddleName" ))
    leMiddle->setText( *ce->find( "X-MiddleName" ));
  else
    leMiddle->setText( "" );
  new QLabel( i18n("Last"), gb );
  leLast = new QLineEdit( gb );
  if (ce->find( "X-LastName" ))
    leLast->setText( *ce->find( "X-LastName" ));
  else
    leLast->setText( "" );

  new QLabel( i18n("Suffix"), gb );
  cbSuffix = new QComboBox( true, gb );
  for (int i =0; sSuffix[i] != ""; ++i )
    cbSuffix->insertItem( i18n( sSuffix[i] ));
  if (ce->find( "X-Suffix" ))
    cbSuffix->setEditText( *ce->find( "X-Suffix" ));
  else
    cbSuffix->setEditText( "" );

  lay->activate();
  hb->addWidget( gb, 0, 0 );

  QFrame *tf = new QFrame( this );
  QVBoxLayout *lay1 = new QVBoxLayout( tf, 10 );
  QPushButton *pbOk = new QPushButton( i18n("OK"), tf );
  lay1->addWidget( pbOk, 0 );
  QPushButton *pbCancel = new QPushButton( i18n("Cancel"), tf );
  lay1->addWidget( pbCancel, 0 );
  lay1->addStretch( 1 );  // Fix the other widgets in place

  lay1->activate();
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
