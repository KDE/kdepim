/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999-2000 Espen Sand, espensa@online.no
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

#ifndef _HEX_BUFFER_H_
#define _HEX_BUFFER_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include <iostream>
#include <time.h>

#include <qbitarray.h> 
#include <qdatetime.h> 
#include <qfile.h>
#include <qfont.h>
#include <qptrlist.h>
#include <qpainter.h>
#include <qstring.h>
#include <qtextstream.h>

#include <kapplication.h>

#include "conversion.h"
#include "hexeditstate.h"
#include "hexprinter.h"
#include "progress.h"

//
// Marco to simplify usage of function pointers
//
#define THIS_FPTR( func ) ((this->*func))

struct SSearchControl
{
  QByteArray key;
  QByteArray val;
  uint keyType;
  bool fromCursor;
  bool inSelection;
  bool forward;
  bool ignoreCase;
  bool match;
  uint numReplace;

  bool wrapValid;
  bool wrapActive;
  uint wrapMark;
};


struct SFilterControl
{
  enum Operation
  {
    OperandAndData = 0,
    OperandOrData,
    OperandXorData,
    InvertData,
    ReverseData,
    RotateData,
    ShiftData,
    SwapBits
  };

  int execute( uchar *dest, uchar *src, uint size );


  QByteArray operand;
  int rotate[2];
  Operation  operation;
  bool fromCursor;
  bool inSelection;
  bool forward;
};


struct SInsertData
{
  uint size;
  QByteArray pattern;
  uint offset;
  bool repeatPattern;
  bool onCursor;
};


struct SExportRange
{
  enum EMode
  {
    All = 0,
    Selection,
    Range
  };
    
  EMode mode;
  uint start;
  uint stop;
};


struct SExportText
{
  SExportRange range;
  QString destFile;
};


struct SExportHtml
{
  SExportRange range;
  QString package;
  QString prefix;
  uint linePerPage;
  uint topCaption;
  uint bottomCaption;
  bool symLink;
  bool navigator;
  bool blackWhite;
};


struct SExportCArray
{
  enum ElementType
  {
    Char = 0,
    Uchar,
    Short,
    Ushort,
    Int,
    Uint,
    Float,
    Double
  };
    
  const char *printFormatted( const char *b, uint maxSize ) const;
  QString variableName( uint range ) const;
  int elementSize( void ) const;

  SExportRange range;
  QString destFile;
  QString arrayName;
  int  elementType;
  uint elementPerLine;
  bool unsignedAsHexadecimal;
};


struct SStatisticControl
{
  SStatisticControl( void )
  {
    memset( occurence, 0, sizeof(occurence) );
    documentSize = 0;
  }

  uint documentSize;
  uint occurence[256];
  QString documentName;
};


struct SPagePosition
{
  SPagePosition( time_t now, uint numLine, uint linePerPage )
  {
    init( now, numLine, linePerPage );
  }

  SPagePosition( void )
  {
    init( 0, 1, 1 );
  }

  void init( time_t at, uint numLine, uint linePerPage )
  {
    if( linePerPage == 0 ) { linePerPage = 1; }

    now = at;
    curPage = 1;
    maxPage = numLine / linePerPage + (numLine % linePerPage ? 1 : 0);
    if( maxPage < curPage ) { maxPage = curPage; }
  }

  void step( uint stepSize=1 )
  {
    curPage += stepSize;
    if( curPage > maxPage ) { curPage = maxPage; }
  }

  uint current( void )
  {
    return( curPage );
  }

  uint max( void )
  {
    return( maxPage );
  }

  uint curPage;
  uint maxPage;
  time_t now;
};



class CStringCollectControl
{
  public:
    CStringCollectControl( void )
    {
      mList.setAutoDelete( true );
    }

    int add( uint offset, const QByteArray &a )
    {
      QString *s = new QString();
      if( s == 0 ) 
      { 
	return( Err_NoMemory ); 
      }
      
      if( decimalOffset == true )
      {
	s->sprintf( "%010u", offset );
      }
      else
      {
	s->sprintf( "%04x:%04x", offset>>16, offset&0x0000FFFF );
      }

      *s += QString( a );

      mList.append( s );
      return( Err_Success );
    }

    uint offsetLen( void )
    {
      return( decimalOffset ? 10 : 9 );
    }

    void clear( void )
    {
      mList.clear();
    }
    
    const QString *get( uint index )
    {
      return( mList.at( index ) );
    }

    int count( void )
    {
      return( mList.count() );
    }

    QPtrList<QString> &list( void )
    {
      return( mList );
    }

  public:
    uint minLength;
    bool decimalOffset;
    bool allow8bit;

  private:
    QPtrList<QString> mList;
};






struct SCursorConfig
{
  SCursorConfig( void ) 
  {
    state = 0;
  }

  bool selectOn( void )
  {
    return( state & Qt::ShiftButton );
  }

  bool removeSelection( void )
  {
    return( state & Qt::ShiftButton ? false : true );
  }

  void setKeepSelection( bool val )
  {
    state = val == true ? state|Qt::ShiftButton : state&~Qt::ShiftButton;
  }
  
  bool controlButton( void )
  {
    return( state & Qt::ControlButton ? true : false );
  }

  bool shiftButton( void )
  {
    return( state & Qt::ShiftButton ? true : false );
  }

  bool altButton( void )
  {
    return( state & Qt::AltButton ? true : false );
  }


  void emulateControlButton( bool val )
  {
    state = val == true ? state|Qt::ControlButton : state&~Qt::ControlButton;
  }

  int state;
};



struct SCursorSpec
{
  enum EShape
  {
    solid = 0,
    frame,
    thin
  };
    
  void reset( void ) 
  { 
    offset = 0;
    x1 = x2 = y = cell = maxCell = 0; 
  }

  void setShape( EShape primaryShape, EShape secondaryShape, uint unitWidth,
		 uint numCell )
  {

    if( primaryShape == thin )
    {
      if( onlyBlock == true ) { primaryShape = solid; }
    }
    if( secondaryShape == thin )
    {
      if( onlyBlock == true ) { secondaryShape = solid; }
    }

    mPrimaryShape = primaryShape;
    if( mPrimaryShape == solid )
    {
      drawWidth1  = unitWidth;
      drawOffset1 = 0;
    }
    else if( mPrimaryShape == frame )
    {
      drawWidth1  = unitWidth * numCell;
      drawOffset1 = 0;
    }
    else
    {
      mPrimaryShape = thin;
      if( thickState == true )
      {
	drawWidth1  = 2;
	drawOffset1 = -1;
      }
      else
      {
	drawWidth1  = 5;
	drawOffset1 = -3;
      }
    }

    mSecondaryShape = secondaryShape;
    if( mSecondaryShape == solid )
    {
      drawWidth2  = unitWidth;
      drawOffset2 = 0;
    }
    else if( mSecondaryShape == frame )
    {
      drawWidth2  = unitWidth * numCell;
      drawOffset2 = 0;
    }
    else
    {
      mSecondaryShape = thin;
      if( thickState == true )
      {
	drawWidth2  = 2;
	drawOffset2 = -1;
      }
      else
      {
	drawWidth2  = 5;
	drawOffset2 = -3;
      }
    }
  }

  void setShapeModifier( bool alwaysBlock, bool useThick )
  {
    onlyBlock  = alwaysBlock;
    thickState = useThick;
  }

  void dump( void )
  {
    std::cout << "offset: " << offset << " y: " << y << " x1: " << x1 << std::endl;
    std::cout << "x2: " << x2 << " cell: " << cell << std::endl;
  }

  bool inside( uint min, uint max )
  {
    return( offset >= min && offset < max );
  }

  int drawX1( void )
  {
    return( x1 + drawOffset1 );
  }

  int drawX2( void )
  {
    return( x2 + drawOffset2 );
  }

  int width1( void )
  {
    return( drawWidth1 );
  }

  int width2( void )
  {
    return( drawWidth2 );
  }

  uint offset;
  int  y;
  int  x1;
  int  x2;
  int  cell;
  int  maxCell;
  bool onlyBlock;
  bool thickState;

  char data;

  int mPrimaryShape;
  int mSecondaryShape;
  int drawWidth1;
  int drawWidth2;
  int drawOffset1;
  int drawOffset2;
};


struct SCursor
{
  SCursor( void )
  {
    mLineSize = 1;
    mDocumentSize = 0;
    mFixedSizeMode = false;
    mArea = 0;
  }

  void reset( void )
  {
    prev.reset();
    curr.reset();
    next.reset();
  }

  void setDocumentSize( uint documentSize )
  {
    mDocumentSize = documentSize;
  }

  void setFixedSizeMode( bool fixedSizeMode )
  {
    mFixedSizeMode = fixedSizeMode;
  }

  bool incCell( void )
  {
    if( curr.cell + 1 >= curr.maxCell )
    {
      addOffset( 1 );
      return( true );
    }
    else
    {
      next.cell = curr.cell + 1;
      return( false );
    }
  }

  bool decCell( void )
  {
    if( curr.cell == 0 )
    {
      if( curr.offset > 0 )
      {
	decOffset( 1, true );
	next.cell = curr.maxCell > 0 ? curr.maxCell - 1 : 0;
      }
      return( true );
    }
    else
    {
      next.cell = curr.cell - 1;
      return( false );
    }
  }

  void resetCell( void )
  {
    next.cell = 0;
  }


  void addOffset( uint val )
  {
    setOffset( curr.offset + val );
  }

  void decOffset( uint val, bool ignoreCell )
  {
    if( ignoreCell == true || curr.cell == 0 )
    {
      setOffset( val > curr.offset ? curr.offset%mLineSize : curr.offset-val);
    }
    else
    {
      setOffset( curr.offset );
    }
  }

  void setOffset( uint offset )
  {
    /*
    if( offset >= mDocumentSize )
    {
      if( mFixedSizeMode == true )
      {
	offset = mDocumentSize > 0 ? mDocumentSize - 1 : 0;
      }
      else
      {
	offset = mDocumentSize;
      }
    }
    */

    next.offset = offset;
    next.cell = 0;
  }

  uint getOffset( void )
  {
    return( curr.offset );
  }

  void setOffset( uint offset, uint bit, bool backward, bool fromCursor,
		  uint textSize )
  {
    if( fromCursor == true )
    {
      if( backward == true )
      {
	setOffset( offset > curr.offset ? 0 : curr.offset - offset );
      }
      else
      {
	setOffset( curr.offset + offset );
      }
    }
    else
    {
      if( backward == true )
      {
	setOffset( offset > textSize ? 0 : textSize - offset );
      }
      else
      {
	setOffset( offset > textSize ? textSize : offset );
      }
    } 
    setBit( bit );
  }


  void setBit( uint bit )
  {
    bit = (bit > 7) ? 0 : 7 - bit;
    next.cell = bit / mCellWeight;
  }


  void home( bool toExtreme )
  {
    if( toExtreme == true )
    {
      setOffset( 0 );
    }
    else
    {
      setOffset( next.offset - curr.offset % mLineSize );
    }
  }

  void end( bool toExtreme )
  {
    uint maxOffset;
    if( mFixedSizeMode == true )
    {
      maxOffset = mDocumentSize > 0 ? mDocumentSize-1 : mDocumentSize;
    }
    else
    {
      maxOffset = mDocumentSize;
    }

    if( toExtreme == true )
    {
      setOffset( maxOffset );
    }
    else
    {
      uint newOffset = next.offset + (mLineSize-1) - curr.offset % mLineSize;
      setOffset( newOffset > maxOffset ? maxOffset : newOffset );
    }

  }

  void up( uint numLines )
  {
    decOffset( numLines * mLineSize, true );
  }

  void down( uint numLines )
  {
    addOffset( numLines * mLineSize );
  }

  bool changed( void )
  {
    return( prev.offset != curr.offset || prev.cell != curr.cell ||
	    prev.data != curr.data ? true : false );
  }

  uint bit( void )
  {
    uint bitValue = mCellWeight*(curr.maxCell - curr.cell) - 1;
    return( bitValue > 7 ? 7 : bitValue );
  }

  uint cellWeight( void )
  {
    return( mCellWeight );
  }

  void setLineSize( uint lineSize )
  {
    mLineSize = lineSize < 1 ? 1 : lineSize;
  }

  void setCellWeight( uint cellWeight )
  {
    mCellWeight = cellWeight;
  }

  int area( void )
  {
    return( mArea );
  }

  void setArea( int area )
  {
    mArea = area;
  }

  void setShape( SCursorSpec::EShape primary, SCursorSpec::EShape secondary,
		 uint unitWidth, uint numCell )
  {
    curr.setShape( primary, secondary, unitWidth, numCell );
  }

  void setShapeModifier( bool alwaysBlock, bool useThick )
  {
    curr.setShapeModifier( alwaysBlock, useThick );
  }

  uint mLineSize;
  uint mDocumentSize;
  uint mCellWeight;
  int  mArea;
  bool mFixedSizeMode;
  SCursorSpec prev;
  SCursorSpec curr;
  SCursorSpec next;
};


struct SCursorPosition
{
  int x;
  int y;
  int w;
  int h;
};

struct SCursorOffset
{
  uint offset;
  uint bit;
};


struct SSelectSpec
{
  void init( uint offset )
  {
    start = stop = anchor = offset;
  }

  void set( uint offset )
  {
    if( offset < anchor )
    {
      start = offset;
      stop  = anchor;
    }
    else
    {
      stop  = offset;
      start = anchor;
    }
  }

  void expand( uint value )
  {
    if( anchor == stop ) { anchor +=value; }
    stop += value;
    //set( stop + value );
  }

  void shrink( uint value )
  {
    uint newVal = start + value > stop ? start : stop - value;
    if( anchor == stop ) { anchor = newVal; }
    stop = newVal;
    /*
    if( start + value > stop )
    {
      set( start );
    }
    else
    {
      set( stop - value );
    }
    */
  }


  uint start;
  uint stop;
  uint anchor;
};


struct SSelect
{
  SSelect( void )
  {
    reset();
  }

  bool init( uint offset )
  {
    curr.init( offset );
    if( isValid == true )
    {
      isValid = false;
      return( true );
    }
    else
    {
      return( false );
    }
  }
  
  void reset( void )
  {
    curr.init( 0 );
    isValid = false;
  }

  bool set( uint offset )
  {
    isValid = true;

    curr.set( offset );
    if( curr.start != prev.start || curr.stop != prev.stop )
    {
      return( true );
    }
    else
    {
      return( false );
    }
  }

  void expand( uint value )
  {
    if( isValid == false )
    {
      return;
    }
    curr.expand( value );
  }

  void shrink( uint value )
  {
    if( isValid == false )
    {
      return;
    }
    curr.shrink( value );
  }


  void sync( void )
  { 
    prev = curr;
  }

  bool inside( uint offset )
  {
    return( isValid == true && offset >= curr.start && offset < curr.stop );
  }

  bool inside( uint offset, uint range )
  {
    return( isValid == true && (offset + range) >= curr.start && 
	    offset < curr.stop );
  }

  bool verify( uint &start, uint &stop )
  {
    if( isValid == false )
    {
      return( false );
    }

    if( start < curr.start ) { start = curr.start; }
    if( stop > curr.stop ) { stop = curr.stop; }
    return( true );
  }

  uint start( uint offset )
  {
    //
    // If 'curr.star' is smaller than  'offset' then the start is at
    // the start of line.
    //
    return( curr.start < offset ? 0 : curr.start - offset );
  }
  
  uint start( void )
  {
    return( curr.start );
  }

  uint stop( uint offset, uint range )
  {
    return( curr.stop > offset + range ? range : curr.stop - offset );
  }

  uint stop( void )
  {
    return( curr.stop );
  }

  bool valid( void )
  {
    return( isValid );
  }

  uint size( void )
  {
    if( isValid == false )
    {
      return( 0 );
    }
    else
    {
      return( curr.start >= curr.stop ? 0 : curr.stop - curr.start );
    }
  }

  void startChange( uint &first, uint &last )
  {
    if( curr.start <= prev.start )
    {
      first = curr.start;
      last  = prev.start;
    }
    else
    {
      first = prev.start;
      last  = curr.start;
    }
  }

  void stopChange( uint &first, uint &last )
  {
    if( curr.stop <= prev.stop )
    {
      first = curr.stop;
      last  = prev.stop;
    }
    else
    {
      first = prev.stop;
      last  = curr.stop;
    }
  }


  bool isValid;
  SSelectSpec prev;
  SSelectSpec curr;
};



class CHexAction 
{
  public:
    enum HexAction 
    { 
      replace
    };
    
  public:
    CHexAction( HexAction action, uint offset );
    ~CHexAction( void );
    void setData( uint size, char *data, uint dataSize );

  public:
    HexAction mAction;
    uint mOffset;
    uint mSize;
    char *mData;
    uint mDataSize;
    CHexAction *mNext;
};

class CHexActionGroup 
{
  public:
    CHexActionGroup( uint startOffset, uint startBit );
    ~CHexActionGroup( void );
    void insertAction( CHexAction *hexAction );

  public:
    uint mStartOffset;
    uint mStopOffset;
    uint mStartBit;
    uint mStopBit;
    CHexAction *mHexAction;
};



struct SCursorState
{ 
  bool valid;
  uint selectionOffset;
  uint selectionSize;
  uint offset;
  uint cell;
  unsigned char data[8];
  uint undoState;
  bool charValid;
};

struct SFileState
{ 
  bool valid;
  uint size;
  bool modified;
};





class CHexBuffer;
typedef int  (CHexBuffer::*PrintCellFunc)( char *buf, unsigned char data );
typedef bool (CHexBuffer::*InputCellFunc)( unsigned char *dest, int value, 
					   uint cell );
typedef void (CHexBuffer::*PrintOffsetFunc)( char *buf, uint offset );


class CHexBuffer : public QByteArray
{
  public:
    enum EColumn
    {
      VisibleColumn   = 0x0,
      OffsetColumn    = 0x1,
      PrimaryColumn   = 0x2,
      SecondaryColumn = 0x4,
      EveryColumn     = 0x7
    };

    enum EEditArea
    {
      edit_none = 0,
      edit_primary,
      edit_secondary
    };

    enum ECursorMode
    {
      cursor_curr = 0,
      cursor_prev
    };

    enum EEditMode
    {
      EditInsert = 0,
      EditReplace
    };

    enum EUndoState
    {
      UndoOk = 1,
      RedoOk = 2
    };

  public:
    CHexBuffer( void );
    ~CHexBuffer( void );

    int  setLayout( SDisplayLayout &layout );
    void setColor( SDisplayColor &color );
    void setInputMode( SDisplayInputMode &mode );
    bool toggleEditor( void );

    bool matchWidth( uint width );

    void setNonPrintChar( QChar nonPrintChar );
    void setShowCursor( bool showCursor );
    void setDisableCursor( bool disableCursor );
    void setCursorShapeModifier( bool alwaysBlock, bool thickInsert );
    void setEditMode( EEditMode editMode );
    void setEditMode( EEditMode editMode, bool alwaysBlock, bool thickInsert );
    void setMaximumSize( uint size );
    void setDocumentSize( uint size );
    void setUndoLevel( uint level );
    void setSoundState( bool inputSound, bool fatalSound );
    void setBookmarkVisibility( bool showInColumn, bool showInEditor ); 

    void setFont( const SDisplayFontInfo &fontInfo );
    int  setEncoding( CConversion::EMode mode, CProgress &p );

    bool setCursorPosition( int x, int y, bool init, bool cellLevel );

    int  readFile( QFile &file, const QString &url, CProgress &p );
    int  insertFile( QFile &file, CProgress &p );
    int  writeFile( QFile &file, CProgress &p );
    int  newFile( const QString &url );
    void closeFile( void );
    void registerDiskModifyTime( const QFile &file );

    bool changedOnDisk( void );

    void drawText( QPainter &paint, uint line, int sx, int x, int w );
    void drawText( QPainter &paint, uint line, int x1, int x2, int y, 
		   bool useBlackWhite );

    void drawHeader( QPainter &paint, int sx, int width, int y, bool isFooter,
		     const SPageHeader &header,const SPagePosition &position );
    int  headerHeight( QPainter &paint );
    int  headerMargin( QPainter &paint );

    bool inputAtCursor( QChar c );
    int  inputAtCursor( const QByteArray &buf, uint oldSize );
    bool removeAtCursor( bool beforeCursor );
 
    int  locateRange( const SExportRange &range, uint &start, uint &stop );
    int  exportText( const SExportText &ex, CProgress &p );
    int  exportHtml( const SExportHtml &ex, CProgress &p );
    int  exportCArray( const SExportCArray &ex, CProgress &p );
    int  copySelectedText( QByteArray &array, int columnSegment=VisibleColumn);
    int  copyAllText( QByteArray &array );
    int  copyText( QByteArray &array, const SExportRange &range, 
		   int columnSegment );
    int  copySelectedData( QByteArray &array );

    uint numPage( CHexPrinter &printer );
    int  print( CHexPrinter &printer, CProgress &p );

    uint printLine( char *dst, uint line );
    uint printLine( char *dst, uint line, int columnSegment );

    bool cutSelection( void );
    bool undo( void );
    bool redo( void );
    int  addBookmark( int position );
    bool removeBookmark( int position );
    void updateBookmarkMap( bool resize );

    int  findFirst( SSearchControl &sc );
    int  findNext( SSearchControl &sc );
    int  findWrap( SSearchControl &sc );
    int  replaceAll( SSearchControl &sc, bool init );
    int  replaceMarked( SSearchControl &sc );
    int  filter( SFilterControl &fc );
    int  collectStrings( CStringCollectControl &sc );
    int  collectStatistic( SStatisticControl &sc, CProgress &p );

    void doActionGroup( CHexActionGroup *group );
    void doAction( CHexAction *action );

    inline SCursorState &cursorState( void );
    inline void valueOnCursor( QByteArray &buf, uint size );
    inline SFileState &fileState( void );
    inline const SDisplayLayout &layout( void );

    inline const SDisplayInputMode &inputMode( void );
    inline QPtrList<SCursorOffset> &bookmarkList( void );

    inline bool documentPresent( void );
    inline bool modified( void );
    inline uint undoState( void );
    inline uint documentSize( void );
    inline EEditMode editMode( void );
    inline const SEncodeState &encoding( void );
    inline bool losslessEncoding( CConversion::EMode mode );

    inline QString &url( void );
    bool hasFileName( void );
    inline void setUrl( const QString &url );
    inline const QDateTime &diskModifyTime( void );
    


    inline uint calculateLine( uint offset );
    inline int  lineSize( void );
    inline int  lineHeight( void );
    inline int  fontAscent( void );
    inline int  lineWidth( void );
    inline int  unitWidth( void );
    inline int  numLines( void );
    inline int  totalHeight( void );
    inline const QFont &font( void );
    inline SCursor *textCursor( void );
    inline const QColor &backgroundColor( void );
    inline int  startX( void );
    inline int  startY( void );
    inline void setStartX( int val );
    inline void setStartY( int val );

    inline bool selectionSet( uint offset, bool init );
    inline void selectionSyncronize( void );
    inline void selectionStartChange( uint &offset1, uint &offset2 );
    inline void selectionStopChange( uint &offset1, uint &offset2 );
    inline bool cursorInsideSelection( void ); 
 
    inline void markSet( uint offset, uint size );
    inline bool markSet( uint offset, bool init );
    inline bool markRemove( void );
    inline void markSyncronize( void );
    inline void markStartChange( uint &offset1, uint &offset2 );
    inline void markStopChange( uint &offset1, uint &offset2 );

    inline uint cursorOffset( void );
    inline uint cursorBit( void );
    inline uint cursorLine( void );
    inline uint prevCursorLine( void );
    inline SCursor &cursor( void );

    inline void currCursor( EEditArea editArea, SCursorPosition &p );
    inline void prevCursor( EEditArea editArea, SCursorPosition &p );

    inline void cursorUp( uint lines );
    inline void cursorDown( uint lines );
    inline void cursorRight( bool cellLevel );
    inline void cursorLeft( bool cellLevel );
    inline void cursorStep( uint size, bool forward, bool modulo );
    inline void cursorHome( bool toExtreme );
    inline void cursorEnd( bool toExtreme );
    inline void cursorGoto( uint offset, uint bit );
    inline void cursorGoto( uint offset, uint bit, bool backward, 
			    bool fromCursor );
    inline bool cursorChanged( void );
    inline void cursorResetEditArea( void );
    inline bool cursorPrimaryEdit( void );
    inline int  cursorFixedPosition( int position, int height );
    inline int  cursorChangePosition( int position, int height );
    void cursorReset( void );
 
  private:
    enum { BookmarkOnLine = 0x01, BookmarkOnCursor = 0x02 };

    void computeLineWidth( void );
    void computeNumLines( void );
    void cursorCompute( void );

    void drawSelection( QPainter &paint, QColor &color, uint start, uint stop, 
			int sx );
    int  drawBookmarks(QPainter &paint, uint line, int startx);
    void drawCursor( QPainter &paint, uint line, int startx, bool onBookmark );

    void recordStart( SCursor &cursor );
    void recordReplace( SCursor &cursor, uint size, char *data, uint dataSize);
    void recordEnd( SCursor &cursor );
    void doReplace( CHexAction *hexAction, bool removeData );
    int  scanData( SSearchControl &sc, bool init );
    int  initScanData( SSearchControl &sc );

    inline const QColor &foregroundColor( uint column );
    inline int  printDummyCell( char *buf, unsigned char data );
    inline int  printHexadecimalBigCell( char *buf, unsigned char data );
    inline int  printHexadecimalSmallCell( char *buf, unsigned char data );
    inline int  printDecimalCell( char *buf, unsigned char data );
    inline int  printOctalCell( char *buf, unsigned char data );
    inline int  printBinaryCell( char *buf, unsigned char data );
    inline int  printAsciiCell( char *buf, unsigned char data );
    inline void printDummyOffset( char *buf, uint offset );
    inline void printHexadecimalBigOffset( char *buf, uint offset );
    inline void printHexadecimalSmallOffset( char *buf, uint offset );
    inline void printDecimalOffset( char *buf, uint offset );

    bool inputDummy( unsigned char *dest, int value, uint cell );
    bool inputHexadecimal( unsigned char *dest, int value, uint cell );
    bool inputDecimal( unsigned char *dest, int value, uint cell );
    bool inputOctal( unsigned char *dest, int value, uint cell );
    bool inputBinary( unsigned char *dest, int value, uint cell );
    bool inputAscii( unsigned char *dest, int value, uint cell );

    int  moveBuffer( uint destOffset, uint srcOffset );
    int  resizeBuffer( uint offset );

    void inputSound( void );
    void fatalSound( void );

    int printHtmlDataPage( const QString &tocName, 
			   const QStringList &fileNames, uint index, 
			   const SExportHtml &ex, uint line, uint numLine );
    void printHtmlCaption( QTextStream &os, uint captionType, uint curPage,
			   uint numPage );
    void printHtmlNavigator( QTextStream &os, const QString *next, 
			     const QString *prev, const QString *toc );
    void printHtmlTocPage( const QString &tocName, 
			   const QString &linkName,
			   const QStringList &fileNames, 
			   const QStringList &offsets, uint numPage );
    int printHtmlHeader( QTextStream &os, bool isFront );
    int printHtmlTable( QTextStream &os, uint line, uint numLine, bool bw );
    int printHtmlLine( QTextStream &os, uint offset, bool isPrimary, bool bw );

  signals:
    void fileSize( uint size );

  private:
    QString mUrl;
    QDateTime mDiskModifyTime;
    SDisplayLayout mLayout;
    SDisplayColor mColor;
    SDisplayFontInfo mFontInfo;
    CConversion mEncode;
    bool mCharValid[256];

    unsigned char *mColorIndex;
    char *mPrintBuf;
    bool mLoadingData;
    int  mStartX;
    int  mStartY;

    int  mFontHeight;
    int  mFontAscent;
    int  mLineWidth;
    int  mFixedWidth;
    int  mUnitWidth;
    int  mSplitWidth;
    int  mNumLines;
    int  mTextStart1;
    int  mTextStart2;
    int  mNumCell;

    uint mDocumentSize;
    uint mMaximumSize;
    bool mFixedSizeMode;
    bool mDocumentModified;
    EEditMode mEditMode;
    SDisplayInputMode mInputMode;

    int  mOffsetSize;
    int  mOffsetIndex;
    int  mPrimaryWidth;
    int  mSecondaryWidth;
    int  mActiveEditor;

    SSelect mSelect;
    SSelect mMark;

    SCursor mCursor;
    bool mShowCursor;
    bool mDisableCursor;

    bool mInputErrorSound;
    bool mFatalErrorSound;

    bool mShowBookmarkInOffsetColumn;
    bool mShowBookmarkInEditor;

    uint mUndoLimit;
    uint mUndoIndex;
    QPtrList<CHexActionGroup> mUndoList;
    QPtrList<SCursorOffset> mBookmarkList;
    QBitArray mBookmarkMap;

    PrintCellFunc   printCell;
    PrintOffsetFunc printOffset;
    InputCellFunc   inputCell;

    static char mHexBigBuffer[16];
    static char mHexSmallBuffer[16];
    static char mDecBuffer[10];
    static char mOctBuffer[8];
    static SCursorState mCursorState;
    static SFileState mFileState;
};



inline SCursorState &CHexBuffer::cursorState( void )
{
  if( size() == 0 )
  {
    mCursorState.valid           = false;
    mCursorState.selectionOffset = 0;
    mCursorState.selectionSize   = 0;
    mCursorState.offset          = 0;
    mCursorState.cell            = 0;
    mCursorState.undoState       = 0;
    memset( mCursorState.data, 0, sizeof(mCursorState.data) );
    mCursorState.charValid       = false;
  }
  else
  {
    mCursorState.valid           = true;
    mCursorState.selectionOffset = mSelect.start();
    mCursorState.selectionSize   = mSelect.size();
    mCursorState.offset          = cursorOffset();
    mCursorState.cell            = cursorBit();
    mCursorState.undoState       = undoState();

    for( uint i = 0; i < sizeof( mCursorState.data ); i++ )
    {
      mCursorState.data[i] = (mCursorState.offset+i >= mDocumentSize) ? 0 : 
	(unsigned char)data()[mCursorState.offset+i];
    }

    mCursorState.charValid      = mCharValid[ mCursorState.data[0] ];
  }
  return( mCursorState );
}


inline void CHexBuffer::valueOnCursor( QByteArray &buf, uint size )
{
  int offset = cursorOffset();
  if( offset + size >= mDocumentSize )
  {
    size = mDocumentSize - offset;
  }

  buf.resize(size);
  for( uint i=0; i<buf.size(); i++)
  {
    buf[i] = (unsigned char)data()[offset+i];
  }
}


inline const SDisplayLayout &CHexBuffer::layout( void )
{
  return( mLayout );
}



inline const SDisplayInputMode &CHexBuffer::inputMode( void )
{
  return( mInputMode );
}

inline QPtrList<SCursorOffset> &CHexBuffer::bookmarkList( void )
{
  return( mBookmarkList );
}




inline SFileState &CHexBuffer::fileState( void )
{
  if( size() == 0 )
  {
    mFileState.valid    = false;
    mFileState.size     = 0;
    mFileState.modified = false;

  }
  else
  {
    mFileState.valid    = true;
    mFileState.size     = mDocumentSize;
    mFileState.modified = mDocumentModified;
  }

  return( mFileState );
}








inline bool CHexBuffer::modified( void )
{
  return( mDocumentModified );
}

inline uint CHexBuffer::undoState( void )
{
  return( (mUndoIndex > 0 ? UndoOk : 0) | 
	  (mUndoIndex < mUndoList.count() ? RedoOk : 0) );
}


inline uint CHexBuffer::documentSize( void )
{
  return( mDocumentSize );
}


inline CHexBuffer::EEditMode CHexBuffer::editMode( void )
{
  return( mEditMode );
}


inline const SEncodeState &CHexBuffer::encoding( void )
{
  return( mEncode.state() );
}

inline bool CHexBuffer::losslessEncoding( CConversion::EMode mode )
{
  return( mEncode.lossless(mode) );
}

inline QString &CHexBuffer::url( void )
{
  return( mUrl );
}

inline bool CHexBuffer::documentPresent( void )
{
  return( size() == 0 ? false : true );
}

inline void CHexBuffer::setUrl( const QString &url )
{
  mUrl = url;
}

inline const QDateTime &CHexBuffer::diskModifyTime( void )
{
  return( mDiskModifyTime );
}

inline uint CHexBuffer::calculateLine( uint offset )
{
  return( mLayout.lineSize == 0 ? 0 : offset / mLayout.lineSize );
}

inline const QColor &CHexBuffer::foregroundColor( uint column )  
{
  if( column > mLayout.lineSize )
  {
    return( Qt::black );
  }
  else
  {
    return( mColor.primaryFg[ mColorIndex[ column ] ] );
  }
}

inline bool CHexBuffer::selectionSet( uint offset, bool init )
{
  if( offset >= size() ) { offset = size() > 0 ? size() - 1 : 0; }

  if( init == true )
  {
    return( mSelect.init( offset ) );
  }
  else
  {
    return( mSelect.set( offset ) );
  }
}

inline void CHexBuffer::selectionSyncronize( void )
{
  mSelect.sync();
}

inline void CHexBuffer::selectionStartChange( uint &offset1, uint &offset2 )
{
  mSelect.startChange( offset1, offset2 );
}

inline void CHexBuffer::selectionStopChange( uint &offset1, uint &offset2 )
{
  mSelect.stopChange( offset1, offset2 );
}

inline bool CHexBuffer::cursorInsideSelection( void )
{
  return( mSelect.inside( cursorOffset() ) );
}

inline void CHexBuffer::markSet( uint offset, uint size )
{
  markSet( offset, true );
  markSet( offset+size, false );
  mMark.sync();
}

inline bool CHexBuffer::markSet( uint offset, bool init )
{
  if( offset >= size() ) { offset = size() > 0 ? size() - 1 : 0; }

  if( init == true )
  {
    return( mMark.init( offset ) );
  }
  else
  {
    return( mMark.set( offset ) );
  }
}

inline bool CHexBuffer::markRemove( void )
{
  return( mMark.init( mMark.start() ) );
}


inline void CHexBuffer::markSyncronize( void )
{
  mMark.sync();
}

inline void CHexBuffer::markStartChange( uint &offset1, uint &offset2 )
{
  mMark.startChange( offset1, offset2 );
}

inline void CHexBuffer::markStopChange( uint &offset1, uint &offset2 )
{
  mMark.stopChange( offset1, offset2 );
}


inline uint CHexBuffer::cursorOffset( void )
{
  return( mCursor.curr.offset );
}

inline uint CHexBuffer::cursorBit( void )
{
  return( mCursor.bit() );
}


inline uint CHexBuffer::cursorLine( void )
{
  return( mCursor.curr.y / lineHeight() );
}

inline uint CHexBuffer::prevCursorLine( void )
{
  return( mCursor.prev.y / lineHeight() );
}

inline SCursor &CHexBuffer::cursor( void )
{
  return( mCursor );
}


inline void CHexBuffer::currCursor( EEditArea editArea, SCursorPosition &p )
{
  if( editArea == edit_primary )
  {
    if( mActiveEditor == edit_primary )
    {
      p.x = mCursor.curr.drawX1();
      p.w = mCursor.curr.width1();
    }
    else
    {
      p.x = mCursor.curr.drawX2();
      p.w = mUnitWidth;
    }
  }
  else
  {
    if( mActiveEditor == edit_primary )
    {
      p.x = mCursor.curr.drawX2();
      p.w = mUnitWidth;
    }
    else
    {
      p.x = mCursor.curr.drawX1();
      p.w = mUnitWidth * mNumCell;
    }
  }

  p.x -= mStartX;
  p.y = mCursor.curr.y - mStartY;
  p.h = lineHeight();

}

inline void CHexBuffer::prevCursor( EEditArea editArea, SCursorPosition &p )
{
  if( editArea == edit_primary )
  {
    if( mActiveEditor == edit_primary )
    {
      p.x = mCursor.prev.drawX1();
      p.w = mUnitWidth * mNumCell;
    }
    else
    {
      p.x = mCursor.prev.drawX2();
      p.w = mUnitWidth;
    }
  }
  else
  {
    if( mActiveEditor == edit_primary )
    {
      p.x = mCursor.prev.drawX2();
      p.w = mUnitWidth;
    }
    else
    {
      p.x = mCursor.prev.drawX1();
      p.w = mUnitWidth * mNumCell;
    }
  }

  p.x -= mStartX;
  p.y = mCursor.prev.y - mStartY;
  p.h = lineHeight();
}






inline void CHexBuffer::cursorUp( uint lines )
{
  mCursor.up( lines );
  cursorCompute();
}

inline void CHexBuffer::cursorDown( uint lines )
{
  mCursor.down( lines );
  cursorCompute();
}


inline void CHexBuffer::cursorRight( bool cellLevel )
{
  if( cellLevel == true && mActiveEditor == edit_primary )
  {
    mCursor.incCell();
  }
  else
  {
    mCursor.addOffset( 1 );
  }
  cursorCompute();
}


inline void CHexBuffer::cursorStep( uint size, bool forward, bool modulo )
{
  if( forward == true )
  {
    if( modulo == true )
    {
      uint offset = mCursor.getOffset() + size;
      mCursor.setOffset( offset - offset % size );
    }
    else
    {
      mCursor.addOffset( size );
    }
  }
  else
  {
    if( modulo == true )
    {
      uint offset = mCursor.getOffset();
      if( offset % size )
      {
	mCursor.decOffset( offset % size, false );
      }
      else
      {
	mCursor.setOffset( offset < size ? 0 : offset - size );
      }
    }
    else
    {
      mCursor.decOffset( size, false );
    }
  }
  cursorCompute();
}


inline void CHexBuffer::cursorLeft( bool cellLevel )
{
  if( cellLevel == true && mActiveEditor == edit_primary )
  {
    mCursor.decCell();
  }
  else
  {
    mCursor.decOffset( 1, false );
  }
  cursorCompute();
}

inline void CHexBuffer::cursorHome( bool toExtreme )
{
  mCursor.home( toExtreme );
  cursorCompute();
}

inline void CHexBuffer::cursorEnd( bool toExtreme )
{
  mCursor.end( toExtreme );
  cursorCompute();
}


inline void CHexBuffer::cursorGoto( uint offset, uint bit, bool backward, 
				    bool fromCursor )
{
  uint maxOffset = mFixedSizeMode == true ? mMaximumSize-1 : documentSize();
  mCursor.setOffset( offset, bit, backward, fromCursor, maxOffset );
  cursorCompute();
}

inline void CHexBuffer::cursorGoto( uint offset, uint bit )
{
  mCursor.setOffset( offset );
  mCursor.setBit( bit );
  cursorCompute();
}

inline bool CHexBuffer::cursorChanged( void )
{
  return( mCursor.changed() );
}

inline void CHexBuffer::cursorResetEditArea( void )
{
  mCursor.setArea( edit_none );
}

inline bool CHexBuffer::cursorPrimaryEdit( void )
{
  return( mActiveEditor == edit_primary ? true : false );
}

inline int CHexBuffer::cursorFixedPosition( int position, int height )
{
  position += mCursor.curr.y - mCursor.prev.y;
  if( position < 0 )
  {
    return( 0 );
  }
  else if( position + height > totalHeight() )
  {
    return( height > totalHeight() ? 0 : totalHeight() - height );
  }
  else
  {
    if( mCursor.curr.y < position )
    {
      return( mCursor.curr.y );
    }
    else if( mCursor.curr.y > position + height )
    {
      return( mCursor.curr.y - height + lineHeight() );
    }
    else
    {
      return( position );
    }
  }
}

inline int CHexBuffer::cursorChangePosition( int position, int height )
{
  if( mCursor.curr.y < position || mCursor.curr.y > position + height )
  {
    // When cursor is at top of window
    position = mCursor.curr.y;
  }
  else if( mCursor.curr.y > position + height - lineHeight() )
  {
    // When cursor is at bottom of window
    position = mCursor.curr.y - height + lineHeight();
  }

  return( position );
}

inline int CHexBuffer::printDummyCell( char *buf, unsigned char )
{
  buf[0] = 0;
  return( 0 );
}

inline int CHexBuffer::printHexadecimalBigCell( char *buf, unsigned char data )
{
  buf[0] = mHexBigBuffer[ (data>>4)&0x0F ];
  buf[1] = mHexBigBuffer[ data&0x0F ];
  return( 0 );
}

inline int CHexBuffer::printHexadecimalSmallCell( char *buf, 
						  unsigned char data )
{
  buf[0] = mHexSmallBuffer[ (data>>4)&0x0F ];
  buf[1] = mHexSmallBuffer[ data&0x0F ];
  return( 0 );
}


inline int CHexBuffer::printDecimalCell( char *buf, unsigned char data )
{
  buf[0] = mDecBuffer[ data/100 ];
  data -= (data/100) * 100;
  buf[1] = mDecBuffer[ data/10 ];
  data -= (data/10) * 10;
  buf[2] = mDecBuffer[ data ];
  return( 0 );
}

inline int CHexBuffer::printOctalCell( char *buf, unsigned char data )
{
  buf[0] = mOctBuffer[ (data>>6)&0x07 ];
  buf[1] = mOctBuffer[ (data>>3)&0x07 ];
  buf[2] = mOctBuffer[ data&0x07 ];
  return( 0 );
}

inline int CHexBuffer::printBinaryCell( char *buf, unsigned char data )
{
  for( int i = 0; i < 8; i++ )
  {
    buf[7-i] = (data&(1<<i)) ? '1' : '0';
  }
  return( 0 );
}

inline int CHexBuffer::printAsciiCell( char *buf, unsigned char data )
{
  if( mCharValid[data] == 0 )
  {
    buf[0] = mFontInfo.nonPrintChar;
    return( 1 );
  }
  else
  {
    buf[0] = data;
    return( 0 );
  }
}

inline void CHexBuffer::printDummyOffset( char *buf, uint /*offset*/ )
{
  buf[0] = 0;
}

inline void CHexBuffer::printHexadecimalBigOffset( char *buf, uint offset )
{
  sprintf( buf, "%04X:%04X", offset>>16, offset&0x0000FFFF );
}

inline void CHexBuffer::printHexadecimalSmallOffset( char *buf, uint offset )
{
  sprintf( buf, "%04x:%04x", offset>>16, offset&0x0000FFFF );
}

inline void CHexBuffer::printDecimalOffset( char *buf, uint offset )
{
  sprintf( buf, "%010u", offset );
}

inline int CHexBuffer::lineSize( void )
{
  return( mLayout.lineSize );
}

inline int CHexBuffer::lineHeight( void )
{
  return( mFontHeight + mLayout.horzGridWidth );
}

inline int CHexBuffer::fontAscent( void )
{
  return( mFontAscent );
}

inline int CHexBuffer::lineWidth( void )
{
  return( mLineWidth );
}

inline int CHexBuffer::unitWidth( void )
{
  return( mUnitWidth );
}

inline int CHexBuffer::numLines( void )
{
  return( mNumLines );
}

inline int CHexBuffer::totalHeight( void )
{
  return( mNumLines * (mFontHeight+mLayout.horzGridWidth) );
}

inline const QFont &CHexBuffer::font( void )
{
  return( mFontInfo.font );
}

inline SCursor *CHexBuffer::textCursor( void )
{
  return( &mCursor );
}

inline const QColor &CHexBuffer::backgroundColor( void )
{
  return( documentPresent() == true ? mColor.textBg : mColor.inactiveBg );
}

inline int CHexBuffer::startX( void )
{
  return( mStartX );
} 

inline int CHexBuffer::startY( void )
{
  return( mStartY );
}

inline void CHexBuffer::setStartX( int val )
{
  mStartX = val;
}

inline void CHexBuffer::setStartY( int val )
{
  mStartY = val;
}



#endif

