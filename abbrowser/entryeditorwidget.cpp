/* This file is part of KDE PIM
   Copyright (C) 1999 Don Sanders <dsanders@kde.org>

   License: GNU GPL
*/

#include <Entity.h>
#include <Field.h>
#include <KAddressBookInterface.h>

#include "entryeditorwidget.h"

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
#include <qfile.h>
#include <qtextstream.h>
#include <qmap.h>

#include <kstddirs.h>

#include "namevaluewidget.h"
#include "attributes.h"
#include "datepickerdialog.h"
#include <klocale.h>
#include <kglobal.h>

ContactDialog::ContactDialog( QWidget *parent, const char *name, Entity *ce, bool modal )
  : QDialog( parent, name, modal ), vs( 0 ), vp( 0 )
{
    ce ? this->ce = ce : this->ce = new Entity;
    
    Field f = ce->field("N");
    if (!f.isNull())
      curName = QString(f.value());
    
    setCaption( name );

    QVBoxLayout *vb = new QVBoxLayout( this, 5 );
    tabs = new QTabWidget( this );
    vb->addWidget( tabs );
    tabs->setMargin( 4 );

    setupTab1();
    setupTab2();
    setupTab3();

    QHBoxLayout *hb = new QHBoxLayout( vb, 5 );
    hb->addStretch( 1 );
    pbOk = new QPushButton( i18n("&OK"), this );
    hb->addWidget( pbOk, 0 );
    pbOk->setDefault( true );
    pbOk->setAutoDefault( true );
    QPushButton *pbCancel = new QPushButton( i18n("Cancel"), this );
    hb->addWidget( pbCancel, 0 );

    connect( pbOk, SIGNAL( clicked() ), this, SLOT( ok()));
    connect( pbCancel, SIGNAL( clicked() ), this, SLOT( reject()));
    leFullName->setFocus();

    ce->touch();
}

void ContactDialog::ok()
{
  pbOk->setFocus();
  accept();
  emit accepted();
}

Entity * ContactDialog::entry()
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

//  connect( ce, SIGNAL( changed() ), this, SLOT( parseName() ));
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
//  connect( ce, SIGNAL( changed() ), this, SLOT( monitorCompany() ));
  tab1lay->addWidget( lCompany, 1, 0 );
  tab1lay->addWidget( leCompany, 1, 1 );

  QFrame * filler2 = new QFrame( tab1, "filler2" );
  filler2->setFrameStyle( QFrame::NoFrame );
  filler2->setMinimumWidth( 1 );
  tab1lay->addWidget( filler2, 1, 2 );

  QLabel *lFileAs = new QLabel( "F&ile as:", tab1 );
  cbFileAs = new FileAsComboBox( tab1, "X-FileAs", ce );
  QString sFileAs;
  
  Field f = ce->field("X-FileAs");
  if (!f.isNull())
    sFileAs = QString(f.value());
  
  updateFileAs();
  if (sFileAs != "")
    ce->replace( "X-FileAs", sFileAs);
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

  QStringList addresses;
  addresses << i18n("Business") << i18n("Home") << i18n("Other");
  addresses.sort();

  QString addressName;
  addressName = Attributes::instance()->nameToField( addresses[0] );
  cbAddress->insertItem( addresses[0], addressName );
  addressName = Attributes::instance()->nameToField( addresses[1] );
  cbAddress->insertItem( addresses[1], addressName );
  addressName = Attributes::instance()->nameToField( addresses[2] );
  cbAddress->insertItem( addresses[2], addressName );
  cbAddress->setCurrentItem( addresses.findIndex( i18n( "Business" )));
  
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

  QLabel *lPhone = new QLabel( i18n( "Phone" ) + ":", tab1 );  
  lPhone->setAlignment( QLabel::AlignTop | QLabel::AlignLeft );
  tab1lay->addWidget( lPhone, 5, 3 );

  QBoxLayout *lay2 = new QBoxLayout( QBoxLayout::TopToBottom, 1 );
  lay2->setSpacing( 10 );
  const int numRows = 4;

  QStringList namePhone;
  QStringList fieldPhone;
  Attributes::instance()->nameFieldList( 8, &namePhone, &fieldPhone );
    
  int iPhone[4];
  iPhone[0] = fieldPhone.findIndex( "X-BusinessPhone" );
  iPhone[1] = fieldPhone.findIndex( "X-HomePhone" );
  iPhone[2] = fieldPhone.findIndex( "X-BusinessFax" );
  iPhone[3] = fieldPhone.findIndex( "X-MobilePhone" );

  QGridLayout *layhGrid = new QGridLayout( numRows, 2 );
  layhGrid->setSpacing( 10 );
  for ( int row = 0; row < numRows; row++ ) {
    QFrame *hGrid = tab1;

    ContactComboBox *cbPhone = new ContactComboBox( hGrid );
    //    for (int i =0; sPhone[i] != ""; ++i )
    //      cbPhone->insertItem( i18n( sPhone[i] ), vPhone[i] );
    for( unsigned int i = 0; i < namePhone.count(); ++i )
      cbPhone->insertItem(  namePhone[i], fieldPhone[i] );
    cbPhone->setCurrentItem( iPhone[row] );
    cbPhone->setMinimumSize( cbPhone->sizeHint() );
    layhGrid->addWidget( cbPhone, row, 0 );

    QLineEdit *ed = new ContactLineEdit( hGrid, fieldPhone[iPhone[row]], ce ); 
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
  QString sLabel[numRows] = { i18n( "D&epartment:" ), i18n( "&Office:" ),
			      i18n( "&Profession:" ), 
			      i18n( "Assistant's &Name:" ), 
			      i18n( "&Managers's Name:" ),
			      i18n( "Birthday" ), i18n( "Anniversary" ), 
			      i18n( "Ni&ckname:" ), i18n( "&Spouse's Name:" )
  };
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
    
    int i = 0;
    tmp = Attributes::instance()->fieldListName( i++ );
    
    while (tmp) {
      cbSelectFrom->insertItem( tmp );
      tmp = Attributes::instance()->fieldListName( i++ );
    }

    cbSelectFrom->insertItem( i18n( "User-defined fields in folder" )); 

    cbSelectFrom->setCurrentItem( cbSelectFrom->count() - 1 );
//    fields = ce->custom(); FIXME
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
    ce->replace( "BDAY", datePicker->getDate().toString());
    // ce autoDelete will clean it up
  delete datePicker;
}

void ContactDialog::pickAnniversaryDate() 
{
  DatePickerDialog* datePicker=new DatePickerDialog( "Select Anniversary", this);
  datePicker->setDate(QDate::currentDate());
  if(datePicker->exec())
    ce->replace( "X-Anniversary", datePicker->getDate().toString());
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
    ce->replace( fd->field(), fd->value());
    cbSelectFrom->setCurrentItem( 9 );
    setSheet( 9 );
  }
}

// We want to update the fileas field using updateFileAs but not 
// the automatically the parse the name into its components
// with parseName
void ContactDialog::newNameDialog()
{
  debug( "newNameDialog " + leFullName->text() );
  Field f = ce->field(".AUXCONTACT-N");
  
  if (!f.isNull() && (leFullName->text() != QString(f.value())) || (f.isNull())) {

    ce->replace( ".AUXCONTACT-N", leFullName->text());
    parseName();
  }
  
  QDialog *nd = new NameDialog( this, ce, true );
  if (nd->exec()) {
    
    Field f = ce->field("N");
    if (!f.isNull()) {
      curName = QString(f.value());
      ce->replace( ".AUXCONTACT-N", curName);
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
  Field f = ce->field("ORG");
  
  if (f.isNull())
    return;
  
  QString org = QString(f.value());
  if (!(org.isEmpty()))
    if (org != curCompany) {
      curCompany = org;
      updateFileAs();
    }
}

void ContactDialog::updateFileAs()
{
  debug( "updateFileAs" );
  cbFileAs->clear();
  QString surnameFirst;

  Field f = ce->field("N");
  
  if (!f.isNull()) {
    
    cbFileAs->insertItem(QString(f.value()));
    cbFileAs->setCurrentItem( 0 );
    //    cbFileAs->updateContact();
    f = ce->field("X-LastName");
    
    if (!f.isNull()) {
      
      surnameFirst += QString(f.value());

      if (
        (!ce->field("X-FirstName").isNull()) ||
        (!ce->field("X-MiddleName").isNull()))
	      surnameFirst += ", ";
      
      if (!ce->field("X-FirstName").isNull())
	      surnameFirst += QString(f.value()) + " ";
      
      if (!ce->field("X-MiddleName").isNull())
	      surnameFirst += QString(f.value());
      
      surnameFirst = surnameFirst.simplifyWhiteSpace();
      
      f = ce->field("N");

      if (!f.isNull()) // Should be ok - got it earlier.
        if (surnameFirst != QString(f.value()))
	        cbFileAs->insertItem( surnameFirst );
    
    } else {
      
      f = ce->field("N");
      if (!f.isNull())
      surnameFirst = QString(f.value());
    }


    f = ce->field("ORG");
    
    if (!f.isNull()) {
      cbFileAs->insertItem( QString(f.value()) + " (" + surnameFirst + ")");
      cbFileAs->insertItem( surnameFirst + " (" + QString(f.value()) + ")");
    }
  }
  else {
    f = ce->field("ORG");
    if (!f.isNull()) {
      cbFileAs->insertItem(QString(f.value()));
      cbFileAs->setCurrentItem( 0 );
      // cbFileAs->updateContact();
    }
  }
}

// We don't want to reparse the "N" field if the newNameDialog
// has been used to enter the name 
void ContactDialog::parseName()
{
  debug( "parseName()" );
  Field f = ce->field(".AUXCONTACT-N");
  if (f.isNull())
    return;
  //  debug( ".AUX" + *ce->field( ".AUXCONTACT-N" ) + " curname " + curName);
  if (QString(f.value()) == curName)
    return;
  curName = QString(f.value()).simplifyWhiteSpace();
  //  debug( "curName " + curName );
  ce->replace( ".AUXCONTACT-N", curName);
  QString name = curName;
  QString prefix;
  QString suffix;
  QString first;
  QString middle;
  QString last;
  
  name = name.simplifyWhiteSpace();
  if (name.find( i18n( "the" ), 0, false ) == 0) {
    QString sTitle[] = {
      i18n( "Doctor" ), i18n( "Dr." ), i18n( "Dr" ), i18n( "Miss" ), 
      i18n ( "Mr." ), i18n( "Mr" ), i18n( "Mrs." ), i18n( "Mrs" ),
      i18n( "Ms." ), i18n( "Ms" ), i18n( "Professor" ), i18n( "Prof." ),
      ""
     };
    QString sSuffix[] = {
      i18n( "IIII" ), i18n( "II" ), i18n( "I" ), i18n( "Junior" ), 
      i18n( "Jr." ), i18n( "Senior" ), i18n( "Sr." ),
      ""
    };
    
    for (int i =0; sTitle[i] != ""; ++i )
      if (name.find( sTitle[i], 0, false ) == 0) {
	prefix = sTitle[i];
	name = name.right( name.length() - prefix.length() - 1 );
	name = name.simplifyWhiteSpace();
	break;
      }
    
    for (int i =0; sSuffix[i] != ""; ++i ) {
      QString tSuffix = sSuffix[i];
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
  ce->replace( "N",             curName );
  ce->replace( "X-Title",       prefix  );
  ce->replace( "X-FirstName",   first   );
  ce->replace( "X-MiddleName",  middle  );
  ce->replace( "X-LastName",    last    );
  ce->replace( "X-Suffix",      suffix  );

  updateFileAs();
}

AddressDialog::AddressDialog( QWidget *parent, 
			      QString entryField, 
            Entity *ce, 
			      bool modal )
 : QDialog( parent, "", modal ), entryField( entryField), ce( ce )
{

  setCaption( i18n("Address") );
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
  
  Field f = ce->field(entryField + "Street");
  if (!f.isNull()) mleStreet->setText(f.value());
  
  mleStreet->setMinimumSize( mleStreet->sizeHint() );
  
  lay->addWidget( new QLabel( i18n( "City" ), gb ), 2, 0 );
  leCity = new QLineEdit( gb );
  
  f = ce->field(entryField + "City");
  if (!f.isNull()) leCity->setText(f.value());
  
  lay->addWidget( leCity, 2, 1 );
  lay->addWidget( new QLabel( i18n( "State/Province" ), gb ), 3, 0 );
  leState = new QLineEdit( gb );
  
  f = ce->field(entryField + "State");
  if (!f.isNull()) leState->setText(f.value());
  
  lay->addWidget( leState, 3, 1 );
  lay->addWidget( new QLabel( i18n( "Zip/Postal Code" ), gb ), 4, 0 );
  lePostal = new QLineEdit( gb );
  
  f = ce->field(entryField + "PostalCode");
  if (!f.isNull()) lePostal->setText(f.value());

  lay->addWidget( lePostal, 4, 1 );

  lay->addWidget( new QLabel( i18n("Country"), gb ), 5, 0 );
  cbCountry = new QComboBox( true, gb );
  QString curCountry;
//  int cbNum = -1; Rikkus: unused ?

  f = ce->field(entryField + "Country");
  if (!f.isNull()) curCountry = QString(f.value());
 
  cbCountry->setAutoCompletion( true );
  lay->addWidget( cbCountry, 5, 1 );
  
  // Try to guess the country the user is in depending
  // on their preferred language.
  // Imperfect but the best I could do.
  // Rikkus: Hacked this around so we load from files instead - easier to
  // change. Also used a QMap for simplicity.
  
  QString language = KGlobal::locale()->language();
  
  QStringList countryList;
  
  QFile countryFile(locate("appdata", "countries"));
  QTextStream countryStream(&countryFile);

  if (countryFile.open(IO_ReadOnly))
    while (!countryStream.atEnd())
        countryList << countryStream.readLine();
  else
    debug("Couldn't read countries file");

  QMap<QString, QString> guessMap;

  QFile guessFile(locate("appdata", "guess"));
  QTextStream guessStream(&guessFile);
  
  if (guessFile.open(IO_ReadOnly)) {
    
    while (!guessStream.atEnd()) {
      
      QString s(guessStream.readLine()); 

      int i = s.find(' ');
      
      if (i == -1)
        continue;

      guessMap[s.left(i)] = s.mid(i).stripWhiteSpace();
    }
    
  } else
    debug("Couldn't open guess file");

  QString guessedCountry = guessMap[language];

  int i(0);
  int guessedIndex(0);
  
  QStringList::ConstIterator it(countryList.begin()); 
  
  for (; it != countryList.end(); ++it, i++) {
    cbCountry->insertItem(*it);
    if (guessedCountry == *it)
      guessedIndex = i;
  }

  cbCountry->setCurrentItem(guessedIndex);

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

  ce->replace( entryField, newAddress );
  ce->replace( entryField + "City", leCity->text());
  ce->replace( entryField + "Country", cbCountry->currentText());
  ce->replace( entryField + "PostalCode", lePostal->text());
  ce->replace( entryField + "State", leState->text());
  ce->replace( entryField + "Street", mleStreet->text());

  accept();
}

NameDialog::NameDialog( QWidget *parent, Entity *ce, bool modal )
  : QDialog( parent, "", modal ), ce( ce )
{
  QStringList sTitle;
  QStringList sSuffix;

  sTitle += i18n( "Dr." );
  sTitle += i18n( "Miss" );
  sTitle += i18n( "Mr." );
  sTitle += i18n( "Mrs." );
  sTitle += i18n( "Ms." );
  sTitle += i18n( "Prof." );
  sTitle.sort();

  sSuffix += i18n( "I" );
  sSuffix += i18n( "II" );
  sSuffix += i18n( "III" );
  sSuffix += i18n( "Jr." );
  sSuffix += i18n( "Sr." );
  sSuffix.sort();

  setCaption( "Full Name" );
  QGridLayout *hb = new QGridLayout( this, 1, 2, 10 );
  hb->setSpacing( 5 );
  
  QGroupBox *gb = new QGroupBox( this );
  gb->setTitle( i18n("Name details") );
  QGridLayout *lay = new QGridLayout( gb, 6, 2, 12 );
  lay->setSpacing( 5 );
  lay->addWidget( new QFrame( gb ), 0, 0 );
  lay->addWidget( new QFrame( gb ), 0, 1 );

  lay->addWidget( new QLabel( i18n("Title"), gb ),1,0);
  cbTitle = new QComboBox( true, gb );
  for ( unsigned int i = 0; i < sTitle.count(); ++i )
    cbTitle->insertItem( sTitle[i] );

  Field f = ce->field("X-Title");
  if (!f.isNull())
    cbTitle->setEditText(f.value());
  else
    cbTitle->setEditText( "" );
  lay->addWidget( cbTitle,1,1 );

  lay->addWidget( new QLabel( i18n("First"), gb ), 2,0);
  leFirst = new QLineEdit( gb );
  
  f = ce->field("X-FirstName");
  if (!f.isNull())
    leFirst->setText(f.value());
  else
    leFirst->setText( "" );
  lay->addWidget( leFirst, 2, 1 );
  leFirst->setMinimumSize( leFirst->sizeHint() );

  lay->addWidget( new QLabel( i18n("Middle"), gb ), 3, 0 );
  leMiddle = new QLineEdit( gb );
  
  f = ce->field("X-MiddleName");
  if (!f.isNull())
    leMiddle->setText(f.value());
  else
    leMiddle->setText( "" );
  lay->addWidget( leMiddle,3 ,1 );

  lay->addWidget( new QLabel( i18n("Last"), gb ), 4, 0 );
  leLast = new QLineEdit( gb );
  
  f = ce->field("X-Title");
  if (!f.isNull())
    leLast->setText(f.value());
  else
    leLast->setText( "" );
  lay->addWidget( leLast, 4, 1 );

  lay->addWidget( new QLabel( i18n("Suffix"), gb ), 5, 0 );
  cbSuffix = new QComboBox( true, gb );
  for ( unsigned int i = 0; i < sSuffix.count(); ++i )
    cbSuffix->insertItem( sSuffix[i] );

  f = ce->field("X-Suffix");
  if (!f.isNull())
    cbSuffix->setEditText(f.value());
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
  debug( "NameOk " + name );
  ce->replace( "N", name.simplifyWhiteSpace());
  ce->replace( "X-Title", cbTitle->currentText());
  ce->replace( "X-FirstName", leFirst->text() );
  ce->replace( "X-MiddleName", leMiddle->text() );
  ce->replace( "X-LastName", leLast->text() );
  ce->replace( "X-Suffix", cbSuffix->currentText() );
  accept();
}

void ContactDialog::setSheet(int sheet)
{
  QStringList names, fields;
  if (!Attributes::instance()->nameFieldList( sheet, &names, &fields )) {
//    fields = ce->custom(); FIXME
    fields.sort();
    for (int i = 0; i < (int)fields.count(); ++i )
      names += fields[i].mid( 9 );
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
