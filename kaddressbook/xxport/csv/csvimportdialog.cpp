/*
    This file is part of KAddressBook.
    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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
*/

#include "csvimportdialog.h"

#include "dateparser.h"
#include "qcsvmodel.h"
#include "templateselectiondialog.h"

#include <QtCore/QPointer>
#include <QtCore/QTextCodec>
#include <QtCore/QThread>
#include <QtCore/QUuid>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QStyledItemDelegate>
#include <QtGui/QTableView>

#include <kapplication.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprogressdialog.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>

enum { Local = 0, Latin1 = 1, Uni = 2, MSBug = 3, Codec = 4 };

class ContactFieldComboBox : public KComboBox
{
  public:

    ContactFieldComboBox( QWidget *parent = 0 )
      : KComboBox( parent )
    {
      fillFieldMap();

      addItem( ContactFields::label( ContactFields::Undefined ), ContactFields::Undefined );

      QMapIterator<QString, ContactFields::Field> it( mFieldMap );
      while ( it.hasNext() ) {
        it.next();

        addItem( it.key(), QVariant( it.value() ) );
      }

      int maxLength = 0;
      for ( int i = 0; i < count(); ++i ) {
        maxLength = qMax( maxLength, itemText( i ).length() );
      }

      setMinimumContentsLength( maxLength );
      setSizeAdjustPolicy( AdjustToMinimumContentsLength );
      setFixedSize( sizeHint() );
    }

    void setCurrentField( ContactFields::Field field )
    {
      setCurrentIndex( findData( (uint)field ) );
    }

    ContactFields::Field currentField() const
    {
      return (ContactFields::Field)itemData( currentIndex() ).toUInt();
    }

  private:
    static void fillFieldMap()
    {
      if ( !mFieldMap.isEmpty() )
        return;

      ContactFields::Fields fields = ContactFields::allFields();
      fields.remove( ContactFields::Undefined );

      for ( int i = 0; i < fields.count(); ++i )
        mFieldMap.insert( ContactFields::label( fields.at( i ) ), fields.at( i ) );
    }

    static QMap<QString, ContactFields::Field> mFieldMap;
};

QMap<QString, ContactFields::Field> ContactFieldComboBox::mFieldMap;

class ContactFieldDelegate : public QStyledItemDelegate
{
  public:
    ContactFieldDelegate( QObject *parent = 0 )
      : QStyledItemDelegate( parent )
    {
    }

    QString displayText( const QVariant &value, const QLocale& ) const
    {
      return ContactFields::label( (ContactFields::Field)value.toUInt() );
    }

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem&, const QModelIndex& ) const
    {
      ContactFieldComboBox *editor = new ContactFieldComboBox( parent );

      return editor;
    }

    void setEditorData( QWidget *editor, const QModelIndex &index ) const
    {
      const unsigned int value = index.model()->data( index, Qt::EditRole ).toUInt();

      ContactFieldComboBox *fieldCombo = static_cast<ContactFieldComboBox*>( editor );
      fieldCombo->setCurrentField( (ContactFields::Field)value );
    }

    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
    {
      ContactFieldComboBox *fieldCombo = static_cast<ContactFieldComboBox*>( editor );

      model->setData( index, fieldCombo->currentField(), Qt::EditRole );
    }

    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& ) const
    {
      editor->setGeometry( option.rect );
    }

    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
    {
      if ( index.row() == 0 ) {
        QStyleOptionViewItem headerOption( option );
        headerOption.font.setBold( true );

        QStyledItemDelegate::paint( painter, headerOption, index );
      } else {
        QStyledItemDelegate::paint( painter, option, index );
      }
    }
};


CSVImportDialog::CSVImportDialog( QWidget *parent )
  : KDialog( parent ),
    mDevice( 0 )
{
  setCaption( i18nc( "@title:window", "CSV Import Dialog" ) );
  setButtons( Ok | Cancel | User1 | User2 );
  setDefaultButton( Ok );
  setModal( true );
  showButtonSeparator( true );

  mModel = new QCsvModel( this );

  initGUI();

  reloadCodecs();

  connect( mUrlRequester, SIGNAL( returnPressed( const QString& ) ),
           this, SLOT( setFile( const QString& ) ) );
  connect( mUrlRequester, SIGNAL( urlSelected( const KUrl& ) ),
           this, SLOT( setFile( const KUrl& ) ) );
  connect( mUrlRequester->lineEdit(), SIGNAL( textChanged ( const QString& ) ),
           this, SLOT( urlChanged( const QString& ) ) );
  connect( mDelimiterGroup, SIGNAL( buttonClicked( int ) ),
           this, SLOT( delimiterClicked( int ) ) );
  connect( mDelimiterEdit, SIGNAL( returnPressed() ),
           this, SLOT( customDelimiterChanged() ) );
  connect( mDelimiterEdit, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( customDelimiterChanged( const QString& ) ) );
  connect( mComboQuote, SIGNAL( activated( const QString& ) ),
           this, SLOT( textQuoteChanged( const QString& ) ) );
  connect( mCodecCombo, SIGNAL( activated( const QString& ) ),
           this, SLOT( codecChanged() ) );
  connect( mSkipFirstRow, SIGNAL( toggled( bool ) ),
           this, SLOT( skipFirstRowChanged( bool ) ) );

  connect( mModel, SIGNAL( finishedLoading() ), this, SLOT( modelFinishedLoading() ) );

  delimiterClicked( 0 );
  textQuoteChanged( "\"" );
  skipFirstRowChanged( false );
}

CSVImportDialog::~CSVImportDialog()
{
}

KABC::AddresseeList CSVImportDialog::contacts() const
{
  KABC::AddresseeList contacts;
  DateParser dateParser( mDatePatternEdit->text() );

  KProgressDialog progressDialog( const_cast<CSVImportDialog*>( this )->mainWidget() );
  progressDialog.setAutoClose( true );
  progressDialog.progressBar()->setMaximum( mModel->rowCount() );
  progressDialog.setLabelText( i18nc( "@label", "Importing contacts" ) );
  progressDialog.show();

  kapp->processEvents();

  for ( int row = 1; row < mModel->rowCount(); ++row ) {
    KABC::Addressee contact;
    bool emptyRow = true;

    for ( int column = 0; column < mModel->columnCount(); ++column ) {
      QString value = mModel->data( mModel->index( row, column ), Qt::DisplayRole ).toString();

      if ( !value.isEmpty() ) {
        emptyRow = false;

        const ContactFields::Field field = (ContactFields::Field)mModel->data( mModel->index( 0, column ) ).toUInt();

        // convert the custom date format to ISO format
        if ( field == ContactFields::Birthday || field == ContactFields::Anniversary )
          value = dateParser.parse( value ).toString( Qt::ISODate );

        value.replace( "\\n", "\n" );

        ContactFields::setValue( field, value, contact );
      }
    }

    kapp->processEvents();

    if ( progressDialog.wasCancelled() )
      return KABC::AddresseeList();

    progressDialog.progressBar()->setValue( progressDialog.progressBar()->value() + 1 );

    if ( !emptyRow && !contact.isEmpty() )
      contacts.append( contact );
  }

  return contacts;
}

void CSVImportDialog::initGUI()
{
  QWidget *page = new QWidget( this );
  setMainWidget( page );

  QGridLayout *layout = new QGridLayout( page );
  layout->setSpacing( spacingHint() );
  layout->setMargin( 0 );

  QHBoxLayout *hbox = new QHBoxLayout();
  hbox->setSpacing( spacingHint() );

  QLabel *label = new QLabel( i18nc( "@label", "File to import:" ), page );
  hbox->addWidget( label );

  mUrlRequester = new KUrlRequester( page );
  mUrlRequester->setFilter( "*.csv" );
  mUrlRequester->lineEdit()->setTrapReturnKey( true );
  hbox->addWidget( mUrlRequester );

  layout->addLayout( hbox, 0, 0, 1, 5 );

  // Delimiter: comma, semicolon, tab, space, other
  QGroupBox *group = new QGroupBox( i18nc( "@title:group", "Delimiter" ), page );
  QGridLayout *delimiterLayout = new QGridLayout;
  delimiterLayout->setMargin( marginHint() );
  delimiterLayout->setSpacing( spacingHint() );
  group->setLayout( delimiterLayout );
  delimiterLayout->setAlignment( Qt::AlignTop );
  layout->addWidget( group, 1, 0, 4, 1);

  mDelimiterGroup = new QButtonGroup( this );
  mDelimiterGroup->setExclusive( true );

  QRadioButton *button = new QRadioButton( i18nc( "@option:radio Field separator", "Comma" ) );
  button->setChecked( true );
  mDelimiterGroup->addButton( button, 0 );
  delimiterLayout->addWidget( button, 0, 0 );

  button = new QRadioButton( i18nc( "@option:radio Field separator", "Semicolon" ) );
  mDelimiterGroup->addButton( button, 1 );
  delimiterLayout->addWidget( button, 0, 1 );

  button = new QRadioButton( i18nc( "@option:radio Field separator", "Tabulator" ) );
  mDelimiterGroup->addButton( button, 2 );
  delimiterLayout->addWidget( button, 1, 0 );

  button = new QRadioButton( i18nc( "@option:radio Field separator", "Space" ) );
  mDelimiterGroup->addButton( button, 3 );
  delimiterLayout->addWidget( button, 1, 1 );

  button = new QRadioButton( i18nc( "@option:radio Custum field separator", "Other" ) );
  mDelimiterGroup->addButton( button, 4 );
  delimiterLayout->addWidget( button, 0, 2 );

  mDelimiterEdit = new KLineEdit( group );
  delimiterLayout->addWidget( mDelimiterEdit, 1, 2 );

  // text quote
  label = new QLabel( i18nc( "@label:listbox", "Text quote:" ), page );
  layout->addWidget( label, 1, 2 );

  mComboQuote = new KComboBox( page );
  mComboQuote->setEditable( false );
  mComboQuote->addItem( i18nc( "@item:inlistbox Qoute character option", "\"" ), 0 );
  mComboQuote->addItem( i18nc( "@item:inlistbox Quote character option", "'" ), 1 );
  mComboQuote->addItem( i18nc( "@item:inlistbox Quote character option", "None" ), 2 );
  layout->addWidget( mComboQuote, 1, 3 );

  // date format
  label = new QLabel( i18nc( "@label:listbox", "Date format:" ), page );
  layout->addWidget( label, 2, 2 );

  mDatePatternEdit = new KLineEdit( page );
  mDatePatternEdit->setText( "Y-M-D" ); // ISO 8601 date format as default
  mDatePatternEdit->setToolTip( i18nc( "@info:tooltip",
                                         "<para><list><item>y: year with 2 digits</item>"
                                         "<item>Y: year with 4 digits</item>"
                                         "<item>m: month with 1 or 2 digits</item>"
                                         "<item>M: month with 2 digits</item>"
                                         "<item>d: day with 1 or 2 digits</item>"
                                         "<item>D: day with 2 digits</item>"
                                         "<item>H: hours with 2 digits</item>"
                                         "<item>I: minutes with 2 digits</item>"
                                         "<item>S: seconds with 2 digits</item>"
                                         "</list></para>" ) );
  layout->addWidget( mDatePatternEdit, 2, 3 );

  // text codec
  label = new QLabel( i18nc( "@label:listbox", "Text codec:" ), page );
  layout->addWidget( label, 3, 2 );

  mCodecCombo = new KComboBox( page );
  layout->addWidget( mCodecCombo, 3, 3 );

  // skip first line
  mSkipFirstRow = new QCheckBox( i18nc( "@option:check", "Skip first row of file" ), page );
  layout->addWidget( mSkipFirstRow, 4, 2, 1, 2 );

  // csv view
  mTable = new QTableView( page );
  mTable->setModel( mModel );
  mTable->setItemDelegateForRow( 0, new ContactFieldDelegate( this ) );
  mTable->horizontalHeader()->hide();
  mTable->verticalHeader()->hide();
  mTable->setEditTriggers( QAbstractItemView::CurrentChanged );
  mTable->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );
  layout->addWidget( mTable, 5, 0, 1, 5 );

  setButtonText( User1, i18nc( "@action:button", "Apply Template..." ) );
  setButtonText( User2, i18nc( "@action:button", "Save Template..." ) );

  enableButton( Ok, false );
  enableButton( User1, false );
  enableButton( User2, false );

  resize( 500, 400 );
}

void CSVImportDialog::reloadCodecs()
{
  mCodecCombo->clear();

  mCodecs.clear();

  Q_FOREACH( const QByteArray& name, QTextCodec::availableCodecs() ) {
    mCodecs.append( QTextCodec::codecForName( name ) );
  }

  mCodecCombo->addItem( i18nc( "@item:inlistbox Codec setting", "Local (%1)", QLatin1String( QTextCodec::codecForLocale()->name() ) ), Local );
  mCodecCombo->addItem( i18nc( "@item:inlistbox Codec setting", "Latin1" ), Latin1 );
  mCodecCombo->addItem( i18nc( "@item:inlistbox Codec setting", "Unicode" ), Uni );
  mCodecCombo->addItem( i18nc( "@item:inlistbox Codec setting", "Microsoft Unicode" ), MSBug );

  for ( int i = 0; i < mCodecs.count(); ++i )
    mCodecCombo->addItem( mCodecs.at( i )->name(), Codec + i );
}

void CSVImportDialog::customDelimiterChanged()
{
  if ( mDelimiterGroup->checkedId() == 4 )
    delimiterClicked( 4 );
}

void CSVImportDialog::customDelimiterChanged( const QString&, bool reload )
{
  mDelimiterGroup->button( 4 )->setChecked ( true );
  delimiterClicked( 4, reload ); // other
}

void CSVImportDialog::delimiterClicked( int id, bool reload )
{
  switch ( id ) {
    case 0: // comma
      mModel->setDelimiter( ',' );
      break;
    case 4: // other
      mDelimiterEdit->setFocus( Qt::OtherFocusReason );
      if ( !mDelimiterEdit->text().isEmpty() ) {
        mModel->setDelimiter( mDelimiterEdit->text().at( 0 ) );
      }
      break;
    case 2: // tab
      mModel->setDelimiter( '\t' );
      break;
    case 3: // space
      mModel->setDelimiter( ' ' );
      break;
    case 1: // semicolon
      mModel->setDelimiter( ';' );
      break;
  }

  if ( mDevice && reload )
    mModel->load( mDevice );
}

void CSVImportDialog::textQuoteChanged( const QString& mark, bool reload )
{
  if ( mComboQuote->currentIndex() == 2 )
    mModel->setTextQuote( QChar() );
  else
    mModel->setTextQuote( mark.at( 0 ) );

  if ( mDevice && reload )
    mModel->load( mDevice );
}

void CSVImportDialog::skipFirstRowChanged( bool checked, bool reload )
{
  mFieldSelection.clear();
  for ( int column = 0; column < mModel->columnCount(); ++column )
    mFieldSelection.append( (ContactFields::Field)mModel->data( mModel->index( 0, column ) ).toInt() );

  if ( checked )
    mModel->setStartRow( 1 );
  else
    mModel->setStartRow( 0 );

  if ( mDevice && reload )
    mModel->load( mDevice );
}

void CSVImportDialog::slotButtonClicked( int button )
{
  if ( button == KDialog::Ok ) {
    bool assigned = false;

    for ( int column = 0; column < mModel->columnCount(); ++column ) {
      if ( mModel->data( mModel->index( 0, column ), Qt::DisplayRole ).toUInt() != ContactFields::Undefined ) {
        assigned = true;
        break;
      }
    }

    if ( !assigned )
      KMessageBox::sorry( this, i18nc( "@info:status", "You have to assign at least one column." ) );
    else
      accept();
  } else if ( button == User1 ) {
    applyTemplate();
  } else if ( button == User2 ) {
    saveTemplate();
  } else if ( button == KDialog::Cancel ) {
    reject();
  }
}

void CSVImportDialog::applyTemplate()
{
  QPointer<TemplateSelectionDialog> dlg = new TemplateSelectionDialog( this );
  if ( !dlg->templatesAvailable() ) {
    KMessageBox::sorry( this, i18nc( "@label", "There are no templates available yet." ), i18nc( "@title:window", "No templates available" ) );
    delete dlg;
    return;
  }

  if ( !dlg->exec() || !dlg ) {
    delete dlg;
    return;
  }

  const QString templateFileName = dlg->selectedTemplate();
  delete dlg;

  KConfig config( templateFileName, KConfig::SimpleConfig );

  const KConfigGroup generalGroup( &config, "General" );
  mDatePatternEdit->setText( generalGroup.readEntry( "DatePattern", "Y-M-D" ) );
  mDelimiterEdit->setText( generalGroup.readEntry( "DelimiterOther" ) );

  const int delimiterButton = generalGroup.readEntry( "DelimiterType", 0 );
  const int quoteType = generalGroup.readEntry( "QuoteType", 0 );
  const bool skipFirstRow = generalGroup.readEntry( "SkipFirstRow", false );

  mDelimiterGroup->button( delimiterButton )->setChecked( true );
  delimiterClicked( delimiterButton, false );

  mComboQuote->setCurrentIndex( quoteType );
  textQuoteChanged( mComboQuote->currentText(), false );

  // do block signals here, otherwise it will trigger a reload of the model and
  // the following skipFirstRowChanged call end up with an empty model
  mSkipFirstRow->blockSignals( true );
  mSkipFirstRow->setChecked( skipFirstRow );
  mSkipFirstRow->blockSignals( false );

  skipFirstRowChanged( skipFirstRow, false );

  if ( mDevice )
    mModel->load( mDevice );

  setProperty( "TemplateFileName", templateFileName );
  connect( mModel, SIGNAL( finishedLoading() ), this, SLOT( finalizeApplyTemplate() ) );
}

void CSVImportDialog::finalizeApplyTemplate()
{
  const QString templateFileName = property( "TemplateFileName" ).toString();

  KConfig config( templateFileName, KConfig::SimpleConfig );

  const KConfigGroup generalGroup( &config, "General" );
  const uint columns = generalGroup.readEntry( "Columns", 0 );

  // create the column map
  const KConfigGroup columnMapGroup( &config, "csv column map" );

  for ( uint i = 0; i < columns; ++i ) {
    const uint assignedField = columnMapGroup.readEntry( QString::number( i ), 0 );
    mModel->setData( mModel->index( 0, i ), assignedField, Qt::EditRole );
  }
}

void CSVImportDialog::saveTemplate()
{
  const QString name = KInputDialog::getText( i18nc( "@title:window", "Template Name" ),
                                              i18nc( "@info", "Please enter a name for the template:" ) );

  if ( name.isEmpty() )
    return;

  const QString fileName = KStandardDirs::locateLocal( "data", "kaddressbook/csv-templates/"
                                                       + QUuid::createUuid()
                                                       + ".desktop" );

  KConfig config( fileName  );
  KConfigGroup generalGroup( &config, "General" );
  generalGroup.writeEntry( "DatePattern", mDatePatternEdit->text() );
  generalGroup.writeEntry( "Columns", mModel->columnCount() );
  generalGroup.writeEntry( "DelimiterType", mDelimiterGroup->checkedId() );
  generalGroup.writeEntry( "DelimiterOther", mDelimiterEdit->text() );
  generalGroup.writeEntry( "SkipFirstRow", mSkipFirstRow->isChecked() );
  generalGroup.writeEntry( "QuoteType", mComboQuote->currentIndex() );

  KConfigGroup miscGroup( &config, "Misc" );
  miscGroup.writeEntry( "Name", name );

  KConfigGroup columnMapGroup( &config, "csv column map" );
  for ( int column = 0; column < mModel->columnCount(); ++column ) {
    columnMapGroup.writeEntry( QString::number( column ),
                               mModel->data( mModel->index( 0, column ), Qt::DisplayRole ).toUInt() );
  }

  config.sync();
}

void CSVImportDialog::setFile( const KUrl &fileName )
{
   setFile( fileName.toLocalFile() );
}

void CSVImportDialog::setFile( const QString &fileName )
{
  if ( fileName.isEmpty() )
    return;

  QFile *file = new QFile( fileName );
  if ( !file->open( QIODevice::ReadOnly ) ) {
    KMessageBox::sorry( this, i18nc( "@info:status", "Cannot open input file." ) );
    delete file;
    return;
  }

  if ( mDevice )
    delete mDevice;

  mDevice = file;

  mModel->load( mDevice );
}

void CSVImportDialog::urlChanged( const QString &file )
{
  bool state = !file.isEmpty();

  enableButton( Ok, state );
  enableButton( User1, state );
  enableButton( User2, state );
}

void CSVImportDialog::codecChanged( bool reload )
{
  const int code = mCodecCombo->currentIndex();

  if ( code == Local )
    mModel->setTextCodec( QTextCodec::codecForLocale() );
  else if ( code >= Codec )
    mModel->setTextCodec( mCodecs.at( code - Codec ) );
  else if ( code == Uni )
    mModel->setTextCodec( QTextCodec::codecForName( "UTF-16" ) );
  else if ( code == MSBug )
    mModel->setTextCodec( QTextCodec::codecForName( "UTF-16LE" ) );
  else if ( code == Latin1 )
    mModel->setTextCodec( QTextCodec::codecForName( "ISO 8859-1" ) );
  else
    mModel->setTextCodec( QTextCodec::codecForName( "UTF-8" ) );

  if ( mDevice && reload )
    mModel->load( mDevice );
}

void CSVImportDialog::modelFinishedLoading()
{
  ContactFieldComboBox *box = new ContactFieldComboBox();
  int preferredWidth = box->sizeHint().width();
  delete box;

  for ( int i = 0; i < mModel->columnCount(); ++i )
    mTable->setColumnWidth( i, preferredWidth );

  for ( int column = 0; column < mFieldSelection.count(); ++column )
    mModel->setData( mModel->index( 0, column ), mFieldSelection.at( column ), Qt::EditRole );
  mFieldSelection.clear();
}

#include <csvimportdialog.moc>
