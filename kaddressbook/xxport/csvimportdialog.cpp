/*
   This file is part of KAddressBook.
   Copyright (C) 2003 Tobias Koenig <tokoe@kde.org>
                 based on the code of KSpread's CSV Import Dialog

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtable.h>
#include <qtextcodec.h>
#include <qtooltip.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kprogress.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>

#include "dateparser.h"

#include "csvimportdialog.h"

enum { Local = 0, Guess = 1, Latin1 = 2, Uni = 3, MSBug = 4, Codec = 5 };

CSVImportDialog::CSVImportDialog( KABC::AddressBook *ab, QWidget *parent,
                                  const char * name )
  : KDialogBase( Plain, i18n ( "CSV Import Dialog" ), Ok | Cancel | User1 |
                 User2, Ok, parent, name, true, true ),
    mAdjustRows( false ),
    mStartLine( 0 ),
    mTextQuote( '"' ),
    mDelimiter( "," ),
    mAddressBook( ab )
{
  initGUI();

  mTypeMap.insert( i18n( "Undefined" ), Undefined );
  mTypeMap.insert( KABC::Addressee::formattedNameLabel(), FormattedName );
  mTypeMap.insert( KABC::Addressee::familyNameLabel(), FamilyName );
  mTypeMap.insert( KABC::Addressee::givenNameLabel(), GivenName );
  mTypeMap.insert( KABC::Addressee::additionalNameLabel(), AdditionalName );
  mTypeMap.insert( KABC::Addressee::prefixLabel(), Prefix );
  mTypeMap.insert( KABC::Addressee::suffixLabel(), Suffix );
  mTypeMap.insert( KABC::Addressee::nickNameLabel(), NickName );
  mTypeMap.insert( KABC::Addressee::birthdayLabel(), Birthday );

  mTypeMap.insert( KABC::Addressee::homeAddressStreetLabel(), HomeAddressStreet );
  mTypeMap.insert( KABC::Addressee::homeAddressLocalityLabel(),
                   HomeAddressLocality );
  mTypeMap.insert( KABC::Addressee::homeAddressRegionLabel(), HomeAddressRegion );
  mTypeMap.insert( KABC::Addressee::homeAddressPostalCodeLabel(),
                   HomeAddressPostalCode );
  mTypeMap.insert( KABC::Addressee::homeAddressCountryLabel(),
                   HomeAddressCountry );
  mTypeMap.insert( KABC::Addressee::homeAddressLabelLabel(), HomeAddressLabel );

  mTypeMap.insert( KABC::Addressee::businessAddressStreetLabel(),
                   BusinessAddressStreet );
  mTypeMap.insert( KABC::Addressee::businessAddressLocalityLabel(),
                   BusinessAddressLocality );
  mTypeMap.insert( KABC::Addressee::businessAddressRegionLabel(),
                   BusinessAddressRegion );
  mTypeMap.insert( KABC::Addressee::businessAddressPostalCodeLabel(),
                   BusinessAddressPostalCode );
  mTypeMap.insert( KABC::Addressee::businessAddressCountryLabel(),
                   BusinessAddressCountry );
  mTypeMap.insert( KABC::Addressee::businessAddressLabelLabel(),
                   BusinessAddressLabel );

  mTypeMap.insert( KABC::Addressee::homePhoneLabel(), HomePhone );
  mTypeMap.insert( KABC::Addressee::businessPhoneLabel(), BusinessPhone );
  mTypeMap.insert( KABC::Addressee::mobilePhoneLabel(), MobilePhone );
  mTypeMap.insert( KABC::Addressee::homeFaxLabel(), HomeFax );
  mTypeMap.insert( KABC::Addressee::businessFaxLabel(), BusinessFax );
  mTypeMap.insert( KABC::Addressee::carPhoneLabel(), CarPhone );
  mTypeMap.insert( KABC::Addressee::isdnLabel(), Isdn );
  mTypeMap.insert( KABC::Addressee::pagerLabel(), Pager );
  mTypeMap.insert( KABC::Addressee::emailLabel(), Email );
  mTypeMap.insert( KABC::Addressee::mailerLabel(), Mailer );
  mTypeMap.insert( KABC::Addressee::titleLabel(), Title );
  mTypeMap.insert( KABC::Addressee::roleLabel(), Role );
  mTypeMap.insert( KABC::Addressee::organizationLabel(), Organization );
  mTypeMap.insert( KABC::Addressee::noteLabel(), Note );
  mTypeMap.insert( KABC::Addressee::urlLabel(), URL );

  mCustomCounter = mTypeMap.count();
  int count = mCustomCounter;

  KABC::Field::List fields = mAddressBook->fields( KABC::Field::CustomCategory );
  KABC::Field::List::Iterator it;
  for ( it = fields.begin(); it != fields.end(); ++it, ++count )
    mTypeMap.insert( (*it)->label(), count );

  reloadCodecs();

  connect( mDelimiterBox, SIGNAL( clicked( int ) ),
           this, SLOT( delimiterClicked( int ) ) );
  connect( mDelimiterEdit, SIGNAL( returnPressed() ),
           this, SLOT( returnPressed() ) );
  connect( mDelimiterEdit, SIGNAL( textChanged ( const QString& ) ),
           this, SLOT( textChanged ( const QString& ) ) );
  connect( mComboLine, SIGNAL( activated( const QString& ) ),
           this, SLOT( lineSelected( const QString& ) ) );
  connect( mComboQuote, SIGNAL( activated( const QString& ) ),
           this, SLOT( textquoteSelected( const QString& ) ) );
  connect( mIgnoreDuplicates, SIGNAL( stateChanged( int ) ),
           this, SLOT( ignoreDuplicatesChanged( int ) ) );
  connect( mCodecCombo, SIGNAL( activated( const QString& ) ),
           this, SLOT( codecChanged() ) );

  connect( mUrlRequester, SIGNAL( returnPressed( const QString& ) ),
           this, SLOT( setFile( const QString& ) ) );
  connect( mUrlRequester, SIGNAL( urlSelected( const QString& ) ),
           this, SLOT( setFile( const QString& ) ) );
  connect( mUrlRequester->lineEdit(), SIGNAL( textChanged ( const QString& ) ),
           this, SLOT( urlChanged( const QString& ) ) );

  connect( this, SIGNAL( user1Clicked() ),
           this, SLOT( applyTemplate() ) );

  connect( this, SIGNAL( user2Clicked() ),
           this, SLOT( saveTemplate() ) );
}

CSVImportDialog::~CSVImportDialog()
{
  mCodecs.clear();
}

KABC::AddresseeList CSVImportDialog::contacts() const
{
  DateParser dateParser( mDatePatternEdit->text() );
  KABC::AddresseeList contacts;

  KProgressDialog progressDialog( mPage );
  progressDialog.setAutoClose( true );
  progressDialog.progressBar()->setTotalSteps( mTable->numRows() );
  progressDialog.setLabel( i18n( "Importing contacts" ) );
  progressDialog.show();

  kapp->processEvents();

  for ( int row = 1; row < mTable->numRows(); ++row ) {
    KABC::Addressee a;
    bool emptyRow = true;
    KABC::Address addrHome( KABC::Address::Home );
    KABC::Address addrWork( KABC::Address::Work );
    for ( int col = 0; col < mTable->numCols(); ++col ) {
      QComboTableItem *item = static_cast<QComboTableItem*>( mTable->item( 0,
                                                             col ) );
      if ( !item ) {
        kdError() << "ERROR: item cast failed" << endl;
        continue;
      }

      QString value = mTable->text( row, col );
      if ( !value.isEmpty() )
        emptyRow = false;

      switch ( posToType( item->currentItem() ) ) {
        case Undefined:
          continue;
          break;
        case FormattedName:
          a.setFormattedName( value );
          break;
        case GivenName:
          a.setGivenName( value );
          break;
        case FamilyName:
          a.setFamilyName( value );
          break;
        case AdditionalName:
          a.setAdditionalName( value );
          break;
        case Prefix:
          a.setPrefix( value );
          break;
        case Suffix:
          a.setSuffix( value );
          break;
        case NickName:
          a.setNickName( value );
          break;
        case Birthday:
          a.setBirthday( dateParser.parse( value ) );
          break;
        case Email:
          if ( !value.isEmpty() )
            a.insertEmail( value, true );
          break;
        case Role:
          a.setRole( value );
          break;
        case Title:
          a.setTitle( value );
          break;
        case Mailer:
          a.setMailer( value );
          break;
        case URL:
          a.setUrl( KURL( value ) );
          break;
        case Organization:
          a.setOrganization( value );
          break;
        case Note:
          a.setNote( value );
          break;

        case HomePhone:
          if ( !value.isEmpty() ) {
            KABC::PhoneNumber number( value, KABC::PhoneNumber::Home );
            a.insertPhoneNumber( number );
          }
          break;
        case BusinessPhone:
          if ( !value.isEmpty() ) {
            KABC::PhoneNumber number( value, KABC::PhoneNumber::Work );
            a.insertPhoneNumber( number );
          }
          break;
        case MobilePhone:
          if ( !value.isEmpty() ) {
            KABC::PhoneNumber number( value, KABC::PhoneNumber::Cell );
            a.insertPhoneNumber( number );
          }
          break;
        case HomeFax:
          if ( !value.isEmpty() ) {
            KABC::PhoneNumber number( value, KABC::PhoneNumber::Home |
                                             KABC::PhoneNumber::Fax );
            a.insertPhoneNumber( number );
          }
          break;
        case BusinessFax:
          if ( !value.isEmpty() ) {
            KABC::PhoneNumber number( value, KABC::PhoneNumber::Work |
                                             KABC::PhoneNumber::Fax );
            a.insertPhoneNumber( number );
          }
          break;
        case CarPhone:
          if ( !value.isEmpty() ) {
            KABC::PhoneNumber number( value, KABC::PhoneNumber::Car );
            a.insertPhoneNumber( number );
          }
          break;
        case Isdn:
          if ( !value.isEmpty() ) {
            KABC::PhoneNumber number( value, KABC::PhoneNumber::Isdn );
            a.insertPhoneNumber( number );
          }
          break;
        case Pager:
          if ( !value.isEmpty() ) {
            KABC::PhoneNumber number( value, KABC::PhoneNumber::Pager );
            a.insertPhoneNumber( number );
          }
          break;

        case HomeAddressStreet:
          addrHome.setStreet( value );
          break;
        case HomeAddressLocality:
          addrHome.setLocality( value );
          break;
        case HomeAddressRegion:
          addrHome.setRegion( value );
          break;
        case HomeAddressPostalCode:
          addrHome.setPostalCode( value );
          break;
        case HomeAddressCountry:
          addrHome.setCountry( value );
          break;
        case HomeAddressLabel:
          addrHome.setLabel( value );
          break;

        case BusinessAddressStreet:
          addrWork.setStreet( value );
          break;
        case BusinessAddressLocality:
          addrWork.setLocality( value );
          break;
        case BusinessAddressRegion:
          addrWork.setRegion( value );
          break;
        case BusinessAddressPostalCode:
          addrWork.setPostalCode( value );
          break;
        case BusinessAddressCountry:
          addrWork.setCountry( value );
          break;
        case BusinessAddressLabel:
          addrWork.setLabel( value );
          break;
        default:
          KABC::Field::List fields = mAddressBook->fields( KABC::Field::CustomCategory );
          KABC::Field::List::Iterator it;

          int counter = 0;
          for ( it = fields.begin(); it != fields.end(); ++it ) {
            if ( counter == (int)( posToType( item->currentItem() ) - mCustomCounter ) ) {
              (*it)->setValue( a, value );
              continue;
            }
            ++counter;
          }
          break;
      }
    }

    kapp->processEvents();

    if ( progressDialog.wasCancelled() )
      return KABC::AddresseeList();

    progressDialog.progressBar()->advance( 1 );

    if ( !addrHome.isEmpty() )
      a.insertAddress( addrHome );
    if ( !addrWork.isEmpty() )
      a.insertAddress( addrWork );

    if ( !emptyRow && !a.isEmpty() )
      contacts.append( a );
  }

  return contacts;
}

void CSVImportDialog::initGUI()
{
  mPage = plainPage();

  QGridLayout *layout = new QGridLayout( mPage, 1, 1, marginHint(),
                                         spacingHint() );
  QHBoxLayout *hbox = new QHBoxLayout();
  hbox->setSpacing( spacingHint() );

  QLabel *label = new QLabel( i18n( "File to import:" ), mPage );
  hbox->addWidget( label );

  mUrlRequester = new KURLRequester( mPage );
  mUrlRequester->setFilter( "*.csv" );
  hbox->addWidget( mUrlRequester );

  layout->addMultiCellLayout( hbox, 0, 0, 0, 4 );

  // Delimiter: comma, semicolon, tab, space, other
  mDelimiterBox = new QButtonGroup( i18n( "Delimiter" ), mPage );
  mDelimiterBox->setColumnLayout( 0, Qt::Vertical );
  mDelimiterBox->layout()->setSpacing( spacingHint() );
  mDelimiterBox->layout()->setMargin( marginHint() );
  QGridLayout *delimiterLayout = new QGridLayout( mDelimiterBox->layout() );
  delimiterLayout->setAlignment( Qt::AlignTop );
  layout->addMultiCellWidget( mDelimiterBox, 1, 4, 0, 0 );

  mRadioComma = new QRadioButton( i18n( "Comma" ), mDelimiterBox );
  mRadioComma->setChecked( true );
  delimiterLayout->addWidget( mRadioComma, 0, 0 );

  mRadioSemicolon = new QRadioButton( i18n( "Semicolon" ), mDelimiterBox );
  delimiterLayout->addWidget( mRadioSemicolon, 0, 1 );

  mRadioTab = new QRadioButton( i18n( "Tabulator" ), mDelimiterBox );
  delimiterLayout->addWidget( mRadioTab, 1, 0 );

  mRadioSpace = new QRadioButton( i18n( "Space" ), mDelimiterBox );
  delimiterLayout->addWidget( mRadioSpace, 1, 1 );

  mRadioOther = new QRadioButton( i18n( "Other" ), mDelimiterBox );
  delimiterLayout->addWidget( mRadioOther, 0, 2 );

  mDelimiterEdit = new QLineEdit( mDelimiterBox );
  delimiterLayout->addWidget( mDelimiterEdit, 1, 2 );

  mComboLine = new QComboBox( false, mPage );
  mComboLine->insertItem( i18n( "1" ) );
  layout->addWidget( mComboLine, 2, 3 );

  mComboQuote = new QComboBox( false, mPage );
  mComboQuote->insertItem( i18n( "\"" ), 0 );
  mComboQuote->insertItem( i18n( "'" ), 1 );
  mComboQuote->insertItem( i18n( "None" ), 2 );
  layout->addWidget( mComboQuote, 2, 2 );

  mDatePatternEdit = new QLineEdit( mPage );
  mDatePatternEdit->setText( "Y-M-D" ); // ISO 8601 format as default
  QToolTip::add( mDatePatternEdit, i18n( "<ul><li>y: year with 2 digits</li>"
                                         "<li>Y: year with 4 digits</li>"
                                         "<li>m: month with 1 or 2 digits</li>"
                                         "<li>M: month with 2 digits</li>"
                                         "<li>d: day with 1 or 2 digits</li>"
                                         "<li>D: day with 2 digits</li></ul>" ) );
  layout->addWidget( mDatePatternEdit, 2, 4 );

  label = new QLabel( i18n( "Start at line:" ), mPage );
  layout->addWidget( label, 1, 3 );

  label = new QLabel( i18n( "Textquote:" ), mPage );
  layout->addWidget( label, 1, 2 );

  label = new QLabel( i18n( "Date format:" ), mPage );
  layout->addWidget( label, 1, 4 );

  mIgnoreDuplicates = new QCheckBox( mPage );
  mIgnoreDuplicates->setText( i18n( "Ignore duplicate delimiters" ) );
  layout->addMultiCellWidget( mIgnoreDuplicates, 3, 3, 2, 4 );

  mCodecCombo = new QComboBox( mPage );
  layout->addMultiCellWidget( mCodecCombo, 4, 4, 2, 4 );

  mTable = new QTable( 0, 0, mPage );
  mTable->setSelectionMode( QTable::NoSelection );
  mTable->horizontalHeader()->hide();
  layout->addMultiCellWidget( mTable, 5, 5, 0, 4 );

  setButtonText( User1, i18n( "Apply Template..." ) );
  setButtonText( User2, i18n( "Save Template..." ) );

  enableButtonOK( false );
  actionButton( User1 )->setEnabled( false );
  actionButton( User2 )->setEnabled( false );

  resize( 400, 300 );
}

void CSVImportDialog::fillTable()
{
  int row, column;
  bool lastCharDelimiter = false;
  bool ignoreDups = mIgnoreDuplicates->isChecked();
  enum { S_START, S_QUOTED_FIELD, S_MAYBE_END_OF_QUOTED_FIELD, S_END_OF_QUOTED_FIELD,
         S_MAYBE_NORMAL_FIELD, S_NORMAL_FIELD } state = S_START;

  QChar x;
  QString field;

  // store previous assignment
  mTypeStore.clear();
  for ( column = 0; column < mTable->numCols(); ++column ) {
    QComboTableItem *item = static_cast<QComboTableItem*>( mTable->item( 0,
                                                           column ) );
    if ( !item || mClearTypeStore )
      mTypeStore.append( typeToPos( Undefined ) );
    else if ( item )
      mTypeStore.append( item->currentItem() );
  }

  clearTable();

  row = column = 1;
  mData = QString( mFileArray );

  QTextStream inputStream( mData, IO_ReadOnly );

  // find the current codec
  int code = mCodecCombo->currentItem();
  if ( code >= Codec )
    inputStream.setCodec( mCodecs.at( code - Codec ) );
  else if ( code == Uni )
    inputStream.setEncoding( QTextStream::Unicode );
  else if ( code == MSBug )
    inputStream.setEncoding( QTextStream::UnicodeReverse );
  else if ( code == Latin1 )
    inputStream.setEncoding( QTextStream::Latin1 );
  else if ( code == Guess ) {
    QTextCodec* codec = QTextCodec::codecForContent( mFileArray.data(), mFileArray.size() );
    if ( codec ) {
      KMessageBox::information( this, i18n( "Using codec '%1'" ).arg( codec->name() ), i18n( "Encoding" ) );
      inputStream.setCodec( codec );
    }
  }

  int maxColumn = 0;
  while ( !inputStream.atEnd() ) {
    inputStream >> x; // read one char

    if ( x == '\r' ) inputStream >> x; // eat '\r', to handle DOS/LOSEDOWS files correctly

    switch ( state ) {
     case S_START :
      if ( x == mTextQuote ) {
        state = S_QUOTED_FIELD;
      } else if ( x == mDelimiter ) {
        if ( ( ignoreDups == false ) || ( lastCharDelimiter == false ) )
          ++column;
        lastCharDelimiter = true;
      } else if ( x == '\n' ) {
        ++row;
        column = 1;
      } else {
        field += x;
        state = S_MAYBE_NORMAL_FIELD;
      }
      break;
     case S_QUOTED_FIELD :
      if ( x == mTextQuote ) {
        state = S_MAYBE_END_OF_QUOTED_FIELD;
      } else if ( x == '\n' ) {
        setText( row - mStartLine + 1, column, field );
        field = "";
        if ( x == '\n' ) {
          ++row;
          column = 1;
        } else {
          if ( ( ignoreDups == false ) || ( lastCharDelimiter == false ) )
            ++column;
          lastCharDelimiter = true;
        }
        state = S_START;
      } else {
        field += x;
      }
      break;
     case S_MAYBE_END_OF_QUOTED_FIELD :
      if ( x == mTextQuote ) {
        field += x;
        state = S_QUOTED_FIELD;
      } else if ( x == mDelimiter || x == '\n' ) {
        setText( row - mStartLine + 1, column, field );
        field = "";
        if ( x == '\n' ) {
          ++row;
          column = 1;
        } else {
          if ( ( ignoreDups == false ) || ( lastCharDelimiter == false ) )
            ++column;
          lastCharDelimiter = true;
        }
        state = S_START;
      } else {
        state = S_END_OF_QUOTED_FIELD;
      }
      break;
     case S_END_OF_QUOTED_FIELD :
      if ( x == mDelimiter || x == '\n' ) {
        setText( row - mStartLine + 1, column, field );
        field = "";
        if ( x == '\n' ) {
          ++row;
          column = 1;
        } else {
          if ( ( ignoreDups == false ) || ( lastCharDelimiter == false ) )
            ++column;
          lastCharDelimiter = true;
        }
        state = S_START;
      } else {
        state = S_END_OF_QUOTED_FIELD;
      }
      break;
     case S_MAYBE_NORMAL_FIELD :
      if ( x == mTextQuote ) {
        field = "";
        state = S_QUOTED_FIELD;
        break;
      }
     case S_NORMAL_FIELD :
      if ( x == mDelimiter || x == '\n' ) {
        setText( row - mStartLine + 1, column, field );
        field = "";
        if ( x == '\n' ) {
          ++row;
          column = 1;
        } else {
          if ( ( ignoreDups == false ) || ( lastCharDelimiter == false ) )
            ++column;
          lastCharDelimiter = true;
        }
        state = S_START;
      } else {
        field += x;
      }
    }
    if ( x != mDelimiter )
      lastCharDelimiter = false;

    if ( column > maxColumn )
      maxColumn = column;
  }

  // file with only one line without '\n'
  if ( field.length() > 0 ) {
    setText( row - mStartLine + 1, column, field );
    ++row;
    field = "";
  }

  adjustRows( row - mStartLine );
  mTable->setNumCols( maxColumn );

  for ( column = 0; column < mTable->numCols(); ++column ) {
    QComboTableItem *item = new QComboTableItem( mTable, mTypeMap.keys() );
    mTable->setItem( 0, column, item );
    if ( column < (int)mTypeStore.count() )
      item->setCurrentItem( mTypeStore[ column ] );
    else
      item->setCurrentItem( typeToPos( Undefined ) );
    mTable->adjustColumn( column );
  }

  resizeColumns();
}

void CSVImportDialog::clearTable()
{
  for ( int row = 0; row < mTable->numRows(); ++row )
    for ( int column = 0; column < mTable->numCols(); ++column )
      mTable->clearCell( row, column );
}

void CSVImportDialog::fillComboBox()
{
  mComboLine->clear();
  for ( int row = 1; row < mTable->numRows() + 1; ++row )
    mComboLine->insertItem( QString::number( row ), row - 1 );
}

void CSVImportDialog::reloadCodecs()
{
  mCodecCombo->clear();

  mCodecs.clear();

  QTextCodec *codec;
  for ( int i = 0; ( codec = QTextCodec::codecForIndex( i ) ); i++ )
    mCodecs.append( codec );

  mCodecCombo->insertItem( i18n( "Local (%1)" ).arg( QTextCodec::codecForLocale()->name() ), Local );
  mCodecCombo->insertItem( i18n( "[guess]" ), Guess );
  mCodecCombo->insertItem( i18n( "Latin1" ), Latin1 );
  mCodecCombo->insertItem( i18n( "Unicode" ), Uni );
  mCodecCombo->insertItem( i18n( "Microsoft Unicode" ), MSBug );

	for ( uint i = 0; i < mCodecs.count(); i++ )
    mCodecCombo->insertItem( mCodecs.at( i )->name(), Codec + i );
}

void CSVImportDialog::setText( int row, int col, const QString& text )
{
  if ( row < 1 ) // skipped by the user
    return;

  if ( mTable->numRows() < row ) {
    mTable->setNumRows( row + 5000 ); // We add 5000 at a time to limit recalculations
    mAdjustRows = true;
  }

  if ( mTable->numCols() < col )
    mTable->setNumCols( col + 50 ); // We add 50 at a time to limit recalculation

  mTable->setText( row - 1, col - 1, text );
}

/*
 * Called after the first fillTable() when number of rows are unknown.
 */
void CSVImportDialog::adjustRows( int rows )
{
  if ( mAdjustRows ) {
    mTable->setNumRows( rows );
    mAdjustRows = false;
  }
}

void CSVImportDialog::resizeColumns()
{
  QFontMetrics fm = fontMetrics();
  int width = 0;

  QMap<QString, uint>::ConstIterator it;
  for ( it = mTypeMap.begin(); it != mTypeMap.end(); ++it ) {
    width = QMAX( width, fm.width( it.key() ) );
  }

  for ( int i = 0; i < mTable->numCols(); ++i )
    mTable->setColumnWidth( i, QMAX( width + 15, mTable->columnWidth( i ) ) );
}

void CSVImportDialog::returnPressed()
{
  if ( mDelimiterBox->id( mDelimiterBox->selected() ) != 4 )
    return;

  mDelimiter = mDelimiterEdit->text();
  fillTable();
}

void CSVImportDialog::textChanged ( const QString& )
{
  mRadioOther->setChecked ( true );
  delimiterClicked( 4 ); // other
}

void CSVImportDialog::delimiterClicked( int id )
{
  switch ( id ) {
   case 0: // comma
    mDelimiter = ",";
    break;
   case 4: // other
    mDelimiter = mDelimiterEdit->text();
    break;
   case 2: // tab
    mDelimiter = "\t";
    break;
   case 3: // space
    mDelimiter = " ";
    break;
   case 1: // semicolon
    mDelimiter = ";";
    break;
  }

  fillTable();
}

void CSVImportDialog::textquoteSelected( const QString& mark )
{
  if ( mComboQuote->currentItem() == 2 )
    mTextQuote = 0;
  else
    mTextQuote = mark[ 0 ];

  fillTable();
}

void CSVImportDialog::lineSelected( const QString& line )
{
  mStartLine = line.toInt() - 1;
  fillTable();
}

void CSVImportDialog::slotOk()
{
  bool assigned = false;

  for ( int column = 0; column < mTable->numCols(); ++column ) {
    QComboTableItem *item = static_cast<QComboTableItem*>( mTable->item( 0,
                                                           column ) );
    if ( item && posToType( item->currentItem() ) != Undefined )
      assigned = true;
  }

  if ( assigned )
    KDialogBase::slotOk();
  else
    KMessageBox::sorry( this, i18n( "You have to assign at least one column." ) );
}

void CSVImportDialog::applyTemplate()
{
  QMap<uint,int> columnMap;
  QMap<QString, QString> fileMap;
  QStringList templates;

  // load all template files
  QStringList list = KGlobal::dirs()->findAllResources( "data" , QString( kapp->name() ) +
      "/csv-templates/*.desktop", true, true );

  for ( QStringList::iterator it = list.begin(); it != list.end(); ++it )
  {
    KSimpleConfig config( *it, true );

    if ( !config.hasGroup( "csv column map" ) )
	    continue;

    config.setGroup( "Misc" );
    templates.append( config.readEntry( "Name" ) );
    fileMap.insert( config.readEntry( "Name" ), *it );
  }

  // let the user chose, what to take
  bool ok = false;
  QString tmp;
  tmp = KInputDialog::getItem( i18n( "Template Selection" ),
                  i18n( "Please select a template, that matches the CSV file:" ),
                  templates, 0, false, &ok, this );

  if ( !ok )
    return;

  KSimpleConfig config( fileMap[ tmp ], true );
  config.setGroup( "General" );
  mDatePatternEdit->setText( config.readEntry( "DatePattern", "Y-M-D" ) );
  uint numColumns = config.readUnsignedNumEntry( "Columns" );
  mDelimiterEdit->setText( config.readEntry( "DelimiterOther" ) );
  mDelimiterBox->setButton( config.readNumEntry( "DelimiterType" ) );
  delimiterClicked( config.readNumEntry( "DelimiterType" ) );
  int quoteType = config.readNumEntry( "QuoteType" );
  mComboQuote->setCurrentItem( quoteType );
  textquoteSelected( mComboQuote->currentText() );

  // create the column map
  config.setGroup( "csv column map" );
  for ( uint i = 0; i < numColumns; ++i ) {
    int col = config.readNumEntry( QString::number( i ) );
    columnMap.insert( i, col );
  }

  // apply the column map
  for ( uint column = 0; column < columnMap.count(); ++column ) {
    int type = columnMap[ column ];
    QComboTableItem *item = static_cast<QComboTableItem*>( mTable->item( 0,
                                                           column ) );
    if ( item )
      item->setCurrentItem( typeToPos( type ) );
  }
}

void CSVImportDialog::saveTemplate()
{
  QString fileName = KFileDialog::getSaveFileName(
                     locateLocal( "data", QString( kapp->name() ) + "/csv-templates/" ),
                     "*.desktop", this );

  if ( fileName.isEmpty() )
    return;

  if ( !fileName.contains( ".desktop" ) )
    fileName += ".desktop";

  QString name = KInputDialog::getText( i18n( "Template Name" ), i18n( "Please enter a name for the template:" ) );

  if ( name.isEmpty() )
    return;

  KConfig config( fileName );
  config.setGroup( "General" );
  config.writeEntry( "DatePattern", mDatePatternEdit->text() );
  config.writeEntry( "Columns", mTable->numCols() );
  config.writeEntry( "DelimiterType", mDelimiterBox->id( mDelimiterBox->selected() ) );
  config.writeEntry( "DelimiterOther", mDelimiterEdit->text() );
  config.writeEntry( "QuoteType", mComboQuote->currentItem() );

  config.setGroup( "Misc" );
  config.writeEntry( "Name", name );

  config.setGroup( "csv column map" );

  for ( int column = 0; column < mTable->numCols(); ++column ) {
    QComboTableItem *item = static_cast<QComboTableItem*>( mTable->item( 0,
                                                           column ) );
    if ( item )
      config.writeEntry( QString::number( column ), posToType(
                         item->currentItem() ) );
    else
      config.writeEntry( QString::number( column ), 0 );
  }

  config.sync();
}

QString CSVImportDialog::getText( int row, int col )
{
  return mTable->text( row, col );
}

uint CSVImportDialog::posToType( int pos ) const
{
  uint counter = 0;
  QMap<QString, uint>::ConstIterator it;
  for ( it = mTypeMap.begin(); it != mTypeMap.end(); ++it, ++counter )
    if ( counter == (uint)pos )
      return it.data();

  return 0;
}

int CSVImportDialog::typeToPos( uint type ) const
{
  uint counter = 0;
  QMap<QString, uint>::ConstIterator it;
  for ( it = mTypeMap.begin(); it != mTypeMap.end(); ++it, ++counter )
    if ( it.data() == type )
      return counter;

  return -1;
}

void CSVImportDialog::ignoreDuplicatesChanged( int )
{
  fillTable();
}

void CSVImportDialog::setFile( const QString &fileName )
{
  if ( fileName.isEmpty() )
    return;

  QFile file( fileName );
  if ( !file.open( IO_ReadOnly ) ) {
    KMessageBox::sorry( this, i18n( "Cannot open input file." ) );
    file.close();
    return;
  }

  mFileArray = file.readAll();
  file.close();

  mClearTypeStore = true;
  clearTable();
  mTable->setNumCols( 0 );
  mTable->setNumRows( 0 );
  fillTable();
  mClearTypeStore = false;

  fillComboBox();
}

void CSVImportDialog::urlChanged( const QString &file )
{
  bool state = !file.isEmpty();

  enableButtonOK( state );
  actionButton( User1 )->setEnabled( state );
  actionButton( User2 )->setEnabled( state );
}

void CSVImportDialog::codecChanged()
{
  fillTable();
}

#include <csvimportdialog.moc>
