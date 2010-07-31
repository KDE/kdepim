/*
    This file is part of libkdepim.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Generic CSV import. Please do not add application specific code to this
// class. Application specific code should go to a subclass provided by the
// application using this dialog.

#include <tqbuttongroup.h>
#include <tqfile.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqlistview.h>
#include <tqradiobutton.h>
#include <tqregexp.h>
#include <tqtable.h>
#include <tqtextstream.h>
#include <tqvbox.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kcombobox.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kprogress.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>
#include <kfiledialog.h>

#include "kimportdialog.h"
#include "kimportdialog.moc"

KImportColumn::KImportColumn(KImportDialog *dlg,const TQString &header, int count)
    : m_maxCount(count),
      m_refCount(0),
      m_header(header),
      mDialog(dlg)
{
  mFormats.append(FormatPlain);
  mFormats.append(FormatUnquoted);
//  mFormats.append(FormatBracketed);

  mDefaultFormat = FormatUnquoted;

  mDialog->addColumn(this);
}

TQValueList<int> KImportColumn::formats()
{
  return mFormats;
}

TQString KImportColumn::formatName(int format)
{
  switch (format) {
    case FormatPlain:
      return i18n("Plain");
    case FormatUnquoted:
      return i18n("Unquoted");
    case FormatBracketed:
      return i18n("Bracketed");
    default:
      return i18n("Undefined");
  }
}

int KImportColumn::defaultFormat()
{
  return mDefaultFormat;
}

TQString KImportColumn::preview(const TQString &value, int format)
{
  if (format == FormatBracketed) {
    return "(" + value + ")";
  } else if (format == FormatUnquoted) {
    if (value.left(1) == "\"" && value.right(1) == "\"") {
      return value.mid(1,value.length()-2);
    } else {
      return value;
    }
  } else {
    return value;
  }
}

void KImportColumn::addColId(int id)
{
  mColIds.append(id);
}

void KImportColumn::removeColId(int id)
{
  mColIds.remove(id);
}

TQValueList<int> KImportColumn::colIdList()
{
  return mColIds;
}

TQString KImportColumn::convert()
{
  TQValueList<int>::ConstIterator it = mColIds.begin();
  if (it == mColIds.end()) return "";
  else return mDialog->cell(*it);
}


class ColumnItem : public TQListViewItem {
  public:
    ColumnItem(KImportColumn *col,TQListView *parent) : TQListViewItem(parent), mColumn(col)
    {
      setText(0,mColumn->header());
    }

    KImportColumn *column() { return mColumn; }

  private:
    KImportColumn *mColumn;
};

/**
  This is a generic class for importing line-oriented data from text files. It
  provides a dialog for file selection, preview, separator selection and column
  assignment as well as generic conversion routines. For conversion to special
  data objects, this class has to be inherited by a special class, which
  reimplements the convertRow() function.
*/
KImportDialog::KImportDialog(TQWidget* parent)
    : KDialogBase(parent,"importdialog",true,i18n("Import Text File"),Ok|Cancel),
      mSeparator(","),
      mCurrentRow(0)
{
  mData.setAutoDelete( true );

  TQVBox *topBox = new TQVBox(this);
  setMainWidget(topBox);
  topBox->setSpacing(spacingHint());

  TQHBox *fileBox = new TQHBox(topBox);
  fileBox->setSpacing(spacingHint());
  new TQLabel(i18n("File to import:"),fileBox);
  KURLRequester *urlRequester = new KURLRequester(fileBox);
  urlRequester->setFilter( "*.csv" );
  connect(urlRequester,TQT_SIGNAL(returnPressed(const TQString &)),
          TQT_SLOT(setFile(const TQString &)));
  connect(urlRequester,TQT_SIGNAL(urlSelected(const TQString &)),
          TQT_SLOT(setFile(const TQString &)));
  connect(urlRequester->lineEdit(),TQT_SIGNAL(textChanged ( const TQString & )),
          TQT_SLOT(slotUrlChanged(const TQString & )));
  mTable = new TQTable(5,5,topBox);
  mTable->setMinimumHeight( 150 );
  connect(mTable,TQT_SIGNAL(selectionChanged()),TQT_SLOT(tableSelected()));

  TQHBox *separatorBox = new TQHBox( topBox );
  separatorBox->setSpacing( spacingHint() );

  new TQLabel( i18n( "Separator:" ), separatorBox );

  mSeparatorCombo = new KComboBox( separatorBox );
  mSeparatorCombo->insertItem( "," );
  mSeparatorCombo->insertItem( i18n( "Tab" ) );
  mSeparatorCombo->insertItem( i18n( "Space" ) );
  mSeparatorCombo->insertItem( "=" );
  mSeparatorCombo->insertItem( ";" );
  connect(mSeparatorCombo, TQT_SIGNAL( activated(int) ),
          this, TQT_SLOT( separatorClicked(int) ) );
  mSeparatorCombo->setCurrentItem( 0 );

  TQHBox *rowsBox = new TQHBox( topBox );
  rowsBox->setSpacing( spacingHint() );

  new TQLabel( i18n( "Import starts at row:" ), rowsBox );
  mStartRow = new TQSpinBox( rowsBox );
  mStartRow->setMinValue( 1 );
/*
  new TQLabel( i18n( "And ends at row:" ), rowsBox );
  mEndRow = new TQSpinBox( rowsBox );
  mEndRow->setMinValue( 1 );
*/
  TQVBox *assignBox = new TQVBox(topBox);
  assignBox->setSpacing(spacingHint());

  TQHBox *listsBox = new TQHBox(assignBox);
  listsBox->setSpacing(spacingHint());

  mHeaderList = new TQListView(listsBox);
  mHeaderList->addColumn(i18n("Header"));
  connect(mHeaderList, TQT_SIGNAL(selectionChanged(TQListViewItem*)),
          this, TQT_SLOT(headerSelected(TQListViewItem*)));
  connect(mHeaderList,TQT_SIGNAL(doubleClicked(TQListViewItem*)),
          TQT_SLOT(assignColumn(TQListViewItem *)));

  mFormatCombo = new KComboBox( listsBox );
  mFormatCombo->setDuplicatesEnabled( false );

  TQPushButton *assignButton = new TQPushButton(i18n("Assign to Selected Column"),
                                              assignBox);
  connect(assignButton,TQT_SIGNAL(clicked()),TQT_SLOT(assignColumn()));

  TQPushButton *removeButton = new TQPushButton(i18n("Remove Assignment From Selected Column"),
                                              assignBox);
  connect(removeButton,TQT_SIGNAL(clicked()),TQT_SLOT(removeColumn()));

  TQPushButton *assignTemplateButton = new TQPushButton(i18n("Assign with Template..."),
                                              assignBox);
  connect(assignTemplateButton,TQT_SIGNAL(clicked()),TQT_SLOT(assignTemplate()));

  TQPushButton *saveTemplateButton = new TQPushButton(i18n("Save Current Template"),
                                              assignBox);
  connect(saveTemplateButton,TQT_SIGNAL(clicked()),TQT_SLOT(saveTemplate()));

  resize(500,300);

  connect(this,TQT_SIGNAL(okClicked()),TQT_SLOT(applyConverter()));
  connect(this,TQT_SIGNAL(applyClicked()),TQT_SLOT(applyConverter()));
  enableButtonOK(!urlRequester->lineEdit()->text().isEmpty());
}

void KImportDialog::slotUrlChanged(const TQString & text)
{
    enableButtonOK(!text.isEmpty());
}

bool KImportDialog::setFile(const TQString& file)
{
    enableButtonOK(!file.isEmpty());
  kdDebug(5300) << "KImportDialog::setFile(): " << file << endl;

  TQFile f(file);

  if (f.open(IO_ReadOnly)) {
    mFile = "";
    TQTextStream t(&f);
    mFile = t.read();
//    while (!t.eof()) mFile.append(t.readLine());
    f.close();

    readFile();

//    mEndRow->setValue( mData.count() );

    return true;
  } else {
    kdDebug(5300) << " Open failed" << endl;
    return false;
  }
}

void KImportDialog::registerColumns()
{
  TQPtrListIterator<KImportColumn> colIt(mColumns);
  for (; colIt.current(); ++colIt) {
    new ColumnItem(*colIt,mHeaderList);
  }
  mHeaderList->setSelected(mHeaderList->firstChild(),true);
}

void KImportDialog::fillTable()
{
//  kdDebug(5300) << "KImportDialog::fillTable()" << endl;

  int row, column;

  for (row = 0; row < mTable->numRows(); ++row)
      for (column = 0; column < mTable->numCols(); ++column)
          mTable->clearCell(row, column);

  for ( row = 0; row < int(mData.count()); ++row ) {
    TQValueVector<TQString> *rowVector = mData[ row ];
    for( column = 0; column < int(rowVector->size()); ++column ) {
      setCellText( row, column, rowVector->at( column ) );
    }
  }
}

void KImportDialog::readFile( int rows )
{
  kdDebug(5300) << "KImportDialog::readFile(): " << rows << endl;

  mData.clear();

  int row, column;
  enum { S_START, S_QUOTED_FIELD, S_MAYBE_END_OF_QUOTED_FIELD, S_END_OF_QUOTED_FIELD,
         S_MAYBE_NORMAL_FIELD, S_NORMAL_FIELD } state = S_START;

  TQChar m_textquote = '"';
  int m_startline = 0;

  TQChar x;
  TQString field = "";

  row = column = 0;
  TQTextStream inputStream(mFile, IO_ReadOnly);
  inputStream.setEncoding(TQTextStream::Locale);

  KProgressDialog pDialog(this, 0, i18n("Loading Progress"),
                    i18n("Please wait while the file is loaded."), true);
  pDialog.setAllowCancel(true);
  pDialog.showCancelButton(true);
  pDialog.setAutoClose(true);
  
  KProgress *progress = pDialog.progressBar();
  progress->setTotalSteps( mFile.contains(mSeparator, false) );
  progress->setValue(0);
  int progressValue = 0;
  
  if (progress->totalSteps() > 0)  // We have data
    pDialog.show();
    
  while (!inputStream.atEnd() && !pDialog.wasCancelled()) {
    inputStream >> x; // read one char

    // update the dialog if needed
    if (x == mSeparator)
    {
      progress->setValue(progressValue++);
      if (progressValue % 15 == 0) // try not to constantly repaint
        kapp->processEvents();
    }
    
    if (x == '\r') inputStream >> x; // eat '\r', to handle DOS/LOSEDOWS files correctly

    switch (state) {
      case S_START :
        if (x == m_textquote) {
          field += x;
          state = S_QUOTED_FIELD;
        } else if (x == mSeparator) {
          ++column;
        } else if (x == '\n')  {
          ++row;
          column = 0;
        } else {
          field += x;
          state = S_MAYBE_NORMAL_FIELD;
        }
        break;
      case S_QUOTED_FIELD :
        if (x == m_textquote) {
          field += x;
          state = S_MAYBE_END_OF_QUOTED_FIELD;
        } else if (x == '\n') {
          setData(row - m_startline, column, field);
          field = "";
          if (x == '\n') {
            ++row;
            column = 0;
          } else {
            ++column;
          }
          state = S_START;
        } else {
          field += x;
        }
        break;
      case S_MAYBE_END_OF_QUOTED_FIELD :
        if (x == m_textquote) {
          field += x;
          state = S_QUOTED_FIELD;
        } else if (x == mSeparator || x == '\n') {
          setData(row - m_startline, column, field);
          field = "";
          if (x == '\n') {
            ++row;
            column = 0;
          } else {
            ++column;
          }
          state = S_START;
        } else {
          state = S_END_OF_QUOTED_FIELD;
        }
        break;
      case S_END_OF_QUOTED_FIELD :
        if (x == mSeparator || x == '\n') {
          setData(row - m_startline, column, field);
          field = "";
          if (x == '\n') {
            ++row;
            column = 0;
          } else {
            ++column;
          }
          state = S_START;
        } else {
          state = S_END_OF_QUOTED_FIELD;
        }
        break;
      case S_MAYBE_NORMAL_FIELD :
        if (x == m_textquote) {
          field = "";
          state = S_QUOTED_FIELD;
        }
      case S_NORMAL_FIELD :
        if (x == mSeparator || x == '\n') {
          setData(row - m_startline, column, field);
          field = "";
          if (x == '\n') {
            ++row;
            column = 0;
          } else {
            ++column;
          }
          state = S_START;
        } else {
          field += x;
        }
    }

    if ( rows > 0 && row > rows ) break;
  }

  fillTable();
}

void KImportDialog::setCellText(int row, int col, const TQString& text)
{
  if (row < 0) return;

  if ((mTable->numRows() - 1) < row) mTable->setNumRows(row + 1);
  if ((mTable->numCols() - 1) < col) mTable->setNumCols(col + 1);

  KImportColumn *c = mColumnDict.find(col);
  TQString formattedText;
  if (c) formattedText = c->preview(text,findFormat(col));
  else formattedText = text;
  mTable->setText(row, col, formattedText);
}

void KImportDialog::formatSelected(TQListViewItem*)
{
//    kdDebug(5300) << "KImportDialog::formatSelected()" << endl;
}

void KImportDialog::headerSelected(TQListViewItem* item)
{
  KImportColumn *col = ((ColumnItem *)item)->column();

  if (!col) return;

  mFormatCombo->clear();

  TQValueList<int> formats = col->formats();

  TQValueList<int>::ConstIterator it = formats.begin();
  TQValueList<int>::ConstIterator end = formats.end();
  while(it != end) {
    mFormatCombo->insertItem( col->formatName(*it), *it - 1 );
    ++it;
  }

  TQTableSelection selection = mTable->selection(mTable->currentSelection());

  updateFormatSelection(selection.leftCol());
}

void KImportDialog::updateFormatSelection(int column)
{
  int format = findFormat(column);

  if ( format == KImportColumn::FormatUndefined )
    mFormatCombo->setCurrentItem( 0 );
  else
    mFormatCombo->setCurrentItem( format - 1 );
}

void KImportDialog::tableSelected()
{
  TQTableSelection selection = mTable->selection(mTable->currentSelection());

  TQListViewItem *item = mHeaderList->firstChild();
  KImportColumn *col = mColumnDict.find(selection.leftCol());
  if (col) {
    while(item) {
      if (item->text(0) == col->header()) {
        break;
      }
      item = item->nextSibling();
    }
  }
  if (item) {
    mHeaderList->setSelected(item,true);
  }

  updateFormatSelection(selection.leftCol());
}

void KImportDialog::separatorClicked(int id)
{
  switch(id) {
    case 0:
      mSeparator = ',';
      break;
    case 1:
      mSeparator = '\t';
      break;
    case 2:
      mSeparator = ' ';
      break;
    case 3:
      mSeparator = '=';
      break;
    case 4:
      mSeparator = ';';
      break;
    default:
      mSeparator = ',';
      break;
  }

  readFile();
}

void KImportDialog::assignColumn(TQListViewItem *item)
{
  if (!item) return;

//  kdDebug(5300) << "KImportDialog::assignColumn(): current Col: " << mTable->currentColumn()
//            << endl;

  ColumnItem *colItem = (ColumnItem *)item;

  TQTableSelection selection = mTable->selection(mTable->currentSelection());

//  kdDebug(5300) << " l: " << selection.leftCol() << "  r: " << selection.rightCol() << endl;

  for(int i=selection.leftCol();i<=selection.rightCol();++i) {
    if (i >= 0) {
      mTable->horizontalHeader()->setLabel(i,colItem->text(0));
      mColumnDict.replace(i,colItem->column());
      int format = mFormatCombo->currentItem() + 1;
      mFormats.replace(i,format);
      colItem->column()->addColId(i);
    }
  }

  readFile();
}

void KImportDialog::assignColumn()
{
  assignColumn(mHeaderList->currentItem());
}

void KImportDialog::assignTemplate()
{
  TQMap<uint,int> columnMap;
  TQMap<TQString, TQString> fileMap;
  TQStringList templates;

  // load all template files
  TQStringList list = KGlobal::dirs()->findAllResources( "data" , TQString( kapp->name() ) +
      "/csv-templates/*.desktop", true, true );

  for ( TQStringList::iterator it = list.begin(); it != list.end(); ++it )
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
  TQString tmp;
  tmp = KInputDialog::getItem( i18n( "Template Selection" ),
                  i18n( "Please select a template, that matches the CSV file:" ),
                  templates, 0, false, &ok, this );

  if ( !ok )
    return;

  KSimpleConfig config( fileMap[ tmp ], true );
  config.setGroup( "General" );
  uint numColumns = config.readUnsignedNumEntry( "Columns" );
  int format = config.readNumEntry( "Format" );

  // create the column map
  config.setGroup( "csv column map" );
  for ( uint i = 0; i < numColumns; ++i ) {
    int col = config.readNumEntry( TQString::number( i ) );
    columnMap.insert( i, col );
  }

  // apply the column map
  for ( uint i = 0; i < columnMap.count(); ++i ) {
    int tableColumn = columnMap[i];
    if ( tableColumn == -1 )
      continue;
    KImportColumn *col = mColumns.at(i);
    mTable->horizontalHeader()->setLabel( tableColumn, col->header() );
    mColumnDict.replace( tableColumn, col );
    mFormats.replace( tableColumn, format );
    col->addColId( tableColumn );
  }

  readFile();
}

void KImportDialog::removeColumn()
{
  TQTableSelection selection = mTable->selection(mTable->currentSelection());

//  kdDebug(5300) << " l: " << selection.leftCol() << "  r: " << selection.rightCol() << endl;

  for(int i=selection.leftCol();i<=selection.rightCol();++i) {
    if (i >= 0) {
      mTable->horizontalHeader()->setLabel(i,TQString::number(i+1));
      KImportColumn *col = mColumnDict.find(i);
      if (col) {
        mColumnDict.remove(i);
        mFormats.remove(i);
        col->removeColId(i);
      }
    }
  }

  readFile();
}

void KImportDialog::applyConverter()
{
  kdDebug(5300) << "KImportDialog::applyConverter" << endl;

  KProgressDialog pDialog(this, 0, i18n("Importing Progress"),
                    i18n("Please wait while the data is imported."), true);
  pDialog.setAllowCancel(true);
  pDialog.showCancelButton(true);
  pDialog.setAutoClose(true);
  
  KProgress *progress = pDialog.progressBar();
  progress->setTotalSteps( mTable->numRows()-1 );
  progress->setValue(0);

  readFile( 0 );
  
  pDialog.show();
  for( uint i = mStartRow->value() - 1; i < mData.count() && !pDialog.wasCancelled(); ++i ) {
    mCurrentRow = i;
    progress->setValue(i);
    if (i % 5 == 0)  // try to avoid constantly processing events
      kapp->processEvents();
      
    convertRow();
  }
}

int KImportDialog::findFormat(int column)
{
  TQMap<int,int>::ConstIterator formatIt = mFormats.find(column);
  int format;
  if (formatIt == mFormats.end()) format = KImportColumn::FormatUndefined;
  else format = *formatIt;

//  kdDebug(5300) << "KImportDialog::findformat(): " << column << ": " << format << endl;

  return format;
}

TQString KImportDialog::cell(uint col)
{
  if ( col >= mData[ mCurrentRow ]->size() ) return "";
  else return data( mCurrentRow, col );
}

void KImportDialog::addColumn(KImportColumn *col)
{
  mColumns.append(col);
}

void KImportDialog::setData( uint row, uint col, const TQString &value )
{
  TQString val = value;
  val.replace( "\\n", "\n" );

  if ( row >= mData.count() ) {
    mData.resize( row + 1 );
  }
  
  TQValueVector<TQString> *rowVector = mData[ row ];
  if ( !rowVector ) {
    rowVector = new TQValueVector<TQString>;
    mData.insert( row, rowVector );
  }
  if ( col >= rowVector->size() ) {
    rowVector->resize( col + 1 );
  }
  
  KImportColumn *c = mColumnDict.find( col );
  if ( c )
  	rowVector->at( col ) = c->preview( val, findFormat(col) );
  else
    rowVector->at( col ) = val;
}

TQString KImportDialog::data( uint row, uint col )
{
  return mData[ row ]->at( col );
}

void KImportDialog::saveTemplate()
{
  TQString fileName = KFileDialog::getSaveFileName(
                      locateLocal( "data", TQString( kapp->name() ) + "/csv-templates/" ),
                      "*.desktop", this );

  if ( fileName.isEmpty() )
    return;

  if ( !fileName.contains( ".desktop" ) )
    fileName += ".desktop";

  TQString name = KInputDialog::getText( i18n( "Template Name" ), i18n( "Please enter a name for the template:" ) );

  if ( name.isEmpty() )
    return;

  KConfig config( fileName );
  config.setGroup( "General" );
  config.writeEntry( "Columns", mColumns.count() );
  config.writeEntry( "Format", mFormatCombo->currentItem() + 1 );

  config.setGroup( "Misc" );
  config.writeEntry( "Name", name );

  config.setGroup( "csv column map" );
  
  KImportColumn *column;
  uint counter = 0;
  for ( column = mColumns.first(); column; column = mColumns.next() ) {
    TQValueList<int> list = column->colIdList();
    if ( list.count() > 0 )
      config.writeEntry( TQString::number( counter ), list[ 0 ] );
    else
      config.writeEntry( TQString::number( counter ), -1 );
    counter++;
  }

  config.sync();
}
