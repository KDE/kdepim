#include "browserwidget.h"
#include "entryeditorwidget.h"
#include "viewoptions.h"
#include "selectfields.h"
#include "attributes.h"
#include "entry.h"
#include "browserentryeditor.h"

#include <qtabwidget.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qgroupbox.h>
#include <qbuttongroup.h> 
#include <qlistbox.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qcombobox.h>
#include <qtooltip.h>
#include <qdialog.h>
#include <qheader.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qimageio.h>
#include <qcheckbox.h>
#include <qradiobutton.h>

#include <klocale.h>
#include <kconfig.h>
#include <kapp.h>
#include <kcolorbtn.h>

#include <kglobal.h>
#include <kiconloader.h>

#include "stdlib.h" // for atoi

/////////////////////////////////
// PwDelete Methods

PwDeleteCommand::PwDeleteCommand( PabWidget *pw, 
				  QString entryKey, 
          KAB::Entity *ce )
  : pw( pw ), entryKey( entryKey ), ce( new KAB::Entity( *ce ))
{
  redo();
}

PwDeleteCommand::~PwDeleteCommand()
{
  delete ce;
}

QString PwDeleteCommand::name()
{
  return i18n( "Delete" );
}

void PwDeleteCommand::undo()
{
  KAB::AddressBookClient* cel = pw->contactEntryList();
// Rikkus: FIXME  cel->unremove( entryKey, new KAB::Entity( *ce ));
  PabListViewItem *plvi;
  plvi = new PabListViewItem( entryKey, pw->pabListView(), pw->fields() );
  plvi->refresh();
  pw->pabListView()->resort();
  pw->pabListView()->setCurrentItem( plvi );
}

void PwDeleteCommand::redo()
{
  KAB::AddressBookClient *cel = pw->contactEntryList();
  cel->remove( entryKey );
  delete pw->pabListView()->getItem( entryKey );
}

/////////////////////////////////
// PwPaste Methods

PwPasteCommand::PwPasteCommand( PabWidget *pw, QString clipboard )
{
  this->pw = pw;
  this->clipboard = clipboard;

  QTextIStream clipStream( &clipboard );
  KAB::AddressBookClient* cel = pw->contactEntryList();
  while (!clipStream.eof()) {
// Rikkus: FIXME
//    KAB::Entity *newEntry = new KAB::Entity( clipStream );
//    QString key = cel->insert( newEntry );
//    keyList.append( key );
//    PabListViewItem* plvi = pw->addEntry( key );
//    pw->pabListView()->setSelected( plvi, true );
  }
}

QString PwPasteCommand::name()
{
  return i18n( i18n( "Paste" ));
}

void PwPasteCommand::undo()
{
//  PabListViewItem* plvi; Rikkus: unused ?
  KAB::AddressBookClient* cel = pw->contactEntryList();
  QStringList::Iterator it;
  for( it = keyList.begin(); it != keyList.end(); ++it ) {
    cel->remove( *it );
    delete pw->pabListView()->getItem( *it );
  }
}

void PwPasteCommand::redo()
{
  QTextIStream clipStream( &clipboard );
  KAB::AddressBookClient* cel = pw->contactEntryList();
  QStringList::Iterator it;
  for( it = keyList.begin(); it != keyList.end(); ++it ) {
    // Rikkus: FIXME
#if 0
    if (clipStream.eof())
      break;
    KAB::Entity *newEntry = new KAB::Entity( clipStream );
    cel->unremove( *it, newEntry );
    PabListViewItem* plvi = pw->addEntry( *it );
    pw->pabListView()->setSelected( plvi, true );
#endif
  }
  pw->pabListView()->resort();
}

/////////////////////////////////
// PwNew Methods

PwNewCommand::PwNewCommand( PabWidget *pw, KAB::Entity *ce )
  : pw( pw ), ce( ce )
{
  KAB::AddressBookClient* cel = pw->contactEntryList();
  entryKey = ce->id();
  bool wroteOK = cel->write(ce);
  PabListViewItem *plvi;
  plvi = new PabListViewItem( entryKey, pw->pabListView(), pw->fields() );
  plvi->refresh();
  pw->pabListView()->resort();
}

QString PwNewCommand::name()
{
  return i18n( i18n( "New Entry" ));
}

void PwNewCommand::undo()
{
  KAB::AddressBookClient *cel = pw->contactEntryList();
  KAB::Entity *tempce = cel->entity( entryKey );
  if (!tempce) { // Another process deleted it already(!)
    debug( "PwNewCommand::undo() Associated Entity not found." );
    debug( "Unable to undo insert" );
  }
  else
    ce = new KAB::Entity( *tempce );
  PabListViewItem *plvi = pw->pabListView()->getItem( entryKey );
  if (plvi)
    delete plvi;
  else // Should never happen
    debug( "PwNewCommand::undo() missing PabListViewItem." );
  cel->remove( entryKey );
}

void PwNewCommand::redo()
{
  KAB::AddressBookClient * cel = pw->contactEntryList();
  // Rikkus: FIXME
#if 0
  cel->unremove( entryKey, new KAB::Entity( *ce ));
  PabListViewItem *plvi;
  plvi = new PabListViewItem( entryKey, pw->pabListView(), pw->fields() );
  plvi->refresh();
  pw->pabListView()->resort();
#endif
}

/////////////////////////////////
// PwEdit Methods

PwEditCommand::PwEditCommand( PabWidget *pw, 
			      QString entryKey,
            KAB::Entity *oldCe, 
            KAB::Entity *newCe )
{
  this->pw = pw;
  this->entryKey = entryKey;
  this->oldCe = new KAB::Entity( *oldCe );
  this->newCe = new KAB::Entity( *newCe );
  redo();
}

PwEditCommand::~PwEditCommand()
{
  delete oldCe;
  delete newCe;
}

QString PwEditCommand::name()
{
  return i18n( i18n( "Entry Edit" ));
}

void PwEditCommand::undo()
{
  KAB::AddressBookClient *cel = pw->contactEntryList();
  // Rikkus: FIXME
#if 0
  cel->replace( entryKey, new KAB::Entity( *oldCe ));

  PabListViewItem *plvi = pw->pabListView()->getItem( entryKey );
  if (plvi)
    plvi->refresh();
  delete new QListViewItem( plvi->parent() ); //force resort
  pw->pabListView()->resort();  //grossly inefficient?
#endif
}

void PwEditCommand::redo()
{
  KAB::AddressBookClient *cel = pw->contactEntryList();
  cel->update(newCe);

  PabListViewItem *plvi = pw->pabListView()->getItem( entryKey );
  if (plvi)
    plvi->refresh();
  delete new QListViewItem( plvi->parent() ); //force resort
  pw->pabListView()->resort();  //grossly inefficient?
}

/////////////////////////////////
// PwCut Methods

PwCutCommand::PwCutCommand( PabWidget *pw )
{
  this->pw = pw;
  QTextOStream clipStream( &clipText );
  PabListView *listView = pw->pabListView();
  QListViewItem *item;
  KAB::AddressBookClient *cel = pw->contactEntryList();
  for(item = listView->firstChild(); item; item = item->itemBelow()) {
    if (!listView->isSelected( item ))
      continue;
    PabListViewItem *plvi = dynamic_cast< PabListViewItem* >(item);
    if (!plvi)
      continue;
    QString entryKey = plvi->entryKey();
    KAB::Entity *ce = plvi->getEntry();
    if (!ce)
      continue;
// Rikkus: FIXME    ce->save( clipStream );
    cel->remove( entryKey );
    delete plvi;
    keyList.append( entryKey );
  }
  QClipboard *cb = QApplication::clipboard();
  oldText = cb->text();
  cb->setText( clipText );
}

QString PwCutCommand::name()
{
  return i18n( "Cut" );
}

void PwCutCommand::undo()
{
  QTextIStream clipStream( &clipText );
  KAB::AddressBookClient* cel = pw->contactEntryList();
  QStringList::Iterator it;
  for( it = keyList.begin(); it != keyList.end(); ++it ) {
    if (clipStream.eof())
      break;
// Rikkus: FIXME    KAB::Entity *newEntry = new KAB::Entity( clipStream );
//  cel->unremove( *it, newEntry );
    PabListViewItem* plvi = pw->addEntry( *it );
    pw->pabListView()->setSelected( plvi, true );
  }
  pw->pabListView()->resort();
  QClipboard *cb = QApplication::clipboard();
  cb->setText( oldText );
}

void PwCutCommand::redo()
{
  PabListViewItem* plvi;
  KAB::AddressBookClient* cel = pw->contactEntryList();
  QStringList::Iterator it;
  for( it = keyList.begin(); it != keyList.end(); ++it ) {
    delete pw->pabListView()->getItem( *it );
    cel->remove( *it );
  }
  QClipboard *cb = QApplication::clipboard();
  cb->setText( clipText );
}

/////////////////////////////////
// DynamicTip Methods

DynamicTip::DynamicTip( PabListView *parent )
  : QToolTip( parent )
{
    // no explicit initialization needed
}

void DynamicTip::maybeTip( const QPoint &pos )
{
    if (!parentWidget()->inherits( "PabListView" ))
	return;
    PabListView *plv = (PabListView*)parentWidget();
    if (!plv->tooltips())
      return;
    QPoint posVp = plv->viewport()->pos();

    QListViewItem *lvi = plv->itemAt( pos - posVp );
    if (!lvi)
      return;
    PabListViewItem *plvi = dynamic_cast< PabListViewItem* >(lvi);
    if (!plvi)
      return;
    KAB::AddressBookClient *cel = plvi->parent()->getPabWidget()->contactEntryList();
    KAB::Entity *ce = cel->entity( plvi->entryKey() );
    if (!ce)
      return;
    QString s;
    QRect r = plv->itemRect( lvi );
    r.moveBy( posVp.x(), posVp.y() );
    
    KAB::Field * f = ce->find("N");
    if (f != 0)
      s += i18n( "Name" ) + ": " + QString(f->data()) + "\n";
    
    f = ce->find("ORG");
    if (f != 0)
      s += i18n( "Company" ) + ": " + QString(f->data()) + "\n";
    
    f = ce->find("X-Notes");
    if (f != 0)
      s += i18n( "Notes:" ) + "\n" +
        QString(f->data()).stripWhiteSpace() + "\n";
    
    tip( r, s );
}

///////////////////////////
// PabListViewItem Methods

PabListViewItem::PabListViewItem( QString entryKey,
				  PabListView* parent, 
				  QStringList* field )
  : QListViewItem( parent ), entryKey_( entryKey ), field( field ),
    parentListView( parent )
{
  refresh();
}

QString PabListViewItem::entryKey()
{
  return entryKey_;
}

KAB::Entity *PabListViewItem::getEntry()
{
  KAB::AddressBookClient *cel = parent()->getPabWidget()->contactEntryList();
  KAB::Entity *ce = cel->entity( entryKey_ );
  if (!ce)  // can only happen to shared address book
    debug( "PabListViewItem::getEntry() Associated Entity not found" );
  return ce;
}

QString PabListViewItem::key( int column, bool ascending ) const 
{
  return QListViewItem::key( column, ascending ).lower();
}

// Some of this is very similar to TrollTechs code,
// I should ask if it's ok or not.
void PabListViewItem::paintCell ( QPainter * p, 
				  const QColorGroup & cg, 
				  int column, 
				  int width, 
				  int align )
{
  if ( !p )
    return;

  QListView *lv = listView();
  int r = lv ? lv->itemMargin() : 1;
  const QPixmap * icon = pixmap( column );
  int marg = lv ? lv->itemMargin() : 1;

  if (!parentListView->backPixmapOn || parentListView->background.isNull()) {
    p->fillRect( 0, 0, width, height(), cg.base() );
    if ( isSelected() &&
	 (column==0 || listView()->allColumnsShowFocus()) ) {
      p->fillRect( r - marg, 0, width - r + marg, height(),
		   cg.brush( QColorGroup::Highlight ) );
      p->setPen( cg.highlightedText() );
    } else {
      p->setPen( cg.text() );
    }
  }
  else {
    QRect rect = parentListView->itemRect( this );
    int cw = 0;
    cw = parentListView->header()->cellPos( column );
    
    QPixmap* back = &(parentListView->background);
    if (isSelected()) {
      back = &(parentListView->iBackground);
      p->setPen( cg.highlightedText() );
    }
    p->drawTiledPixmap( 0, 0, width, height(), 
			*back,
			rect.left() + cw + parentListView->contentsX(), 
			rect.top() + parentListView->contentsY() );
      
    if ( icon ) {
      p->drawPixmap( r, (height()-icon->height())/2, *icon );
      r += icon->width() + listView()->itemMargin();
    }
  }
    
  QString t = text( column );
  if ( !t.isEmpty() ) {
    p->drawText( r, 0, width-marg-r, height(),
		 align | AlignVCenter, t );
  }

  if (parentListView->underline) {
    p->setPen( parentListView->cUnderline );
    p->drawLine( 0, height() - 1, width, height() - 1 );
  }
}

PabListView *PabListViewItem::parent()
{
  return parentListView;
}

void PabListViewItem::refresh()
{
  KAB::Entity *ce = getEntry();
  if (!ce)
    return;
  for ( uint i = 0; i < field->count(); i++ ) {
    if ((*field)[i] == "X-FileAs") {
      KAB::Field * f = ce->find("X-Notes");
      if (f != 0)
	      setPixmap( i, BarIcon( "abentry" ));
      else
	      setPixmap( i, BarIcon( "group" ));
    }
      
    KAB::Field * f = ce->find((*field)[i]);
    if (f != 0)
      setText( i, QString(f->data()));
    else
      setText( i, "" );
  }
}

/////////////////////
// PabWidget Methods

void PabWidget::selectNames( QStringList newFields )
{
  field.clear();
  fieldWidth.clear();
  if (newFields.count() == 0) {
    newFields += "X-FileAs";
    newFields += "EMAIL";
    newFields += "X-BusinessPhone";
    newFields += "X-HomePhone";
  }

  QStringList::Iterator it;
  for(it = newFields.begin(); it != newFields.end(); ++it)
  {
    field += *it;
    if (*it == "X-FileAs")
      fieldWidth += 180;
    else if (*it == "EMAIL")
      fieldWidth += 160;
    else
      fieldWidth += 120;
  }
}

KAB::AddressBookClient *PabWidget::contactEntryList()
{
  return cel;
}

PabListView *PabWidget::pabListView()
{
  return listView;
}

QStringList *PabWidget::fields()
{ 
  return &field; 
}

void PabWidget::showSelectNameDialog()
{
  SelectFields* sel = new SelectFields( field, this, "select", TRUE );
  if (!sel->exec())
    return;
  // Can't undo this action maybe should show a warning.
  //1  UndoStack::instance()->clear();
  //1  RedoStack::instance()->clear();
  
  selectNames( sel->chosenFields() );
  reconstructListView();
  delete sel;
}

void PabWidget::defaultSettings()
{
  selectNames( QStringList() );
  listView->backPixmapOn = false;
  listView->underline = true;
  listView->autUnderline = true;
  listView->tooltips_ = true;
  reconstructListView();
}

void PabWidget::reconstructListView()
{
  cbField->clear();
  for ( uint i = 0; i < field.count(); i++ )
    cbField->insertItem( Attributes::instance()->fieldToName( field[i]));

  QObject::disconnect( iSearch, SIGNAL( textChanged( const QString& )), 
		       listView, SLOT( incSearch( const QString& ))); 
  QObject::disconnect( cbField, SIGNAL( activated( int )),
		       listView, SLOT( setSorting( int )));
  delete listView;

  setupListView();
  listView->setSorting( 0, true );  

  QObject::connect( iSearch, SIGNAL( textChanged( const QString& )), 
		    listView, SLOT( incSearch( const QString& ))); 
  QObject::connect( cbField, SIGNAL( activated( int )),
		    listView, SLOT( setSorting( int )));
  mainLayout->addWidget( listView );
  mainLayout->activate();
  listView->show();
}

PabWidget::PabWidget( KAB::AddressBookClient *cel,
		      QWidget *parent, 
		      const char *name )
  : QWidget( parent, name ), cel( cel )
{
  qInitImageIO();

  readConfig();
  mainLayout = new QVBoxLayout( this, 2 );

  QBoxLayout *searchLayout = new QHBoxLayout( mainLayout, 2 );

  QLabel *liSearch = new QLabel( "&Incremental Search", this );
  liSearch->resize( liSearch->sizeHint() );
  searchLayout->addWidget( liSearch, 0 );

  iSearch = new QLineEdit( this );
  iSearch->resize( iSearch->sizeHint() );
  searchLayout->addWidget( iSearch, 0 );
  liSearch->setBuddy( iSearch );

  // Create a non-editable Combobox and a label below...
  cbField = new QComboBox( FALSE, this );
  cbField->resize( iSearch->sizeHint() );
  searchLayout->addWidget( cbField, 0 );

  for ( uint i = 0; i < field.count(); i++ )
    cbField->insertItem( Attributes::instance()->fieldToName( field[i]));
  setupListView();
  listView->resort();

  mainLayout->addWidget( listView );
  mainLayout->activate();
  resize( sizeHint() );
  QObject::connect( iSearch, SIGNAL( textChanged( const QString& )), 
		    listView, SLOT( incSearch( const QString& ))); 
  QObject::connect( cbField, SIGNAL( activated( int )),
		    listView, SLOT( setSorting( int )));
  QObject::connect( listView, SIGNAL( returnPressed( QListViewItem *)),
		    this, SLOT( properties()));
}

PabWidget::~PabWidget()
{
  debug( "Destroying PabWidget" );
}

void PabWidget::setupListView()
{
  listView = new PabListView( this );
  listView->setMultiSelection( TRUE );
  QObject::connect( listView, SIGNAL( selectionChanged() ), 
		    this, SLOT( selectionChanged() ) );
  QObject::connect( listView, SIGNAL( doubleClicked( QListViewItem* ) ), 
		    this, SLOT( itemSelected( QListViewItem* ) ) );
  repopulate();
}

void PabWidget::repopulate()
{
  listView->clear();
  for ( uint i = 0; i < field.count(); i++ )
    listView->addColumn( Attributes::instance()->fieldToName( field[i] ), 
			 fieldWidth[i] );

  QStrList keyList = cel->allKeys();

  QStrListIterator it(keyList);
  for (; it.current() ; ++it)
    addEntry(it.current());
}

PabListViewItem* PabWidget::addEntry( QString entryKey )
{
  PabListViewItem *item = new PabListViewItem( entryKey, listView, &field );
  item->refresh();
  return item;
}

// Will have to insert into cel and save key
void PabWidget::addNewEntry( KAB::Entity *ce )
{
  PwNewCommand *command = new PwNewCommand( this, ce );
  UndoStack::instance()->push( command );
  RedoStack::instance()->clear();
}

void PabWidget::selectionChanged()
{
}

void PabWidget::selectAll()
{
  QListViewItem *item;
  for(item = listView->firstChild(); item; item = item->itemBelow())
    listView->setSelected( item, true );
}

void PabWidget::properties()
{
  itemSelected( listView->currentItem() );
}

void PabWidget::itemSelected( QListViewItem *item )
{
  PabListViewItem *plvi = dynamic_cast< PabListViewItem* >(item);
  if (plvi) {
    QString title = i18n( "Address Book Entry Editor" );
    QString entryKey = plvi->entryKey();
    KAB::Entity *ce = cel->entity( entryKey );
    if (!ce) { // Another process deleted it(!)
      debug( "PabWidget::itemSelected Associated entry not found" );
      return;
    }
    PabContactDialog *cd = new PabContactDialog( this, title, entryKey, ce );
    QObject::connect( cd, SIGNAL( change( QString, KAB::Entity* ) ), 
		      this, SLOT( change( QString, KAB::Entity* ) ));
    cd->show();
  }
  item->setSelected( TRUE );
  item->repaint();
}

void PabWidget::change( QString entryKey, KAB::Entity *ce )
{
  PabListViewItem *plvi = listView->getItem( entryKey );
  KAB::Entity *oldce = cel->entity( entryKey );
  if (plvi && oldce) {    
    PwEditCommand *command = new PwEditCommand( this, entryKey, oldce, ce );
    UndoStack::instance()->push( command );
    RedoStack::instance()->clear();
  }
  else {
    PwNewCommand *command = new PwNewCommand( this, ce );
    UndoStack::instance()->push( command );
    RedoStack::instance()->clear();
  }
}

void PabWidget::cut()
{
  PwCutCommand *command = new PwCutCommand( this );
  UndoStack::instance()->push( command );
  RedoStack::instance()->clear();
}

void PabWidget::copy()
{
  QListViewItem *item;
  QString clipText;
  QTextOStream clipStream( &clipText );
  for(item = listView->firstChild(); item; item = item->itemBelow()) {
    if (!listView->isSelected( item ))
      continue;
    PabListViewItem *lvi = dynamic_cast< PabListViewItem* >(item);
    if (lvi) {
      KAB::Entity *ce = lvi->getEntry();
      if (ce) {
        // Rikkus: FIXME
     //   ce->save( clipStream );
      }
    }
  }  
  QClipboard *cb = QApplication::clipboard();
  cb->setText( clipText );
}

void PabWidget::paste()
{
  QClipboard *cb = QApplication::clipboard();
  PwPasteCommand *command = new PwPasteCommand( this, cb->text() );
  UndoStack::instance()->push( command );
  RedoStack::instance()->clear();
}

void PabWidget::clear()
{
  debug( "clear" );
  QListViewItem *item = listView->currentItem();
  PabListViewItem *lvi = dynamic_cast< PabListViewItem* >(item);
  QString entryKey = lvi->entryKey();
  KAB::Entity *ce = lvi->getEntry();
  if (lvi) {
    PwDeleteCommand *command = new PwDeleteCommand( this, entryKey, ce );
    UndoStack::instance()->push( command );
    RedoStack::instance()->clear();
  }
}

void PabWidget::saveConfig()
{
  KConfig *config = kapp->getConfig();

  listView->saveConfig();

  config->setGroup("Browser");

  QStringList actualFields;
  QStringList actualWidths;
  for(uint i = 0; i < field.count(); ++i) {
    int act = listView->header()->mapToLogical( i );
    actualFields += field[act];
    int size = listView->header()->cellSize( act );
    actualWidths += QString().setNum( size );
  }

  config->writeEntry("fields", actualFields );
  config->writeEntry("fieldWidths", actualWidths );
}

void PabWidget::readConfig()
{
  KConfig *config = kapp->getConfig();

  config->setGroup("Browser");
  field = config->readListEntry("fields" );
  selectNames( field );
  QStringList fieldWidthStr = config->readListEntry("fieldWidths" );
  fieldWidth.clear();
  QStringList::Iterator it;
  for(it = fieldWidthStr.begin(); it != fieldWidthStr.end(); ++it)
    fieldWidth += (*it).toInt();
  while (fieldWidth.count() < field.count())
    fieldWidth += 120;
}

void PabWidget::viewOptions()
{
  ViewOptions *vo = new ViewOptions( listView->backPixmapOn,
				     listView->backPixmap,
				     listView->underline, 
				     listView->autUnderline,
				     listView->cUnderline, 
				     listView->tooltips(), 
				     this, 
				     "ViewOptions", 
				     true );

  if (!vo->exec())
    return;
  listView->backPixmapOn = vo->ckBackPixmap->isChecked();
  listView->backPixmap = vo->leBackPixmap->text();
  listView->underline = vo->ckUnderline->isChecked();
  listView->autUnderline = vo->ckAutUnderline->isChecked();
  listView->cUnderline = vo->kcbUnderline->color();
  listView->tooltips_ = vo->ckTooltips->isChecked();
  listView->loadBackground();
  listView->saveConfig();
  listView->triggerUpdate();
  delete vo;
}

///////////////////////
// PabListView Methods

PabListView::PabListView( PabWidget *parent, const char *name )
  : QListView( parent, name ), 
    pabWidget( parent ),
    oldColumn( 0 )
{
  setAllColumnsShowFocus( true );
  up = new QIconSet( BarIcon("up" ), QIconSet::Small );
  down = new QIconSet( BarIcon("down" ), QIconSet::Small );
  new DynamicTip( this );
  readConfig();
  loadBackground();
}

PabWidget* PabListView::getPabWidget()
{
  return pabWidget;
}

PabListViewItem *PabListView::getItem( QString entryKey )
{
  KAB::AddressBookClient *cel = pabWidget->contactEntryList();
  QListViewItem *item = firstChild();
  PabListViewItem *plvi;
  while (item) {
    plvi = dynamic_cast< PabListViewItem* >(item);
    if (plvi && (plvi->entryKey() == entryKey))
      return plvi;
    item = item->nextSibling();
  }
  return 0;
}

void PabListView::loadBackground()
{
  debug( QString ("Image format ") + QPixmap::imageFormat( backPixmap ) );
  if (backPixmapOn && QPixmap::imageFormat( backPixmap )) {
    background = QPixmap( backPixmap );
    QImage invertedBackground( backPixmap );

    /* Thin lines
    const int multa = 6;
    const int multb = 1;
    const int multc = multa + multb;
    invertedBackground.convertDepth( 32 );
    for (int y = 17; y < invertedBackground.height(); y += 18)
      for( int x = 0; x < invertedBackground.width(); ++x ) {
	QRgb a = invertedBackground.pixel( x, y );
	a = ((qRed(a)*multa + (0xFF - qRed(a))*multb)/multc << 16) |
		    ((qGreen(a)*multa + (0xFF - qGreen(a))*multb)/multc << 8) |
		    (qBlue(a)*multa + (0xFF - qBlue(a))*multb)/multc;
	invertedBackground.setPixel( x, y, a  ); //qRed(a) << 16 );  
      }
    background.convertFromImage( invertedBackground );
    */

    invertedBackground.invertPixels();
    iBackground.convertFromImage( invertedBackground );
  } 
  else {
    background = QPixmap();
    iBackground = QPixmap();
  }  
}

void PabListView::saveConfig()
{
  KConfig *config = kapp->getConfig();

  config->setGroup("ListView");
  config->writeEntry( "sortColumn", header()->mapToActual( column )); 
  config->writeEntry( "sortDirection", ascending ); 

  config->writeEntry( "backPixmapOn", backPixmapOn );
  config->writeEntry( "backPixmap", backPixmap );

  config->writeEntry( "underline", underline );
  config->writeEntry( "autUnderline", autUnderline );
  config->writeEntry( "cUnderline", cUnderline );

  config->writeEntry( "tooltips", tooltips_ );
}

void PabListView::readConfig()
{
  KConfig *config = kapp->getConfig();

  config->setGroup("ListView");
  column = config->readNumEntry( "sortColumn", 0 ); 
  ascending = config->readBoolEntry( "sortDirection", true );

  backPixmapOn = config->readBoolEntry( "backPixmapOn", false );
  backPixmap = config->readEntry( "backPixmap", "" );  

  underline = config->readBoolEntry( "underline", true );
  autUnderline = config->readBoolEntry( "autUnderline", true );
  cUnderline = config->readColorEntry( "cUnderline" );
  if (autUnderline)
    cUnderline = kapp->palette().normal().background();
  tooltips_ = config->readBoolEntry( "tooltips", true );
}

// untested, changing kde color scheme isn't affecting qt 2.0 based apps
void PabListView::backgroundColorChange( const QColor &color )
{
  if (autUnderline)
    cUnderline = kapp->palette().normal().background();
  QListView::backgroundColorChange( color );
}

void PabListView::paintEmptyArea( QPainter * p, const QRect & rect )
{
  if (backPixmapOn && !background.isNull())
    p->drawTiledPixmap( rect.left(), rect.top(), rect.width(), rect.height(), 
			background, 
			rect.left() + contentsX(), 
			rect.top() + contentsY() );
  else 
    p->fillRect( rect, colorGroup().base() );
  //    p->fillRect( rect, QColor( 255, 0, 0) );
}

// It should be note that QListView supplies outstanding incremental
// searching, just give it focus and try!
// This class is pretty much just eye candy :-)
// (might be useful for people who can't type quickly or 
// don't realize how good the built in incremental searching is)
void PabListView::incSearch( const QString &value )
{
  if (value == "")
    return;
  QListViewItem *citem = currentItem();

  if (ascending) {
    if (!citem) 
      citem = firstChild();
    if (!citem)
      return;

    QListViewItem *ib = citem->itemAbove();
    while (ib && //(citem->key( column, ascending ).find( value ) != 0) &&
	   ((ib->key( column, ascending ) > value) ||
	    (ib->key( column, ascending ).find( value ) == 0))) {
      citem = ib;
      ib = ib->itemAbove();
    }

    ib = citem->itemBelow();
    while (ib && (citem->key( column, ascending ).find( value ) != 0) &&
	   ((ib->key( column, ascending ) < value) ||
	    (ib->key( column, ascending ).find( value ) == 0))) {
      debug( ib->key( column, ascending ));
      citem = ib;
      ib = ib->itemBelow();
    }
  }
  else {
    if (!citem) 
      citem = firstChild();
    if (!citem)
      return;

    QListViewItem *ib = citem->itemAbove();
    while (ib && //(citem->key( column, ascending ).find( value ) != 0) &&
	   ((ib->key( column, ascending ) < value) ||
	    (ib->key( column, ascending ).find( value ) == 0))) {
      citem = ib;
      ib = ib->itemAbove();
    }

    ib = citem->itemBelow();
    while (ib && (citem->key( column, ascending ).find( value ) != 0) &&
	   ((ib->key( column, ascending ) > value) ||
	    (ib->key( column, ascending ).find( value ) == 0))) {
      debug( ib->key( column, ascending ));
      citem = ib;
      ib = ib->itemBelow();
    }    
  }


  clearSelection();
  setSelected( citem, true );
  setCurrentItem( citem );
  ensureItemVisible( citem );
}

void PabListView::setSorting( int column )
{ 
  this->column = column;
  this->ascending = true;

  if (oldColumn != -1)
    setColumnText( oldColumn, QIconSet( QPixmap()), columnText( oldColumn ));
  oldColumn = column;
  setColumnText( column, *up, columnText( column ));
  QListView::setSorting( column, ascending );
}

void PabListView::setSorting( int column, bool ascending )
{
  this->column = column;
  this->ascending = ascending;

  QString a,b = "descending";
  a.setNum( column );
  if (ascending)
    b = "ascending";
  debug( "PabWidget::setSorting" + a + " " + b );

  if (oldColumn != -1)
    setColumnText( oldColumn, QIconSet( QPixmap()), columnText( oldColumn ));
  oldColumn = column;
  if (ascending)
    setColumnText( column, *up, columnText( column ));
  else
    setColumnText( column, *down, columnText( column ));
  QListView::setSorting( column, ascending );

  if (column != -1)
    pabWidget->cbField->setCurrentItem( column );    
}

void PabListView::resort()
{
  int col = column;
  setSorting( -1, ascending );
  setSorting( col, ascending );
}

bool PabListView::tooltips()
{
  return tooltips_;
}





