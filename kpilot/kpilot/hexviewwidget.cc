/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999  Espen Sand, espensa@online.no
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <iostream>

#include <qclipboard.h>
#include <qdrawutil.h>


#include <kglobalsettings.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <kurldrag.h>

#include "hexdrag.h"
#include "hexerror.h"
#include "hexviewwidget.h"

//
// The usage of the WNorthWestGravity flag should make screen update
// faster (less paintEvents). I have had to add update() some places
// to ensure proper redrawing. Undefine if something goes wrong
// (i.e. not updated )
//
#define USE_NORTHWEST_GRAVITY 1

//
// I don't want to decide what is best: Drag starts when mouse is moved or
// after a timeout.
//
#define USE_DRAG_MOVEMENT 1






CDragManager::CDragManager( void )
{
  mActivateMode = Movement;
  mPending = false;
  mTimerId = 0;
}

void CDragManager::setActivateMode( EDragActivateMode mode )
{
  clear();
  mActivateMode = mode; // Movement or Timer
}

void CDragManager::setup( int x, int y )
{
  if( mActivateMode == Movement )
  {
    mOrigin.setX(x);
    mOrigin.setY(y);
  }
  else
  {
    setupTimer();
  }
  mPending = true;
}

bool CDragManager::start( QMouseEvent *e )
{
  if( mPending == false )
  {
    return( false );
  }

  if( mActivateMode == Movement )
  {
    if( (mOrigin - e->pos()).manhattanLength() > KGlobalSettings::dndEventDelay() )
    {
      mPending = false;
      emit startDrag( e->state() & ShiftButton ? true : false );
    }
    return( true );
  }
  else // Timer
  {
    if( mTimerId != 0 )
    {
      removeTimer();
      mPending = false;
      emit startDrag( e->state() & ShiftButton ? true : false );
      return( true );
    }
    else
    {
      // Should never happen!
      mPending = false;
      return( false );
    }
  }

}

bool CDragManager::clear( void )
{
  if( mPending == false )
  {
    return( false );
  }

  if( mActivateMode == Timer )
  {
    removeTimer();
  }

  mPending = false;
  return( true );
}

void CDragManager::timerEvent( QTimerEvent *e )
{
  if( e->timerId() == mTimerId )
  {
    removeTimer();
    if( mPending == true )
    {
      mPending = false;

      Window root, w;
      uint mask;
      int i;

      root = RootWindow( x11Display(), x11Screen() );
      XQueryPointer( x11Display(), root, &w, &w, &i, &i, &i, &i, &mask );
      emit startDrag( mask & ShiftMask ? true : false );
    }
  }
}

void CDragManager::removeTimer( void )
{
  if( mTimerId != 0 )
  {
    killTimer( mTimerId );
    mTimerId = 0;
  }
}

void CDragManager::setupTimer( void )
{
  if( mTimerId != 0 )
  {
    killTimer( mTimerId );
    mTimerId = 0;
  }
  mTimerId = startTimer( 500 );
}


//
// This widget will use the entire space of the parent widget
//
CHexViewWidget::CHexViewWidget( QWidget *parent, const char *name,
				CHexBuffer *hexBuffer )
  : QFrame( parent, name,
	    #ifdef USE_NORTHWEST_GRAVITY
	    Qt::WStaticContents
	    #else
	    0
	    #endif
	    ), mScrollBarSize( 16 )
{
  if( parent == 0 || hexBuffer == 0 ) { return; }

  //
  // Qt 2.0:
  // -------
  // I use the "CScrollBar" because sometimes (very seldom) when I
  // do a mHorzScroll->hide() the mHorzScroll->isVisible() remains true
  // in updateView() for a short while. I need the correct visibility
  // because I have to redraw the area - due to the "WNorthWestGravity" usage.
  //
  // I tried to do a
  // "while( mHorzScroll->isVisible() ) { mHorzScroll->hide(); }"
  // but then the loop never ended. The "CScrollBar" emits a "hidden()"
  // signal whenever is receives a QHideEvent.
  //

  mVertScroll = new CScrollBar( QScrollBar::Vertical, this );
  if( mVertScroll == 0 ) { return; }
  mHorzScroll = new CScrollBar( QScrollBar::Horizontal, this );
  if( mHorzScroll == 0 ) { return; }
  mCorner = new QWidget( this );
  if( mCorner == 0 ) { return; }
  connect( mHorzScroll, SIGNAL(valueChanged(int)), SLOT(changeXPos(int)) );
  connect( mVertScroll, SIGNAL(valueChanged(int)), SLOT(changeYPos(int)) );
  connect( mHorzScroll, SIGNAL(hidden()), SLOT(update()) );
  connect( mVertScroll, SIGNAL(hidden()), SLOT(update()) );
  mHorzScroll->hide();
  mVertScroll->hide();

  mDragManager = new CDragManager();
  if( mDragManager == 0 ) { return; }
  #ifdef USE_DRAG_MOVEMENT
  mDragManager->setActivateMode( CDragManager::Movement );
  #else
  mDragManager->setActivateMode( CDragManager::Timer );
  #endif
  connect( mDragManager, SIGNAL(startDrag(bool)), SLOT(startDrag(bool)) );

  setFrameStyle( QFrame::WinPanel|QFrame::Sunken );
  setWFlags( WResizeNoErase );
  setFocusPolicy( StrongFocus );

  mHexBuffer = hexBuffer;
  mHexBuffer->cursorReset();

  mEditMode      = mHexBuffer->editMode();
  mShowCursor    = false;
  mCursorTimerId = 0;

  mDocumentMenu  = 0;

  setTextBufferSize(); // Make sure there is a pixmap buffer
  setStartX(0);
  setStartY(0);

  setAcceptDrops(true);
  setDropHighlight(false); // Init state + frame shape
  setBackgroundColor( mHexBuffer->backgroundColor() );
}


CHexViewWidget::~CHexViewWidget( void )
{
  delete mVertScroll;
  delete mHorzScroll;
  delete mCorner;
  delete mDragManager;
}


int CHexViewWidget::readFile( QFile &file, const QString &url, CProgress &p )
{
  int errCode = mHexBuffer->readFile( file, url, p );
  if( errCode != Err_Success )
  {
    return( errCode );
  }

  initFile();
  return( Err_Success );
}


int CHexViewWidget::insertFile( QFile &file, CProgress &p )
{
  int errCode = mHexBuffer->insertFile( file, p );
  if( errCode != Err_Success )
  {
    return( errCode );
  }

  updateWindow( true, true );

  emit dataChanged();
  emit cursorChanged( mHexBuffer->cursorState() );
  emit layoutChanged( mLayout );
  return( Err_Success );
}


int CHexViewWidget::newFile( const QString &url )
{
  int errCode = mHexBuffer->newFile( url );
  if( errCode != Err_Success )
  {
    return( errCode );
  }

  initFile();
  return( Err_Success );
}


int CHexViewWidget::writeFile( QFile &file, CProgress &p )
{
  int errCode = mHexBuffer->writeFile( file, p );
  if( errCode == Err_Success )
  {
    emit fileState( mHexBuffer->fileState() );
  }
  return( errCode );
}


void CHexViewWidget::closeFile( void )
{
  emit fileClosed( mHexBuffer->url() );
  mHexBuffer->closeFile();
  initFile();
}



void CHexViewWidget::initFile( void )
{
  setStartX(0);
  setStartY(0);

  mHexBuffer->cursorReset();
  mHexBuffer->setLayout( mLayout );
  mHexBuffer->setFont( mFontInfo );
  setEditMode( mEditMode );
  setColor( mColor, false );
  setCursor( mCursor, false );
  setMisc( mMisc );

  setBackgroundColor( mHexBuffer->backgroundColor() );
  setBackgroundMode( NoBackground );

  updateView( true, false );
  resizeEvent( 0 );

  emit dataChanged();
  emit cursorChanged( mHexBuffer->cursorState() );
  emit fileState( mHexBuffer->fileState() );
  emit encodingChanged( mHexBuffer->encoding() );
  emit fileName( mHexBuffer->url(), mHexBuffer->hasFileName() );
  emit bookmarkChanged( mHexBuffer->bookmarkList() );
}


void CHexViewWidget::setBuffer( CHexBuffer *hexBuffer )
{
  if( hexBuffer == 0 || mHexBuffer == hexBuffer )
  {
    return;
  }

  unselect();
  unmark();

  mHexBuffer = hexBuffer;
  mHexBuffer->setLayout( mLayout );
  mHexBuffer->setFont( mFontInfo );
  setEditMode( mEditMode );
  setColor( mColor, false );
  setCursor( mCursor, false );
  setMisc( mMisc );

  if( mLayout.lockLine == false )
  {
    mHexBuffer->matchWidth( width() );
  }

  setBackgroundColor( hexBuffer->backgroundColor() );
  setBackgroundMode( NoBackground );

  setEditMode( mEditMode );
  updateWindow();

  emit dataChanged();
  emit cursorChanged( mHexBuffer->cursorState() );
  emit fileState( mHexBuffer->fileState() );
  emit encodingChanged( mHexBuffer->encoding() );
  emit layoutChanged( mLayout );
  emit inputModeChanged( mHexBuffer->inputMode() );
  emit fileName( mHexBuffer->url(), mHexBuffer->hasFileName() );
  emit bookmarkChanged( mHexBuffer->bookmarkList() );
}




void CHexViewWidget::changeXPos( int p )
{
  int dx = startX() - p;
  setStartX(p);


  if( QABS(dx) < width() )
  {
    scroll( dx, 0, contentsRect() );
  }
  else
  {
    QWidget::update();
  }

  //
  // If the start position has become 0, then update the view. This
  // will remove the scrollbar (if it is visible) if the textarea width
  // is wider than the text. The scrollbar will then disappear under the
  // mouse pointer.
  //
  if( startX() == 0 )
  {
    updateView( false, false );
  }

}


void CHexViewWidget::changeYPos( int p )
{
  int dy  = startY() - p;
  setStartY(p);

  if( QABS( dy ) < height() )
  {
    scroll( 0, dy, contentsRect() );
  }
  else
  {
    QWidget::update();
  }

  //
  // If the start position has become 0, then update the view. This
  // will remove the scrollbar (if it is visible) if the textarea height
  // is taller than the text. The scrollbar will then disappear under the
  // mouse pointer.
  //
  if( startY() == 0 )
  {
    updateView( false, false );
  }
}


void CHexViewWidget::clipboardChanged( void )
{
  disconnect(QApplication::clipboard(),SIGNAL(dataChanged()),
	     this,SLOT(clipboardChanged()));
  unselect();
}

void CHexViewWidget::paletteChanged( void )
{
  setColor( mColor, true );
}


void CHexViewWidget::fontChanged( void )
{
  //setFont( kapp->fixedFont, true );
}

void CHexViewWidget::filter( SFilterControl &fc )
{
  int errCode = mHexBuffer->filter( fc );
  if( errCode == Err_Success )
  {
    repaint();
    emit dataChanged();
    emit cursorChanged( mHexBuffer->cursorState() );
  }
}



void CHexViewWidget::insert( SInsertData &id )
{
  if( id.onCursor == false )
  {
    mHexBuffer->cursorGoto( id.offset, 7 );
  }
  SCursorConfig cc;
  updateCursor( cc, true );

  if( id.size == 0 )
  {
    return;
  }

  QByteArray buf( id.size );
  if( buf.isNull() == true )
  {
    return;
  }
  buf.fill( 0 );

  if( id.pattern.size() > 0 )
  {
    uint size = id.pattern.size()>buf.size() ? buf.size() : id.pattern.size();
    if( id.repeatPattern == false )
    {
      memcpy( &buf[0], &id.pattern[0], size );
      if( size < buf.size() )
      {
	memset( &buf[size], id.pattern[id.pattern.size()-1], buf.size()-size );
      }
    }
    else
    {
      for( uint i=0; i < buf.size(); i+= size )
      {
	uint s = i+size > buf.size() ? buf.size()-i : size;
	memcpy( &buf[i], &id.pattern[0], s );
      }
    }
  }

  insert( buf );
}


void CHexViewWidget::insert( const QByteArray &buf )
{
  if( mHexBuffer->documentPresent() == false )
  {
    emit pleaseOpenNewFile();
    if( mHexBuffer->documentPresent() == false )
    {
      return;
    }
  }

  uint offset = mHexBuffer->cursorOffset();
  int errCode = mHexBuffer->inputAtCursor( buf, 0 );
  if( errCode == Err_Success  )
  {
    updateWindow( offset, true );
    emit dataChanged();
  }
}


void CHexViewWidget::append( const QByteArray &buf )
{
  if( mHexBuffer->documentPresent() == false )
  {
    insert( buf );
  }
  else
  {
    SCursorConfig cc;
    cc.emulateControlButton( true );
    cursorEnd( cc );

    int errCode = mHexBuffer->inputAtCursor( buf, 0 );
    if( errCode == Err_Success  )
    {
      updateWindow( true, true );
      emit dataChanged();
    }
  }
}


void CHexViewWidget::valueOnCursor( QByteArray &buf, uint size )
{
  mHexBuffer->valueOnCursor( buf, size );
}



void CHexViewWidget::updateView( bool redraw, bool fixCursor )
{
  int f2 = frameWidth() * 2;
  int scrollBarCount = 0; // Number of visible scrollbars
  int editWidth = 0;
  int editHeight = 0;

  for( uint i=0; i < 2; i++ )
  {
    editWidth  = width()  - f2; // Total available width
    editHeight = height() - f2; // Total available height
    int textWidth  = dataWidth();
    int textHeight = mHexBuffer->totalHeight();

    //
    // This will move the start position of the horizontal scrollbar
    // to the left (if possible) if the text width is smaller than the
    // edit width.
    //
    if( startX() > 0 )
    {
      int size = mVertScroll->isVisible() == true ? mScrollBarSize : 0;
      if( startX() + editWidth - size > textWidth )
      {
	int position = textWidth - editWidth + size;
	setStartX( position > 0 ? position : 0 );
	#ifdef USE_NORTHWEST_GRAVITY
	redraw = true;
	#endif
      }
    }

    int tooMuchX = textWidth - editWidth;
    bool horzScrollbarVisible = startX() > 0 || tooMuchX > 0 ? true : false;
    if( horzScrollbarVisible == true )
    {
      editHeight -= mScrollBarSize;
    }


    //
    // This will move the start position of the vertical scrollbar
    // to the top (if possible) if the text height is smaller than the
    // edit height.
    //
    if( startY() > 0 )
    {
      if( startY() + editHeight > textHeight )
      {
	int position = textHeight - editHeight;
	setStartY( position > 0 ? position : 0 );
	#ifdef USE_NORTHWEST_GRAVITY
	redraw = true;
	#endif
      }
    }

    int tooMuchY  = textHeight - editHeight;
    int startLine = startY() / textHeight;

    if( startLine > 0 || tooMuchY > 0 )
    {
      editWidth -= mScrollBarSize;
      tooMuchX += mScrollBarSize;
      if( horzScrollbarVisible == false && tooMuchX > 0 )
      {
	// Horizontal scrollbar will be visible after all.
	editHeight -= mScrollBarSize;
	tooMuchY += mScrollBarSize;
      }
    }

    if( tooMuchX < startX() ) { tooMuchX = startX(); }
    if( tooMuchY < startY() ) { tooMuchY = startY(); }

    scrollBarCount = 0;
    if( tooMuchX > 0 && documentPresent() == true )
    {
      mHorzScroll->blockSignals( true );
      mHorzScroll->setGeometry( 0, editHeight+f2, editWidth+f2,mScrollBarSize);
      mHorzScroll->setRange( 0, tooMuchX );
      mHorzScroll->setValue( startX() );
      mHorzScroll->setSteps(mHexBuffer->lineHeight(),editWidth-mScrollBarSize);
      mHorzScroll->blockSignals( false );
      if( mHorzScroll->isVisible() == false ) { mHorzScroll->show(); }
      scrollBarCount ++;
    }
    else
    {
      if( mHorzScroll->isVisible() == true ) { mHorzScroll->hide(); }
    }

    if( tooMuchY > 0 && documentPresent() == true )
    {
      mVertScroll->blockSignals( true );
      mVertScroll->setGeometry( editWidth+f2, 0, mScrollBarSize,editHeight+f2);
      mVertScroll->setRange( 0, tooMuchY );
      mVertScroll->setValue( startY() );
      mVertScroll->setSteps(mHexBuffer->lineHeight(),
			    editHeight-mScrollBarSize );
      mVertScroll->blockSignals( false );
      if( mVertScroll->isVisible() == false ) { mVertScroll->show(); }
      scrollBarCount ++;
    }
    else
    {
      if( mVertScroll->isVisible() == true ) { mVertScroll->hide(); }
    }

    if( fixCursor == true )
    {
      int position = mHexBuffer->cursorFixedPosition( startY(), height() );
      if( position != startY() )
      {
	setStartY( position );
	fixCursor = false;
	continue;
      }
    }
    break;
  }


  if( scrollBarCount == 2 )
  {
    mCorner->setGeometry( editWidth+f2, editHeight+f2, mScrollBarSize,
			  mScrollBarSize );
    mCorner->show();
  }
  else
  {
    mCorner->hide();
  }


  updateFrameSize();

  if( redraw == true )
  {
    QWidget::update();
  }
}


void CHexViewWidget::setPalette( const QPalette &p )
{
  QWidget::setPalette( p );
  mCorner->setPalette( p );
  mVertScroll->setPalette( p );
  mHorzScroll->setPalette( p );
}


void CHexViewWidget::setLayout( SDisplayLayout &layout )
{
  mLayout = layout;
  mHexBuffer->setLayout( mLayout );
  updateWindow();

  emit layoutChanged( mLayout );
  emit cursorChanged( mHexBuffer->cursorState() );
  emit textWidth( defaultWidth() );
};


void CHexViewWidget::setInputMode( SDisplayInputMode &input )
{
  mHexBuffer->setInputMode( input );
  emit inputModeChanged( mHexBuffer->inputMode() );
}


void CHexViewWidget::setCursor( const SDisplayCursor &cursor,
                                bool /*updateDisplay*/ )
{
  mCursor = cursor;
  mHexBuffer->setCursorShapeModifier( cursor.alwaysBlockShape,
				      cursor.thickInsertShape );
  setupCursorTimer();
  redrawFromOffset( mHexBuffer->cursorOffset(), false );
}

void CHexViewWidget::setColor( const SDisplayColor &color,
			       bool updateDisplay )
{
  mColor = color;
  mHexBuffer->setColor( mColor );
  if( updateDisplay == true )
  {
    repaint();
  }
}

void CHexViewWidget::setFont( const SDisplayFontInfo &fontInfo,
			      bool updateDisplay )
{
  mFontInfo = fontInfo;
  mHexBuffer->setFont( mFontInfo );
  emit textWidth( defaultWidth() );
  if( updateDisplay == true )
  {
    updateWindow();
  }
}



void CHexViewWidget::setMisc( SDisplayMisc &misc )
{
  mMisc = misc;
  mHexBuffer->setUndoLevel( misc.undoLevel );
  mHexBuffer->setSoundState( misc.inputSound, misc.fatalSound );
  mHexBuffer->setBookmarkVisibility( misc.bookmarkOffsetColumn,
				     misc.bookmarkEditor );
  if( mHexBuffer->documentPresent() == true )
  {
    QWidget::update();
  }
}


void CHexViewWidget::setInsertMode( bool insertMode )
{
  setEditMode( insertMode == true ? CHexBuffer::EditInsert :
	       CHexBuffer::EditReplace );
}


int CHexViewWidget::setEncoding( CConversion::EMode mode, CProgress &p )
{
  int errCode = mHexBuffer->setEncoding( mode, p );
  if( errCode == Err_Success )
  {
    repaint();
    emit cursorChanged( mHexBuffer->cursorState() );
    emit encodingChanged( mHexBuffer->encoding() );
  }
  return( errCode );
}


void CHexViewWidget::reportEncoding( void )
{
  emit encodingChanged( mHexBuffer->encoding() );
}


void CHexViewWidget::selectAll( void )
{
  setSelection( 0, true );
  setSelection( mHexBuffer->documentSize(), false );
  autoCopy();
  emit cursorChanged( mHexBuffer->cursorState() );
}

void CHexViewWidget::unselect( void )
{
  setSelection( 0, true );
  emit cursorChanged( mHexBuffer->cursorState() );
}

void CHexViewWidget::unmark( void )
{
  setMark( 0, 0, false );
}


int CHexViewWidget::findFirst( SSearchControl &sc )
{
  int errCode = mHexBuffer->findFirst( sc );
  if( errCode == Err_Success ) { updateWindow( true, false ); }
  return( errCode );
}

int CHexViewWidget::findNext( SSearchControl &sc )
{
  int errCode = mHexBuffer->findNext( sc );
  if( errCode == Err_Success ) { updateWindow( true, false ); }
  return( errCode );
}

int CHexViewWidget::findWrap( SSearchControl &sc )
{
  int errCode = mHexBuffer->findWrap( sc );
  if( errCode == Err_Success ) { updateWindow( true, false ); }
  return( errCode );
}

int CHexViewWidget::replaceAll( SSearchControl &sc, bool init )
{
  int errCode = mHexBuffer->replaceAll( sc, init );
  if( errCode == Err_Success )
  {
    updateWindow( true, false );
    emit dataChanged();
  }
  return( errCode );
}

int CHexViewWidget::replaceMarked( SSearchControl &sc )
{
  int errCode = mHexBuffer->replaceMarked( sc );
  if( errCode == Err_Success )
  {
    updateWindow( true, false );
    emit dataChanged();
  }
  return( errCode );
}

int CHexViewWidget::collectStrings( CStringCollectControl &sc )
{
  int errCode = mHexBuffer->collectStrings( sc );
  return( errCode );
}


int CHexViewWidget::collectStatistic( SStatisticControl &sc, CProgress &p )
{
  int errCode = mHexBuffer->collectStatistic( sc, p );
  return( errCode );
}


void CHexViewWidget::gotoOffset( uint offset, uint bit, bool fromCursor,
				 bool forward )
{
  bool reverse = forward == true ? false : true;
  mHexBuffer->cursorGoto( offset, bit, reverse, fromCursor );
  updateWindow( true, false );
}


void CHexViewWidget::gotoOffset( uint offset )
{
  gotoOffset( offset, 7, true, true );
}


int CHexViewWidget::print( CHexPrinter &printer, CProgress &p )
{
  return( mHexBuffer->print( printer, p ) );
}


uint CHexViewWidget::numPage( CHexPrinter &printer )
{
  return( mHexBuffer->numPage( printer ) );
}


int CHexViewWidget::exportText( const SExportText &ex, CProgress &p )
{
  return( mHexBuffer->exportText( ex, p ) );
}


int CHexViewWidget::exportHtml( const SExportHtml &ex, CProgress &p )
{
  return( mHexBuffer->exportHtml( ex, p ) );
}


int CHexViewWidget::exportCArray( const SExportCArray &ex, CProgress &p )
{
  return( mHexBuffer->exportCArray( ex, p ) );
}


void CHexViewWidget::startDrag( bool asText )
{
  QByteArray buf;
  if( asText == true )
  {
    if( mHexBuffer->copySelectedText( buf ) != Err_Success )
    {
      return;
    }
    QDragObject *d = new QTextDrag( buf.data(), this );
    d->dragCopy();
  }
  else
  {
    if( mHexBuffer->copySelectedData( buf ) != Err_Success )
    {
      return;
    }
    QDragObject *d = new CHexDrag( buf, this );
    d->dragCopy();
  }
}



void CHexViewWidget::copy( void )
{
  QByteArray buf;
  if( mHexBuffer->copySelectedData( buf ) != Err_Success )
  {
    return;
  }
  disconnect(QApplication::clipboard(),SIGNAL(dataChanged()),
	     this,SLOT(clipboardChanged()));
  //
  // Note: Do no give the CHexDrag a parent != 0. The clipboard
  // owns the current dragdata and will destroy it on exit or
  // when it receives a new object. If the CHexDrag has a parent
  // != 0, the CHexDrag object will be destroyed when the parent
  // is destroyed. We will then have a double destroy situation
  // when the app. is closed (=> segfault).
  //
  QApplication::clipboard()->setData(new CHexDrag( buf ));
  connect(QApplication::clipboard(),SIGNAL(dataChanged()),
	  this,SLOT(clipboardChanged()));
}


void CHexViewWidget::copyText( int columnSegment )
{
  QByteArray buf;
  if( mHexBuffer->copySelectedText( buf, columnSegment ) != Err_Success )
  {
    return;
  }

  disconnect(QApplication::clipboard(),SIGNAL(dataChanged()),
	     this,SLOT(clipboardChanged()));
  QApplication::clipboard()->setText( buf.data() );
  connect(QApplication::clipboard(),SIGNAL(dataChanged()),
	  this,SLOT(clipboardChanged()));
}



void CHexViewWidget::paste( void )
{
  QMimeSource *data = QApplication::clipboard()->data();
  if( data != 0 )
  {
    QByteArray buf;
    if( CHexDrag::decode( data, buf ) == true )
    {
      insert( buf );
      return;
    }

    QString text;
    if( QTextDrag::decode( data, text ) == true )
    {
      QByteArray buf;
      if( mClipConvert.decode( buf, text ) == true )
      {
	insert( buf );
      }
      return;
    }
  }

}


void CHexViewWidget::cut( void )
{
  copy(); // Always make a copy to the clipboard of what we remove.
  bool success = mHexBuffer->cutSelection();
  if( success == false )
  {
    return;
  }

  updateWindow( false, true );
  emit dataChanged();
}


void CHexViewWidget::undo( void )
{
  bool success = mHexBuffer->undo();
  if( success == false )
  {
    return;
  }

  updateWindow( true, true );
  emit dataChanged();
}


void CHexViewWidget::redo( void )
{
  bool success = mHexBuffer->redo();
  if( success == false )
  {
    return;
  }

  updateWindow( true, true );
  emit dataChanged();
}


void CHexViewWidget::addBookmark( int position )
{
  int errCode = mHexBuffer->addBookmark( position );
  if( errCode != Err_Success )
  {
    if( errCode == Err_ListFull )
    {
      replaceBookmark();
    }
    return;
  }
  redrawFromOffset( mHexBuffer->cursorOffset(), false );

  emit bookmarkChanged( mHexBuffer->bookmarkList() );
}



int CHexViewWidget::bookmarkMenu( const QString &title )
{
  QPtrList<SCursorOffset> &list = mHexBuffer->bookmarkList();
  if( list.count() == 0 )
  {
    return( -1 );
  }

  QString text;
  KPopupMenu *popup = new KPopupMenu( title, 0 );
  for( uint i=0; i < list.count(); i++ )
  {
    const SCursorOffset *p = list.at( i );
    if( p == 0 ) { continue; }

    text.sprintf("%04X:%04X", p->offset>>16, p->offset&0x0000FFFF );
    text.prepend( QString("[%1] %2: ").arg(i+1).arg(i18n("Offset")) );
    popup->insertItem( text, i );
  }

  QSize s(popup->sizeHint());
  QPoint center( (width()-s.width())/2, (height()-s.height())/2 );
  int position = popup->exec( mapToGlobal(center) );
  delete popup;

  return( position );
}


void CHexViewWidget::removeBookmark( bool all )
{
  if( all == true )
  {
    bool success = mHexBuffer->removeBookmark( -1 );
    if( success == false )
    {
      return;
    }
    QWidget::update(); // Redraw visisble area.
  }
  else
  {
    int position = bookmarkMenu( i18n("Remove Bookmark") );
    if( position < 0 )
    {
      return;
    }

    const SCursorOffset *p = mHexBuffer->bookmarkList().at(position);
    uint offset = p ? p->offset : 0;

    bool success = mHexBuffer->removeBookmark( position );
    if( success == false )
    {
      return;
    }

    redrawFromOffset( offset, false );
  }

  emit bookmarkChanged( mHexBuffer->bookmarkList() );
}


void CHexViewWidget::replaceBookmark( void )
{
  QPtrList<SCursorOffset> &list = mHexBuffer->bookmarkList();
  if( list.count() == 0 )
  {
    return;
  }

  int position = bookmarkMenu( i18n("Replace Bookmark") );
  if( position < 0 )
  {
    return;
  }
  addBookmark( position );
}


void CHexViewWidget::gotoBookmark( uint position )
{
  QPtrList<SCursorOffset> &list = mHexBuffer->bookmarkList();
  if( position >= list.count() )
  {
    return;
  }

  SCursorOffset *p = list.at( position );
  if( p == 0 )
  {
    return;
  }

  mHexBuffer->cursorGoto( p->offset, p->bit );
  updateWindow();
}



void CHexViewWidget::gotoNextBookmark( bool next )
{
  QPtrList<SCursorOffset> &list = mHexBuffer->bookmarkList();
  uint offset = mHexBuffer->cursorOffset();
  uint diff   = ~0;

  SCursorOffset *match = 0;

  //
  // Note: the list is unsorted.
  //
  if( next == true )
  {
    for( SCursorOffset *co = list.first(); co != 0; co = list.next() )
    {
      if( co->offset > offset )
      {
	if( co->offset-offset < diff )
	{
	  diff = co->offset-offset;
	  match = co;
	}
      }
    }
  }
  else
  {
    for( SCursorOffset *co = list.first(); co != 0; co = list.next() )
    {
      if( co->offset < offset )
      {
	if( offset-co->offset < diff )
	{
	  diff = offset-co->offset;
	  match = co;
	}
      }
    }
  }


  if( match == 0 )
  {
    if( next == true )
    {
      // Wrap: Locate entry with smallest offset.
      offset = ~0;
      for( SCursorOffset *co = list.first(); co != 0; co = list.next() )
      {
	if( co->offset < offset )
	{
	  offset = co->offset;
	  match  = co;
	}
      }
    }
    else
    {
      // Wrap: Locate entry with largest offset.
      offset=0;
      for( SCursorOffset *co = list.first(); co != 0; co = list.next() )
      {
	if( co->offset > offset )
	{
	  offset = co->offset;
	  match  = co;
	}
      }
    }
  }

  if( match != 0 )
  {
    mHexBuffer->cursorGoto( match->offset, match->bit );
    updateWindow();
  }
}






//
// Used to test the speed of drawing
//
#include <sys/time.h>
#include <unistd.h>
void CHexViewWidget::benchmark( void )
{
  struct timeval t1, t2;
  uint loop = 10;

  gettimeofday( &t1, 0 );
  for( uint i=0; i< loop; i++ )
  {
    paintText( contentsRect(), false );
  }
  gettimeofday( &t2, 0 );


  uint area  = width() * height();
  uint last = (t2.tv_sec-t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);

  kdDebug() << "Duration: " << (float)last/ 1000000.0 << endl;
  kdDebug() << "Duration/loop: " << (float)last/ (1000000.0*(float)loop) << endl;
  kdDebug() << "Area: " << area << endl;
  kdDebug() << "Loop: " << loop << endl;
}





void CHexViewWidget::resizeEvent( QResizeEvent * )
{
  setTextBufferSize();

  if( mLayout.lockLine == true )
  {
    updateView( false, false );
    #ifdef USE_NORTHWEST_GRAVITY
    paintFrame();
    #endif
  }
  else
  {
    bool state = mVertScroll->isVisible();
    int size = (state == true ? mScrollBarSize : 0) + frameWidth()*2;
    #ifdef USE_NORTHWEST_GRAVITY
    int w = dataWidth();
    #endif

    bool bufferChanged = mHexBuffer->matchWidth( width() - size );
    updateView( false, bufferChanged );

    if( mVertScroll->isVisible() != state )
    {
      size = (mVertScroll->isVisible() ? mScrollBarSize : 0) + frameWidth()*2;
      bufferChanged = mHexBuffer->matchWidth( width() - size );
      updateView( false, bufferChanged );
    }

    #ifdef USE_NORTHWEST_GRAVITY
    if( w != dataWidth() )
    {
      QWidget::update();
    }
    else
    {
      paintFrame();
    }
    #endif
  }
}



void CHexViewWidget::paintEvent( QPaintEvent *e )
{
  paintText( e->rect(), true );
}



void CHexViewWidget::updateFrameSize( void )
{
  int w = width() - (mVertScroll->isVisible() ? mScrollBarSize : 0);
  if( w < 0 ) { w = 0; }
  int h = height() - (mHorzScroll->isVisible() ? mScrollBarSize : 0);
  if( h < 0 ) { h = 0; }

  setFrameRect( QRect(0,0,w,h) );
}



void CHexViewWidget::paintFrame( void )
{
  QPainter paint;
  paint.begin( this );
  drawFrame( &paint );
  paint.end();
}



void CHexViewWidget::drawFrame( QPainter *p )
{
  //
  // 2000-01-10 Espen Sand
  // I want to display the frame with a custom color whenever the widget
  // accepts a drop. The setPalette() function causes quite a bit of flicker
  // in the scrollbars (even when PropagationMode is NoChildren), so I
  // draw the frame manually when it can accept a drop. Note that the
  // code below is for the frame shape "QFrame::WinPanel|QFrame::Plain"
  //
  if( mDropHighlight == true )
  {
    qDrawPlainRect( p, frameRect(), QColor("SteelBlue2"), lineWidth() );
  }
  else
  {
    QFrame::drawFrame(p); // Use standard drawFrame
  }
}



void CHexViewWidget::keyPressEvent( QKeyEvent *e )
{
  SCursorConfig cc;
  cc.state = e->state();

  //
  // Some special actions that we have to trap here
  //
  if( e->state() & ControlButton )
  {
    switch( e->key() )
    {
      case Key_Space:
	e->accept();
	toggleEditor();
	return;
      break;

      case Key_1:
	e->accept();
	cursorStep( cc, 1 );
	return;
      break;

      case Key_2:
	e->accept();
	cursorStep( cc, 2 );
	return;
      break;

      case Key_4:
	e->accept();
	cursorStep( cc, 4 );
	return;
      break;

      case Key_8:
	e->accept();
	cursorStep( cc, 8 );
	return;
      break;
    }
  }

  if( e->state() & AltButton )
  {
    if( e->key() == Key_Left || e->key() == Key_Right )
    {
      emit pleaseStepFile( e->key() == Key_Left ? true : false );
      e->accept();
    }
    else if( e->key() == Key_Up || e->key() == Key_Down )
    {
      gotoNextBookmark( e->key() == Key_Down ? true : false );
      e->accept();
    }
    else
    {
      e->ignore();
    }
    return;
  }

  switch ( e->key() )
  {
    case Key_Left:
      cursorLeft( cc );
    break;

    case Key_Right:
      cursorRight( cc );
    break;

    case Key_Up:
      cursorUp( cc );
    break;

    case Key_Down:
      cursorDown( cc );
    break;

    case Key_Home:
      cursorHome( cc );
    break;

    case Key_End:
      cursorEnd( cc );
    break;

    case Key_Next:
      cursorPageDown( cc );
    break;

    case Key_Prior:
      cursorPageUp( cc );
    break;

    case Key_Insert:
      cursorInsert( cc );
    break;

    case Key_Delete:
      cursorDelete( cc );
    break;

    case Key_Backspace:
      cursorBackspace( cc );
    break;

    default:
      if( (e->text()[0]).isPrint() == true )
      {
	cursorInput( e->text()[0] );
      }
    break;
  }

  e->accept();
}


void CHexViewWidget::keyReleaseEvent( QKeyEvent *e )
{
  if( e->state() & ShiftButton && shiftButtonState() == false )
  {
    //
    // The shift button was pressed when event was triggered, but is
    // now released. I use this as a sign to copy selected data to the
    // clipboard. The shiftButtonState() uses X-lib functions so have a
    // look in the headerfile (inlined method) if you want to remove
    // this dependency.
    //
    autoCopy();
  }
}


void CHexViewWidget::mousePressEvent( QMouseEvent *e )
{
  //
  // The RMB popup menu is managed by the KContextMenuManager
  //

  if( e->button() == LeftButton )
  {
    if( e->state() & ControlButton )
    {
      if( KContextMenuManager::showOnButtonPress() == true
	  && mDocumentMenu != 0 )
      {
	mDocumentMenu->popup( e->globalPos() );
      }
    }
    else
    {
      bool cellLevel = mMisc.cursorJump == false;
      setCursorPosition( e->x(), e->y(), true, cellLevel );
    }
  }
  else if( e->button() == MidButton )
  {
    paste();
  }

}

void CHexViewWidget::mouseMoveEvent( QMouseEvent *e )
{
  if( e->state() & LeftButton )
  {
    if( mDragManager->start( e ) == false )
    {
      bool cellLevel = mMisc.cursorJump == false||e->state() & ControlButton;
      setCursorPosition( e->x(), e->y(), false, cellLevel );
    }
  }
}

void CHexViewWidget::mouseReleaseEvent( QMouseEvent *e )
{
  //
  // The RMB popup menu is managed by the KContextMenuManager
  //

  if( e->button() == LeftButton )
  {
    if( e->state() & ControlButton )
    {
      if( KContextMenuManager::showOnButtonPress() == false
	  && mDocumentMenu != 0 )
      {
	mDocumentMenu->popup( e->globalPos() );
      }
    }
    else
    {
      if( mDragManager->clear() == true )
      {
	// Remove any selection
	SCursorConfig cc;
	cc.setKeepSelection( false );
	updateCursor( cc, true );
      }
      else
      {
	mHexBuffer->cursorResetEditArea();
	autoCopy();
      }
    }
  }

}



void CHexViewWidget::wheelEvent( QWheelEvent *e )
{
  if( mVertScroll->isVisible() == true )
  {
    QApplication::sendEvent( mVertScroll, e );
  }
}


void CHexViewWidget::dragEnterEvent( QDragEnterEvent *e )
{
  if( QTextDrag::canDecode(e) || CHexDrag::canDecode(e) ||
      KURLDrag::canDecode(e))
  {
    e->accept();
    setDropHighlight( true );
  }
}


void CHexViewWidget::dragLeaveEvent( QDragLeaveEvent * )
{
  setDropHighlight( false );
}


void CHexViewWidget::dragMoveEvent( QDragMoveEvent *e )
{
  //
  // Move the cursor if we are dragging (readable) text or binary
  // data. Note: the QTextDrag::canDecode() will return true if we
  // are dragging a file so we have to test for KURLDrag::canDecode()
  // first.
  //

  if( KURLDrag::canDecode(e) == true )
  {
    return;
  }

  if( QTextDrag::canDecode(e) == true || CHexDrag::canDecode(e) == true )
  {
    int x = startX() + e->pos().x();
    int y = startY() + e->pos().y();
    if( mHexBuffer->setCursorPosition( x, y, false, false ) == true )
    {
      SCursorConfig cc;
      cc.setKeepSelection( true );
      updateCursor( cc, false, false );
    }
  }
}


void CHexViewWidget::dropEvent( QDropEvent *e )
{
  QMimeSource &m = *(QDropEvent*)e;
  setDropHighlight( false );

  KURL::List list;
  if( KURLDrag::decode( &m, list ) == true )
  {
    //
    // This widget can not itself open a file so it will simply pass
    // the request to a parent that can (hopefully) do this
    //
    for( KURL::List::ConstIterator it = list.begin(); it != list.end(); it++ )
    {
      emit pleaseOpenFile( (*it).url(), true, 0 );
    }
    return;
  }

  QByteArray buf;
  if( CHexDrag::decode( &m, buf ) == true )
  {
    insert( buf );
    return;
  }

  QString text;
  if( QTextDrag::decode( &m, text ) == true )
  {
    bool success = mClipConvert.decode( buf, text );
    if( success == true )
    {
      insert( buf );
    }
    return;
  }

}


void CHexViewWidget::showEvent( QShowEvent * )
{
  // Currently we do nothing here.
}




void CHexViewWidget::timerEvent( QTimerEvent *e )
{
  if( e->timerId() == mCursorTimerId )
  {
    if( hasFocus() == true )
    {
      if( mCursor.alwaysVisible == true )
      {
	mShowCursor = true;
      }
      else
      {
	mShowCursor = mShowCursor == true ? false : true;
      }
    }
    else if( mCursor.focusMode == SDisplayCursor::hide )
    {
      mShowCursor = false;
    }
    else if( mCursor.focusMode == SDisplayCursor::stopBlinking )
    {
      mShowCursor = true;
    }
    else
    {
      mShowCursor = mShowCursor == true ? false : true;
    }
    mHexBuffer->setShowCursor( mShowCursor );
    paintCursor( CHexBuffer::cursor_curr );
  }
}

void CHexViewWidget::focusInEvent( QFocusEvent * )
{
  setupCursorTimer();
  paintCursor( CHexBuffer::cursor_curr );
}

void CHexViewWidget::focusOutEvent( QFocusEvent * )
{
  if( mCursor.focusMode != SDisplayCursor::ignore )
  {
    setupCursorTimer();
    paintCursor( CHexBuffer::cursor_curr );
  }
}


void CHexViewWidget::setSelection( uint offset, bool init )
{
  bool selectionChanged = mHexBuffer->selectionSet( offset, init );
  if( selectionChanged == true )
  {
    uint off1, off2;
    mHexBuffer->selectionStartChange( off1, off2 );
    if( off1 != off2 )
    {
      redrawInterval( off1, off2 );
    }
    mHexBuffer->selectionStopChange( off1, off2 );
    if( off1 != off2 )
    {
      redrawInterval( off1, off2 );
    }
  }
  mHexBuffer->selectionSyncronize();
}



void CHexViewWidget::setMark( uint offset, uint size, bool moveCursor )
{
  bool changed;
  if( size == 0 )
  {
    changed = mHexBuffer->markRemove();
  }
  else
  {
    mHexBuffer->markSet( offset, size );
    if( moveCursor == true )
    {
      changed = false;
      gotoOffset( offset, 7, false, true );
    }
    else
    {
      changed = true;
    }
  }

  if( changed == true )
  {
    uint off1, off2;
    mHexBuffer->markStartChange( off1, off2 );
    if( off1 != off2 )
    {
      redrawInterval( off1, off2 );
    }

    mHexBuffer->markStopChange( off1, off2 );
    if( off1 != off2 )
    {
      redrawInterval( off1, off2 );
    }
  }
  mHexBuffer->markSyncronize();
}



void CHexViewWidget::setCursorPosition(int x, int y, bool init, bool cellLevel)
{
  x += startX();
  y += startY();

  if( mHexBuffer->setCursorPosition( x, y, init, cellLevel ) == false )
  {
    if( init == true )
    {
      unselect();
      unmark();
    }
  }
  else if( init == false )
  {
    SCursorConfig cc;
    cc.setKeepSelection( true );
    updateCursor( cc, false );
  }
  else
  {
    SCursorConfig cc;
    if( mHexBuffer->cursorInsideSelection() == true )
    {
      mDragManager->setup( x - startX(), y - startY() );
      cc.setKeepSelection( true );
      updateCursor( cc, true, false );
    }
    else
    {
      cc.setKeepSelection( false );
      updateCursor( cc, true );
    }
  }
}



void CHexViewWidget::redrawInterval( uint startOffset, uint stopOffset )
{
  //
  // Can be improved, I repaint the entire line even if the offsets
  // only specify one byte.
  //
  uint lineStart = mHexBuffer->calculateLine( startOffset );
  uint lineStop  = mHexBuffer->calculateLine( stopOffset );
  if( lineStart <= lineStop )
  {
    redrawLines( lineStart, lineStop - lineStart + 1 );
  }
  else
  {
    redrawLines( lineStop, lineStart - lineStop + 1 );
  }
}



void CHexViewWidget::redrawLines( uint docLine, int numLine )
{
  int lineHeight = mHexBuffer->lineHeight();
  int lineOffset = startY() / lineHeight;

  // FIXME: startY() should return uint
  if( (uint)lineOffset > docLine )
  {
    numLine -= (lineOffset-docLine);
    if( numLine <= 0 ) { return; }
    docLine = lineOffset;
  }

  int t = docLine * lineHeight - startY() + frameWidth();
  if( mEditMode == CHexBuffer::EditInsert )
  {
    QRect r = contentsRect();
    r.setTop( t );
    paintText( contentsRect().intersect( r ), false );
  }
  else
  {
    int h = (numLine + (startY() % lineHeight ? 1 : 0)) * lineHeight;
    QRect r( contentsRect().left(), t, contentsRect().width(), h );
    paintText( contentsRect().intersect( r ), false );
  }
}



void CHexViewWidget::redrawFromOffset( uint offset, bool finishWindow )
{
  int lineHeight = mHexBuffer->lineHeight();
  uint docLine   = mHexBuffer->calculateLine( offset );

  int t = docLine * lineHeight - startY() + frameWidth();
  if( finishWindow == true )
  {
    QRect r = contentsRect();
    r.setTop( t );
    paintText( contentsRect().intersect( r ), false );
  }
  else
  {
    int h = t + lineHeight;
    QRect r( contentsRect().left(), t, contentsRect().width(), h );
    paintText( contentsRect().intersect( r ), false );
  }
}




void CHexViewWidget::paintText( const QRect &rect, bool expand )
{
  QRect r = rect;

  if( expand == true )
  {
    #ifdef USE_NORTHWEST_GRAVITY
    r.setLeft( r.left() - frameWidth() );
    r.setTop( r.top() - frameWidth() );
    #endif
  }

  if( contentsRect().contains( r ) == false )
  {
    paintFrame();
    if( r.left() < frameWidth() ) { r.setLeft( frameWidth() ); }
    if( r.top() < frameWidth() ) { r.setTop( frameWidth() ); }
  }

  int maxX = width() - frameWidth() - 1 -
    (mVertScroll->isVisible() ? mScrollBarSize : 0);
  int maxY = height() - frameWidth() - 1 -
    (mHorzScroll->isVisible() ? mScrollBarSize : 0);

  if( r.right() > maxX ) { r.setRight( maxX ); }
  if( r.bottom() > maxY ) { r.setBottom( maxY ); }

  QPainter paint( &mTextBuffer );
  paint.setFont( mHexBuffer->font() );

  int lineHeight = mHexBuffer->lineHeight();
  int docLine    = (startY() + r.y() - frameWidth()) / lineHeight;
  if( docLine < 0 ) { docLine = 0; }
  int y          = docLine * lineHeight - startY();
  int yMax       = r.height();
  int xMax       = r.x() + r.width();

  y += frameWidth();

  int s = 0;
  int d = r.y()-y;
  int h;
  while( yMax > 0 )
  {
    mHexBuffer->drawText( paint, docLine, startX()-frameWidth(), r.x(), xMax );

    if( d != 0 )
    {
      h = lineHeight - d;
      if( h > yMax ) { h = yMax; }
    }
    else
    {
      h = yMax > lineHeight ? lineHeight : yMax;
    }
    bitBlt( this, r.x(), r.y()+s, &mTextBuffer, r.x(), d, r.width(), h );

    s       += h;
    yMax    -= h;
    docLine += 1;
    d = 0;
  }
  paint.end();
}



void CHexViewWidget::paintCursor( int cursorMode )
{
  QPainter paint;
  paint.begin( &mTextBuffer );
  paint.setFont( mHexBuffer->font() );

  int f = frameWidth();

  if( cursorMode == CHexBuffer::cursor_prev )
  {
    int line = mHexBuffer->prevCursorLine();
    SCursorPosition p;

    mHexBuffer->prevCursor( CHexBuffer::edit_primary, p );
    mHexBuffer->drawText( paint, line, startX(), p.x, p.x + p.w );
    if( p.y + p.h + f > contentsRect().bottom() )
      p.h = contentsRect().bottom() - p.y - f + 1;
    bitBlt( this, p.x+f, p.y+f, &mTextBuffer, p.x, 0, p.w, p.h );

    mHexBuffer->prevCursor( CHexBuffer::edit_secondary, p );
    mHexBuffer->drawText( paint, line, startX(), p.x, p.x + p.w );
    if( p.y + p.h + f > contentsRect().bottom() )
      p.h = contentsRect().bottom() - p.y - f + 1;
    bitBlt( this, p.x+f, p.y+f, &mTextBuffer, p.x, 0, p.w, p.h );
  }
  else
  {
    int line = mHexBuffer->cursorLine();
    SCursorPosition p;

    mHexBuffer->currCursor( CHexBuffer::edit_primary, p );
    mHexBuffer->drawText( paint, line, startX(), p.x, p.x + p.w );
    if( p.y + p.h + f > contentsRect().bottom() )
      p.h = contentsRect().bottom() - p.y - f + 1;
    bitBlt( this, p.x+f, p.y+f, &mTextBuffer, p.x, 0, p.w, p.h );

    mHexBuffer->currCursor( CHexBuffer::edit_secondary, p );
    mHexBuffer->drawText( paint, line, startX(), p.x, p.x + p.w );
    if( p.y + p.h + f > contentsRect().bottom() )
      p.h = contentsRect().bottom() - p.y - f + 1;
    bitBlt( this, p.x+f, p.y+f, &mTextBuffer, p.x, 0, p.w, p.h );
  }

  paint.end();
}





void CHexViewWidget::updateCursor( SCursorConfig &cc, bool always,
				   bool touchSelection )
{
  if( mHexBuffer->cursorChanged() == false && always == false )
  {
    return;
  }

  //
  // Make blinking (and perhaps invisible) cursor visible
  //
  setupCursorTimer();

  //
  // Clear cursor at old location
  //
  paintCursor( CHexBuffer::cursor_prev );

  //
  // Compute the new position of the vertical scroll bar.
  //
  int position, h;
  if( cc.controlButton() == true )
  {
    //
    // The cursor should stay fixed (if possible) in the window while
    // the text is scrolled (e.g., PageUp/Down behavior). The position
    // of the vertical scrollbar must change just as much as the cursor
    // has changed in the vertical direction.
    //
    h  = frameWidth()*2;
    h += mHorzScroll->isVisible() == false ? 0 : mScrollBarSize;
    position = mHexBuffer->cursorFixedPosition( startY(), height()-h );
    changeYPos( position );
  }
  else
  {
    h  = frameWidth()*2;
    h += mHorzScroll->isVisible() == false ? 0 : mScrollBarSize;
    position = mHexBuffer->cursorChangePosition( startY(), height()-h );
    changeYPos( position );
  }

  //
  // Paint cursor at new location and update the vertical scroll bar.
  //
  paintCursor( CHexBuffer::cursor_curr );
  mVertScroll->blockSignals( true );
  mVertScroll->setValue( position );
  mVertScroll->blockSignals( false );

  if( touchSelection == true )
  {
    setSelection( mHexBuffer->cursorOffset(), cc.removeSelection() );
    unmark();
  }
  emit cursorChanged( mHexBuffer->cursorState() );
}



void CHexViewWidget::toggleEditor( void )
{
  bool success = mHexBuffer->toggleEditor();
  if( success == false )
  {
    return;
  }

  SCursorConfig cc;
  updateCursor( cc, true );
  redrawFromOffset( mHexBuffer->cursorOffset(), false );
}


void CHexViewWidget::cursorStep( SCursorConfig &cc, uint stepSize )
{
  mHexBuffer->cursorStep( stepSize, cc.altButton() ? false : true, true );
  cc.emulateControlButton( false );
  updateCursor( cc );
}



void CHexViewWidget::cursorLeft( SCursorConfig &cc )
{
  bool cellLevel = mMisc.cursorJump == false || cc.controlButton();
  cc.emulateControlButton( false );
  mHexBuffer->cursorLeft( cellLevel );
  updateCursor( cc, cellLevel );
}


void CHexViewWidget::cursorRight( SCursorConfig &cc )
{
  bool cellLevel = mMisc.cursorJump == false || cc.controlButton();
  cc.emulateControlButton( false );
  mHexBuffer->cursorRight( cellLevel );
  updateCursor( cc, cellLevel );
}



void CHexViewWidget::cursorUp( SCursorConfig &cc )
{
  mHexBuffer->cursorUp( 1 );
  updateCursor( cc );
}


void CHexViewWidget::cursorDown( SCursorConfig &cc )
{
  mHexBuffer->cursorDown( 1 );
  updateCursor( cc );
}


void CHexViewWidget::cursorHome( SCursorConfig &cc )
{
  mHexBuffer->cursorHome( cc.controlButton() );
  updateCursor( cc );
}


void CHexViewWidget::cursorEnd( SCursorConfig &cc )
{
  mHexBuffer->cursorEnd( cc.controlButton() );
  updateCursor( cc );
}


void CHexViewWidget::cursorPageDown( SCursorConfig &cc )
{
  mHexBuffer->cursorDown( height() / mHexBuffer->lineHeight() );
  cc.emulateControlButton( true );
  updateCursor( cc );
}


void CHexViewWidget::cursorPageUp( SCursorConfig &cc )
{
  mHexBuffer->cursorUp( height() / mHexBuffer->lineHeight() );
  cc.emulateControlButton( true );
  updateCursor( cc );
}


void CHexViewWidget::cursorInsert( SCursorConfig &/*cc*/ )
{
  // Toggle mode
  setEditMode( mEditMode == CHexBuffer::EditInsert ?
	       CHexBuffer::EditReplace : CHexBuffer::EditInsert );
}



void CHexViewWidget::cursorDelete( SCursorConfig &/*cc*/ )
{
  int numLine = mHexBuffer->numLines();

  bool success = mHexBuffer->removeAtCursor( false );
  if( success == false )
  {
    return;
  }

  updateWindow( numLine == mHexBuffer->numLines() ? false : true, true );
  emit dataChanged();
}


void CHexViewWidget::cursorBackspace( SCursorConfig &/*cc*/ )
{
  int numLine = mHexBuffer->numLines();

  bool success = mHexBuffer->removeAtCursor( true );
  if( success == false )
  {
    return;
  }

  updateWindow( numLine == mHexBuffer->numLines() ? false : true, true );
  emit dataChanged();
}


void CHexViewWidget::cursorInput( QChar c )
{
  uint cursorLine = mHexBuffer->cursorLine();
  bool success = mHexBuffer->inputAtCursor( c );
  if( success == false )
  {
    return;
  }

  updateWindow( cursorLine );
  emit dataChanged();
}


void CHexViewWidget::setEditMode( CHexBuffer::EEditMode mode )
{
  mEditMode = mode;
  mHexBuffer->setEditMode( mEditMode, mCursor.alwaysBlockShape,
			   mCursor.thickInsertShape );
  setupCursorTimer();

  //
  // This will redraw the current line (which contains the cursor)
  //
  redrawFromOffset( mHexBuffer->cursorOffset(), false );
  emit editMode( mEditMode );
}


void CHexViewWidget::setDropHighlight( bool dropHighlight )
{
  mDropHighlight = dropHighlight;
  if( mDropHighlight == true )
  {
    //
    // 2000-01-10 Espen Sand
    // Highlight. I have reimplemented QFrame::drawFrame(QPainter *)
    // to support a custom frame color. I assume the frame shape is
    // "QFrame::WinPanel|QFrame::Plain" in that function.
    //
    setFrameStyle( QFrame::WinPanel|QFrame::Plain );
  }
  else
  {
    setFrameStyle( QFrame::WinPanel|QFrame::Sunken );
  }
}



#include "hexviewwidget.moc"
