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

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <qfileinfo.h>

#include <klocale.h>
#include <kglobal.h>
#include <knotifyclient.h>

#include "hexbuffer.h"
#include "hexerror.h"

//
// There are some comments marked with a "// ##" at various places.
// These indicate a patch from Sergey A. Sukiyazov which I have applied
// "as is" for now. The number of QString::fromLocal8Bit in this modification
// indicates that I should perhaps modify code elsewhere as well 
// (espen 2000-11-26)
//

// #define DEBUG_FIXED_SIZE 1024
// #define PRINTER_TEST


CHexAction::CHexAction( HexAction action, uint offset )
{
  mAction   = action;
  mOffset   = offset;
  mSize     = 0;
  mData     = 0;
  mDataSize = 0;
  mNext     = 0;
}

CHexAction::~CHexAction( void )
{
  delete [] mData;
}

void CHexAction::setData( uint size, char *data, uint dataSize )
{

  if( data != 0 && dataSize > 0 )
  {
    mData = new char[ dataSize ];
    if( mData == 0 )
    {
      return;
    }
    memcpy( mData, data, dataSize );
    mDataSize = dataSize;
  }
  else
  {
    mDataSize = 0;
    mData = 0;
  }
  mSize = size;
}


CHexActionGroup::CHexActionGroup( uint startOffset, uint startBit )
{
  mStartOffset = startOffset;
  mStartBit    = startBit;
  mHexAction   = 0;
}

CHexActionGroup::~CHexActionGroup( void )
{
  CHexAction *ptr = mHexAction;
  while( ptr != 0 )
  {
    CHexAction *next = ptr->mNext;
    delete ptr;
    ptr = next;
  }
}

void CHexActionGroup::insertAction( CHexAction *hexAction )
{
  hexAction->mNext = mHexAction;
  mHexAction = hexAction;
}



int SFilterControl::execute( uchar *dest, uchar *src, uint size )
{
  if( size == 0 )
  {
    return( Err_IllegalArgument );
  }

  uint numElement = operand.size();
  if( operation == OperandAndData )
  {
    if( numElement == 0 ) { return( Err_IllegalArgument ); }
    if( forward == true )
    {
      for( uint i = 0; i < size; )
      {
	for( uint j = 0; i < size && j < numElement; j++, i++ )
	{
	  dest[i] = src[i] & operand[j];
	}
      }
    }
    else
    {
      for( uint i = size; i > 0; )
      {
	for( uint j = numElement; i > 0 && j > 0; j--, i-- )
	{
	  dest[i-1] = src[i-1] & operand[j-1];
	}
      }
    }
  }
  else if( operation == OperandOrData )
  {
    if( numElement == 0 ) { return( Err_IllegalArgument ); }
    if( forward == true )
    {
      for( uint i = 0; i < size; )
      {
	for( uint j = 0; i < size && j < numElement; j++, i++ )
	{
	  dest[i] = src[i] | operand[j];
	}
      }
    }
    else
    {
      for( uint i = size; i > 0; )
      {
	for( uint j = numElement; i > 0 && j > 0; j--, i-- )
	{
	  dest[i-1] = src[i-1] | operand[j-1];
	}
      }
    }
  }
  else if( operation == OperandXorData )
  {
    if( numElement == 0 ) { return( Err_IllegalArgument ); }
    if( forward == true )
    {
      for( uint i = 0; i < size; )
      {
	for( uint j = 0; i < size && j < numElement; j++, i++ )
	{
	  dest[i] = src[i] ^ operand[j];
	}
      }
    }
    else
    {
      for( uint i = size; i > 0; )
      {
	for( uint j = numElement; i > 0 && j > 0; j--, i-- )
	{
	  dest[i-1] = src[i-1] ^ operand[j-1];
	}
      }
    }
  }
  else if( operation == InvertData )
  {
    for( uint i = 0; i < size; i++ )
    {
      dest[i] = ~src[i];
    }
  }
  else if( operation == ReverseData )
  {
    for( uint i = 0; i < size; i++ )
    {
      uchar flag = src[i];
      uchar rev  = 0;
      for( uint j = 0; j < 8; j++ )
      {
	rev |= (((flag & 0x80) >> (7-j)));
	flag <<= 1;
      }
      dest[i] = rev;
    }
  }
  else if( operation == RotateData || operation == ShiftData )
  {
    //
    // Only forward here
    //
    bool up   = rotate[1] > 0 ? true : false;
    int range = rotate[0];
    int shift = abs(rotate[1]);
    if( range == 0 || shift == 0 ) { return( Err_IllegalArgument ); }
    shift = shift % (range*8);

    int b = shift / 8;
    int s = shift - b * 8;

    for( uint i = 0; i < size; )
    {
      if( up == true )
      {
	int j;
	if( operation == RotateData )
	{
	  for( j=0; j < b && i+range < size ; i++, j++ )
	  {
	    dest[i] = src[i+range-b];
	  }
	}
	else
	{
	  for( j=0; j < b && i < size ; dest[i] = 0, i++, j++ );
	}
	for( ; j < range && i < size ; i++, j++ )
	{
	  dest[i] = src[i-b];
	}

	uchar last = dest[i-1];
	for( int k=1; k <= j; k++ )
	{
	  dest[i-k] >>= s;
	  if( k < j )
	  {
	    dest[i-k] |= dest[i-k-1]<<(8-s);
	  }
	  else if( j == range && operation == RotateData )
	  {
	    dest[i-k] |= last<<(8-s);
	  }
	}
      }
      else
      {
	int j;
	for( j=0; j+b < range && i+b < size ; i++, j++ )
	{
	  dest[i] = src[i+b];
	}
	for( ; j < range && i < size ; i++, j++ )
	{
	  dest[i] = operation == RotateData ? src[i+b-range] : 0;
	}
	
	uchar first = dest[i-j];
	for( int k=j; k>0; k-- )
	{
	  dest[i-k] <<= s;
	  if( k>1 )
	  {
	    dest[i-k] |= dest[i-k+1]>>(8-s);
	  }
	  else if( j == range && operation == RotateData )
	  {
	    dest[i-k] |= first>>(8-s);
	  }
	}
      }
    }
  }
  else if( operation == SwapBits )
  {
    //
    // Swap bits. Based on Leon Lessing's work.
    //

    //
    // Make non swapped version first.
    //
    for( uint i = 0; i < size; i++ )
    {
      dest[i] = src[i];
    }

    //
    // Swap the pairs the have been defined
    // Format of operand (example):
    // 7 2 5 0 0 0 0 0
    // Swap bit 7 with bit 2 and swap bit 5 with bit 0
    //
    for( uint j=0; j<4; j++ )
    {
      uchar b1 = 1 << (uchar)operand[j*2];
      uchar b2 = 1 << (uchar)operand[j*2+1];
      if( b1 == b2 ) { continue; } // Equal, no need to swap.

      for( uint i = 0; i < size; i++ )
      {
	uchar b = 0;
	if( dest[i] & b1 ) { b |= b2; }
	if( dest[i] & b2 ) { b |= b1; }

	//
	// A short description so that I will understand what the
	// h... is going on five minutes from now.
	//
	// Destination byte is masked (AND'ed) with the inverse bitmap
	// (the noninversed bitmap contains position of the
	// two swap bits, eg 7-2 gives 10000100). Then the destination
	// is OR'ed with the swapped bitmap.
	//
	dest[i] = (dest[i] & ~(b1 | b2)) | b;
      }
    }
  }
  else
  {
    return( Err_IllegalArgument );
  }

  return( Err_Success );
}


const char *SExportCArray::printFormatted( const char *b, uint maxSize ) const
{
  static char buf[12];
  if( elementType == Char )
  {
    char e = 0;
    memcpy( &e, b, QMIN(sizeof(e),maxSize) );
    sprintf( buf, "%d", e );
    return( buf );
  }
  else if( elementType == Uchar )
  {
    unsigned char e = 0;
    memcpy( &e, b, QMIN(sizeof(e),maxSize) );
    if( unsignedAsHexadecimal == true )
    {
      sprintf( buf, "0x%02x", e );
    }
    else
    {
      sprintf( buf, "%u", e );
    }
    return( buf );
  }
  else if( elementType == Short )
  {
    short e = 0;
    memcpy( &e, b, QMIN(sizeof(e),maxSize) );
    sprintf( buf, "%d", e );
    return( buf );

  }
  else if( elementType == Ushort )
  {
    unsigned short e = 0;
    memcpy( &e, b, QMIN(sizeof(e),maxSize) );
    if( unsignedAsHexadecimal == true )
    {
      sprintf( buf, "0x%04x", e );
    }
    else
    {
      sprintf( buf, "%u", e );
    }
    return( buf );
  }
  else if( elementType == Int )
  {
    int e = 0;
    memcpy( &e, b, QMIN(sizeof(e),maxSize) );
    sprintf( buf, "%u", e );
    return( buf );
  }
  else if( elementType == Uint )
  {
    unsigned int e = 0;
    memcpy( &e, b, QMIN(sizeof(e),maxSize) );
    if( unsignedAsHexadecimal == true )
    {
      sprintf( buf, "0x%08x", e );
    }
    else
    {
      sprintf( buf, "%u", e );
    }
    return( buf );
  }
  else if( elementType == Float )
  {
    float e = 0;
    memcpy( &e, b, QMIN(sizeof(e),maxSize) );
    sprintf( buf, "%f", e );
    return( buf );
  }
  else if( elementType == Double )
  {
    double e = 0;
    memcpy( &e, b, QMIN(sizeof(e),maxSize) );
    sprintf( buf, "%f", e );
    return( buf );
  }

  else
  {
    return("");
  }
}


QString SExportCArray::variableName( uint range ) const
{
  const char *typeString[] =
  {
    "char",
    "unsigned char",
    "short",
    "unsigned short",
    "int",
    "unsigned int",
    "float",
    "double"
  };

  uint es = elementSize();
  uint numElement = range / es + ((range % es) ? 1 : 0);

  return( QString("%1 %2[%2]").arg(typeString[elementType]).
	  arg(arrayName).arg(numElement) );
}



int SExportCArray::elementSize( void ) const
{
  if( elementType == Char || elementType == Uchar )
  {
    return( sizeof(char) );
  }
  else if( elementType == Short || elementType == Ushort )
  {
    return( sizeof(short) );
  }
  else if( elementType == Int || elementType == Uint )
  {
    return( sizeof(int) );
  }
  else if( elementType == Float )
  {
    return( sizeof(float) );
  }
  else if( elementType == Double )
  {
    return( sizeof(double) );
  }
  else
  {
    return(1);
  }
}


char CHexBuffer::mHexBigBuffer[16]=
{
  '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
};

char CHexBuffer::mHexSmallBuffer[16]=
{
  '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'
};

char CHexBuffer::mDecBuffer[10]=
{
  '0','1','2','3','4','5','6','7','8','9'
};

char CHexBuffer::mOctBuffer[8]=
{
  '0','1','2','3','4','5','6','7'
};


SCursorState CHexBuffer::mCursorState;
SFileState CHexBuffer::mFileState;




CHexBuffer::CHexBuffer( void )
  :QByteArray()
{
  mColorIndex   = 0;
  mPrintBuf     = 0;
  mLoadingData  = false;
  mEditMode     = EditReplace;
  mActiveEditor = edit_primary;

  mDocumentModified = false;

  #ifdef DEBUG_FIXED_SIZE
  setMaximumSize( DEBUG_FIXED_SIZE );
  #else
  setMaximumSize( ~0 );
  #endif

  setDocumentSize(0);

  #ifdef PRINTER_TEST
  puts("<CHexBuffer> Printer test is activated");
  #endif


  setInputMode( mInputMode );

  int errCode = setLayout( mLayout );
  if( errCode != 0 )
  {
    return;
  }
  setColor( mColor );
  setFont( mFontInfo.init() );
  setShowCursor( true );
  setDisableCursor( false );
  setEditMode( EditReplace, false, false );
  setSoundState( false, false );

  mUndoLimit = 10;
  mUndoIndex = 0;
  mUndoList.setAutoDelete( TRUE );
  mBookmarkList.setAutoDelete( TRUE );
}


CHexBuffer::~CHexBuffer( void )
{
  //debug("CHexBuffer::~CHexBuffer");
  delete [] mColorIndex;
  delete [] mPrintBuf;
}



bool CHexBuffer::hasFileName( void )
{
  //
  // FIXME: Files can be called "Untitled" so this must be corrected.
  //
  if( mUrl.isEmpty() || mUrl.contains( i18n( "Untitled" ), false ) )
  {
    return( false );
  }
  else
  {
    return( true );
  }
}



int CHexBuffer::setLayout( SDisplayLayout &layout )
{
  mLayout = layout;
  mLayout.verify();

  if( mLayout.primaryMode == SDisplayLayout::textOnly )
  {
    mActiveEditor = edit_primary;
    setEditMode( mEditMode );
  }

  mCursor.setLineSize( mLayout.lineSize );
  mCursor.addOffset( 0 ); // This will only reset the cell position

  computeLineWidth();
  cursorCompute();

  delete [] mColorIndex; mColorIndex = 0;
  delete [] mPrintBuf; mPrintBuf = 0;

  mColorIndex = new unsigned char[ mLayout.lineSize ];
  if( mColorIndex == 0 )
  {
    return( Err_NoMemory );
  }
  setColor( mColor );

  //
  // The 'mPrintBuf' is used to store formatted text. It is used for all
  // print operations and must have the size of the 'mDpyState.lineSize' which
  // is the number of bytes in one single display line.
  //
  mPrintBuf = new char[ mLayout.lineSize < 12 ? 12 : mLayout.lineSize ];
  if( mPrintBuf == 0 )
  {
    delete [] mColorIndex; mColorIndex = 0;
    return( Err_NoMemory );
  }

  return( Err_Success );
}


void CHexBuffer::setColor( SDisplayColor &color )
{
  mColor = color;

  //
  // Test...
  //
  //mColor.secondTextBg = Qt::yellow;
  //mColor.offsetBg = Qt::lightGray;
  //mColor.gridFg = Qt::darkCyan;

  /*
  mColor.secondTextBg = mColor.textBg;
  mColor.offsetBg = mColor.textBg;
  mColor.gridFg = mColor.textBg;
  */

  if( mColorIndex != 0 )
  {
    uint columnSize = mLayout.columnSize == 0 ? 1 : mLayout.columnSize;
    for( uint i = 0, entry = 0;  i < mLayout.lineSize; i++ )
    {
      if( i > 0 && i % columnSize == 0 ) { entry = entry == 0 ? 1 : 0; }
      mColorIndex[i] = entry;
    }
  }
}

void CHexBuffer::setInputMode( SDisplayInputMode &mode )
{
  mInputMode = mode;
  if( mInputMode.allowResize == false && mEditMode != EditReplace )
  {
    setEditMode( EditReplace );
  }
}


bool CHexBuffer::toggleEditor( void )
{
  bool changed;
  if( mLayout.secondaryMode == SDisplayLayout::hide )
  {
    changed = mActiveEditor == edit_secondary ? true : false;
    mActiveEditor = edit_primary;
  }
  else
  {
    changed = true;
    mActiveEditor = mActiveEditor == edit_primary ?
      edit_secondary : edit_primary;
  }

  setEditMode( mEditMode ); // Sets the cursor shapes as well

  if( changed == true )
  {
    mCursor.resetCell();
    cursorCompute();
  }

  return( changed );
}







bool CHexBuffer::matchWidth( uint width )
{
  if( documentPresent() == false || (uint)mFixedWidth >= width )
  {
    return( false );
  }

  width -= mFixedWidth;

  uint  g = mLayout.columnSpacing == 0 ? 1 : mLayout.columnSize;
  uint  n = g * mNumCell;
  uint  u = mUnitWidth;
  uint  s = mLayout.secondaryMode == SDisplayLayout::hide ? 0 : g;
  uint  o = mLayout.columnSpacing == 0 ? 0 : mSplitWidth;
  float x = (float)(width+o)/(float)(u*(n+s)+o);

  uint lineSize = (uint)x * g;

  if( mLayout.lockColumn == false )
  {
    //
    // Examine if we can add one or more entries from the next column. This
    // will make the rightmost column smaller than the rest but we will
    // utilize as much of the available space (ie., width) as possible.
    // (Note that the entry itself (which represents one byte of filedata)
    // can not be splitted, eg., in binary mode the entry is eight byte
    // wide and will not be splitted).
    //
    int w = (int)((float)((int)x)* (float)(u*(n+s)+o) - (float)o);
    if( w > 0 && (uint)w < width )
    {
      width -= w;
      if( width > o )
      {
	x = (float)(width-o) / (float)(u*(mNumCell+1));
	lineSize += (uint)x;
      }
    }
  }

  if( lineSize == 0 || lineSize == mLayout.lineSize )
  {
    //
    // We have to redraw all text if a change occurs so we avoid it if
    // possible.
    //
    return( false );
  }

  mLayout.lineSize  = lineSize;
  setLayout( mLayout );
  return( true );
}


void CHexBuffer::setNonPrintChar( QChar nonPrintChar )
{
  mFontInfo.nonPrintChar = nonPrintChar;
}


void CHexBuffer::setShowCursor( bool showCursor )
{
  mShowCursor = showCursor;
}


void CHexBuffer::setDisableCursor( bool disableCursor )
{
  mDisableCursor = disableCursor;
}


void CHexBuffer::setCursorShapeModifier( bool alwaysBlock, bool thickInsert )
{
  mCursor.setShapeModifier( alwaysBlock, thickInsert );
  setEditMode( mEditMode );
}

void CHexBuffer::setEditMode( EEditMode editMode, bool alwaysBlock,
			      bool thickInsert )
{
  mCursor.setShapeModifier( alwaysBlock, thickInsert );
  setEditMode( editMode );
}

void CHexBuffer::setEditMode( EEditMode editMode )
{
  mEditMode = editMode;
  if( mEditMode == EditInsert )
  {
    if( mActiveEditor == edit_primary )
    {
      mCursor.setShape( SCursorSpec::thin, SCursorSpec::frame, mUnitWidth,
			mNumCell );
    }
    else
    {
      mCursor.setShape( SCursorSpec::frame, SCursorSpec::thin, mUnitWidth,
			mNumCell );
    }

  }
  else
  {
    if( mActiveEditor == edit_primary )
    {
      mCursor.setShape( SCursorSpec::solid, SCursorSpec::frame, mUnitWidth,
			mNumCell );
    }
    else
    {
      mCursor.setShape( SCursorSpec::frame, SCursorSpec::solid, mUnitWidth,
			mNumCell );
    }
  }
}



void CHexBuffer::setMaximumSize( uint maximumSize )
{
  if( maximumSize == 0 ) { maximumSize = ~0; }

  mMaximumSize   = maximumSize;
  mFixedSizeMode = maximumSize == (uint)~0 ? false : true;
  mCursor.setFixedSizeMode( mFixedSizeMode );

  if( mLayout.offsetVisible == false )
  {
    mOffsetSize  = 0;
    mOffsetIndex = 0;
    printOffset = &CHexBuffer::printDummyOffset;
  }
  else
  {
    if( mLayout.offsetMode ==  SDisplayLayout::decimal )
    {
      printOffset = &CHexBuffer::printDecimalOffset;
      for( mOffsetSize=0; maximumSize > 0; mOffsetSize += 1 )
      {
	maximumSize = maximumSize / 10;
      }
      mOffsetIndex = 10 - mOffsetSize;
    }
    else if( mLayout.offsetMode == SDisplayLayout::hexadecimal )
    {
      if( mLayout.offsetUpperCase == true )
      {
	printOffset = &CHexBuffer::printHexadecimalBigOffset;
      }
      else
      {
	printOffset = &CHexBuffer::printHexadecimalSmallOffset;
      }
      for( mOffsetSize=0; maximumSize > 0; mOffsetSize += 1 )
      {
	maximumSize = maximumSize / 16;
      }
      if( mOffsetSize > 4 ) { mOffsetSize += 1; } // Space for the ':' sign
      mOffsetIndex = 9 - mOffsetSize;
    }
    else
    {
      mLayout.offsetVisible = false;
      mOffsetSize  = 0;
      mOffsetIndex = 0;
      printOffset = &CHexBuffer::printDummyOffset;
    }
  }
}


void CHexBuffer::setDocumentSize( uint size )
{
  if( size > mMaximumSize ) { size = mMaximumSize; }
  mDocumentSize = size;
  mCursor.setDocumentSize( size );
  updateBookmarkMap(true);
}


void CHexBuffer::setUndoLevel( uint level )
{
  if( level < 10 ) { level = 10; }

  if( level >= mUndoLimit )
  {
    mUndoLimit = level;
    return;
  }
  else
  {
    //
    // The maximum size decreases. If the list is larger than the the new
    // limit, then reduce the list size starting with the oldest elements.
    //
    mUndoLimit = level;
    while( mUndoList.count() >= mUndoLimit )
    {
      mUndoList.removeFirst();
      mUndoIndex -= (mUndoIndex > 0 ? 1 : 0);
    }
  }
}


void CHexBuffer::setSoundState( bool inputSound, bool fatalSound )
{
  mInputErrorSound = inputSound;
  mFatalErrorSound = fatalSound;
}


void CHexBuffer::setBookmarkVisibility( bool showInColumn, bool showInEditor )
{
  mShowBookmarkInOffsetColumn = showInColumn;
  mShowBookmarkInEditor = showInEditor;
}

int CHexBuffer::writeFile( QFile &file, CProgress &p )
{
  uint offset = 0;
  uint remaining = documentSize();

  do
  {
    uint blockSize = remaining > 100000 ? 100000 : remaining;
    int writeSize = file.writeBlock( &data()[offset], blockSize );
    if( writeSize == -1 )
    {
      p.finish();
      return( Err_ReadFailed );
    }
    offset    += blockSize;
    remaining -= blockSize;

    if( p.expired() == true )
    {
      int errCode = p.step( (float)offset/(float)documentSize() );
      if( errCode == Err_Stop && remaining > 0 )
      {
	p.finish();
	return( Err_Success );
      }
    }
  }
  while( remaining > 0 );

  p.finish();
  mDocumentModified = false;
  registerDiskModifyTime( file );

  return( Err_Success );
}


int CHexBuffer::readFile( QFile &file, const QString &url, CProgress &p )
{
  if( resize( file.size() + 100 ) == false )
  {
    p.finish();
    return( Err_NoMemory );
  }

  if( file.size() > 0 )
  {
    mLoadingData = true;
    uint offset = 0;
    uint remaining = file.size();
    while( remaining > 0 )
    {
      uint blockSize = remaining > 100000 ? 100000 : remaining;
      int readSize = file.readBlock( &data()[offset], blockSize );
      if( readSize == -1 )
      {
        p.finish();
	mLoadingData = false;
	return( Err_ReadFailed );
      }
      for( uint i=0; i<blockSize; i++)
      {
	data()[offset+i] = mEncode[ (unsigned char) data()[offset+i] ];
      }

      offset    += blockSize;
      remaining -= blockSize;

      if( p.expired() == true )
      {
	int errCode = p.step( (float)offset/(float)file.size() );
	if( errCode == Err_Stop && remaining > 0 )
	{
	  p.finish();
	  return( Err_OperationAborted );
	}
      }
    }
    mLoadingData = false;
  }

  p.finish();

  mDocumentModified = false;
  setDocumentSize( file.size() );
  registerDiskModifyTime( file );
  setUrl( url );
  computeNumLines();
  mSelect.reset();
  mMark.reset();
  mUndoList.clear();
  mUndoIndex = 0;

  return( Err_Success );
}


int CHexBuffer::insertFile( QFile &file, CProgress &p )
{
  if( file.size() == 0 )
  {
    p.finish();
    return( Err_Success );
  }

  QByteArray array( file.size() );
  if( array.isNull() == true )
  {
    p.finish();
    return( Err_NoMemory );
  }

  uint offset = 0;
  uint remaining = file.size();
  while( remaining > 0 )
  {
    uint blockSize = remaining > 100000 ? 100000 : remaining;
    int readSize = file.readBlock( &array[offset], blockSize );
    if( readSize == -1 )
    {
      p.finish();
      return( Err_ReadFailed );
    }
    for( uint i=0; i<blockSize; i++)
    {
      array[offset+i] = mEncode[ (unsigned char) array[offset+i] ];
    }

    offset    += blockSize;
    remaining -= blockSize;

    if( p.expired() == true )
    {
      int errCode = p.step( (float)offset/(float)file.size() );
      if( errCode == Err_Stop && remaining > 0 )
      {
	p.finish();
	return( Err_OperationAborted );
      }
    }
  }

  p.finish();

  int errCode = inputAtCursor( array, 0 );
  return( errCode );
}


int CHexBuffer::newFile( const QString &url )
{
  if( resize( 100 ) == 0 )
  {
    return( Err_NoMemory );
  }

  mDocumentModified = false;
  setDocumentSize( 0 );
  setUrl( url );
  computeNumLines();
  mSelect.reset();

  return( Err_Success );
}


void CHexBuffer::closeFile( void )
{
  resize(0);
  computeNumLines();

  mUndoList.clear();
  mUndoIndex = 0;

  setDocumentSize(0);
  mDocumentModified = false;

  QString emptyUrl;
  setUrl( emptyUrl );

  mSelect.reset();
  mMark.reset();

  removeBookmark(-1); // Negative index - All bookmarks
}


void CHexBuffer::registerDiskModifyTime( const QFile &file )
{
  QFileInfo fileInfo( file );
  mDiskModifyTime = fileInfo.lastModified();
}



void CHexBuffer::setFont( const SDisplayFontInfo &fontInfo )
{
  mFontInfo = fontInfo;
  QFontMetrics fm( mFontInfo.font );
  mFontHeight = fm.height();
  mFontAscent = fm.ascent();
  computeLineWidth();

  for( int i=0; i < 256; i++ )
  {
    mCharValid[i] = QChar(i).isPrint();
  }

  /*
  QFontInfo info( mFontInfo.font );
  puts("CHexBuffer mCharValid broken");

  KCharset charset( info.charSet() );
  for( int i=0; i < 256; i++ )
  {
    mCharValid[i] = charset.printable(i);
  }
  */
}


int CHexBuffer::setEncoding( CConversion::EMode mode, CProgress &p )
{
  int errCode = mEncode.convert( *this, mode, p );
  if( errCode == Err_Success )
  {
    //
    // The cursor stores the byte it is "covering", so this information
    // must be updated.
    //
    cursorCompute();
  }

  return( errCode );
}




void CHexBuffer::computeLineWidth( void )
{
  QFontMetrics fm( mFontInfo.font );
  mUnitWidth  = fm.width( "M" );

  if( mLayout.primaryMode == SDisplayLayout::textOnly )
  {
    mSplitWidth = 0;
  }
  else if( mLayout.columnCharSpace == true )
  {
    mSplitWidth = mUnitWidth;
  }
  else
  {
    mSplitWidth = mLayout.columnSpacing;
  }

  setMaximumSize( mMaximumSize );

  if( mLayout.primaryMode == SDisplayLayout::hexadecimal )
  {
    mNumCell = 2;
    mCursor.setCellWeight( 4 );
    if( mLayout.primaryUpperCase == true )
    {
      printCell = &CHexBuffer::printHexadecimalBigCell;
      inputCell = &CHexBuffer::inputHexadecimal;
    }
    else
    {
      printCell = &CHexBuffer::printHexadecimalSmallCell;
      inputCell = &CHexBuffer::inputHexadecimal;
    }
  }
  else if( mLayout.primaryMode == SDisplayLayout::decimal )
  {
    mNumCell  = 3;
    printCell = &CHexBuffer::printDecimalCell;
    inputCell = &CHexBuffer::inputDecimal;
    mCursor.setCellWeight( 3 );
  }
  else if( mLayout.primaryMode == SDisplayLayout::octal )
  {
    mNumCell  = 3;
    printCell = &CHexBuffer::printOctalCell;
    inputCell = &CHexBuffer::inputOctal;
    mCursor.setCellWeight( 3 );
  }
  else if( mLayout.primaryMode == SDisplayLayout::binary )
  {
    mNumCell  = 8;
    printCell = &CHexBuffer::printBinaryCell;
    inputCell = &CHexBuffer::inputBinary;
    mCursor.setCellWeight( 1 );
  }
  else if( mLayout.primaryMode == SDisplayLayout::textOnly )
  {
    mNumCell  = 1;
    printCell = &CHexBuffer::printAsciiCell;
    inputCell = &CHexBuffer::inputAscii;
    mCursor.setCellWeight( 8 );
  }
  else
  {
    mNumCell  = 2;
    mLayout.primaryMode = SDisplayLayout::hexadecimal;
    mLayout.primaryUpperCase = false;
    printCell = &CHexBuffer::printHexadecimalSmallCell;
    inputCell = &CHexBuffer::inputHexadecimal;
    mCursor.setCellWeight( 4 );
  }

  //
  // 'mPrimaryWidth' is the number of pixels that are needed to display a
  // line in the primary field.
  //
  mPrimaryWidth = mLayout.lineSize * mNumCell * mUnitWidth;

  if( mLayout.columnSpacing != 0 )
  {
    int numSplit = mLayout.lineSize / mLayout.columnSize;
    numSplit -= mLayout.lineSize % mLayout.columnSize == 0 ? 1 : 0;
    mPrimaryWidth += numSplit * mSplitWidth;
  }

  //
  // 'mSecondaryWidth' is the number of pixels that are needed to display a
  // line in the secondary field (there are no spaces).
  //
  if( mLayout.secondaryMode == SDisplayLayout::hide )
  {
    mSecondaryWidth = 0;
  }
  else
  {
    mSecondaryWidth = mLayout.lineSize * mUnitWidth;
  }

  //
  // 'mLineWidth' is the total number of pixels required to display
  // offset data, separators, primary and secondary data on a line.
  //
  mLineWidth = mPrimaryWidth + mSecondaryWidth + mOffsetSize * mUnitWidth;

  //
  // The 'mFixedWidth' is the number of pixels of the width that stays the
  // same regardless of how many characters that are displayed.
  // This entity consists of the edge margins, the inner margins and the
  // separators.
  //
  mFixedWidth = mOffsetSize * mUnitWidth;

  //
  // The edge margin is always present in both ends.
  //
  mLineWidth  += mLayout.edgeMarginWidth * 2;
  mFixedWidth += mLayout.edgeMarginWidth * 2;

  //
  // 'mTextStart1' is the number of pixels from the left edge where the
  // primary field starts.
  //
  mTextStart1 = mLayout.edgeMarginWidth;
  if( mLayout.offsetVisible == true )
  {
    int width;
    if( mLayout.leftSeparatorWidth > 0 )
    {
      width = mLayout.separatorMarginWidth * 2 + mLayout.leftSeparatorWidth;
    }
    else
    {
      width = (mLayout.separatorMarginWidth * 3) / 2;
    }

    mLineWidth  += width;
    mFixedWidth += width;
    mTextStart1 += width + mOffsetSize * mUnitWidth;
  }

  //
  // 'mTextStart2' is the number of pixels from the left edge where the
  // secondary fields start.
  //
  mTextStart2 = mTextStart1;
  if( mLayout.secondaryMode != SDisplayLayout::hide )
  {
    int width;
    if( mLayout.rightSeparatorWidth > 0 )
    {
      width = mLayout.separatorMarginWidth * 2 + mLayout.rightSeparatorWidth;
    }
    else
    {
      width = (mLayout.separatorMarginWidth * 3) / 2;
    }

    mLineWidth  += width;
    mFixedWidth += width;
    mTextStart2 += width + mPrimaryWidth;
  }

  setEditMode( mEditMode );
  computeNumLines();
}


void CHexBuffer::computeNumLines( void )
{
  if( mLayout.lineSize == 0 )
  {
    mNumLines = 1;
  }
  else
  {
    uint s = mFixedSizeMode == true ? mMaximumSize : documentSize() + 1;
    mNumLines = s / mLayout.lineSize + (s % mLayout.lineSize ? 1 : 0);
  }
}



void CHexBuffer::drawSelection( QPainter &paint, QColor &color, uint start,
				uint stop, int sx )
{
  if( start >= stop ) { return; }
  uint width = stop - start;

  uint addStart, addWidth;
  addStart = (start / mLayout.columnSize) * mSplitWidth;
  if( width == 0 )
  {
    addWidth = 0;
  }
  else
  {
    uint g = mLayout.columnSize;
    addWidth = (((start % g) + width - 1) / g) * mSplitWidth;
  }

  int offset = mTextStart1 - sx;
  paint.fillRect( offset + start * mNumCell * mUnitWidth + addStart,
		  0, width * mNumCell * mUnitWidth + addWidth,
		  mFontHeight, color );

  if( mLayout.secondaryMode != SDisplayLayout::hide )
  {
    offset = mTextStart2 - sx;
    paint.fillRect( offset + start * mUnitWidth,
		    0, width * mUnitWidth,
		    mFontHeight, color );
  }
}





void CHexBuffer::drawText( QPainter &paint, uint line, int sx, int x1, int x2 )
{
  uint fileOffset = line * mLayout.lineSize;
  if( documentPresent() == false || mLoadingData == true )
  {
    paint.fillRect( x1, 0, x2-x1, lineHeight(), mColor.inactiveBg );
    return;
  }

  bool outsideText;
  if( size() == 0 || fileOffset > documentSize() || fileOffset >= mMaximumSize)
  {
    outsideText = true;
  }
  else
  {
    outsideText = false;
  }

  if( (line+1) % 2 || outsideText == true )
  {
    paint.fillRect( x1, 0, x2-x1, lineHeight(), mColor.textBg );
  }
  else
  {
    paint.fillRect( x1, 0, x2-x1, lineHeight(), mColor.secondTextBg );
  }
  if( mLayout.horzGridWidth > 0 && outsideText == false )
  {
    paint.setPen( mColor.gridFg );
    paint.drawLine( x1, mFontHeight, x2, mFontHeight );
  }

  if( mSelect.inside( fileOffset, mLayout.lineSize ) == true )
  {
    uint start = mSelect.start( fileOffset );
    uint stop  = mSelect.stop( fileOffset, mLayout.lineSize );
    drawSelection( paint, mColor.selectBg, start, stop, sx );
  }

  //
  // A marked area will be displayed "above" a selcted area (given
  // the mark background color is different)
  //
  if( mMark.inside( fileOffset, mLayout.lineSize ) == true )
  {
    uint start = mMark.start( fileOffset );
    uint stop  = mMark.stop( fileOffset, mLayout.lineSize );
    drawSelection( paint, mColor.markBg, start, stop, sx );
  }

  uint dataSize;
  unsigned char *fileData;
  if( outsideText == true )
  {
    if( size() == 0 )
    {
      return;
    }
    dataSize = 0;
    fileData = 0;
  }
  else
  {
    dataSize = documentSize() - fileOffset;
    if( dataSize > mLayout.lineSize ) { dataSize = mLayout.lineSize; }
    fileData = (unsigned char*)&(data()[ fileOffset ]);
  }

  //
  // Compute the offset area size. We postpose the actual drawing
  // until we have drawn any bookmark indicators in the editor areas.
  // because we may want to draw an indicator in the offset area as well.
  //
  int offset = mLayout.edgeMarginWidth - sx;
  if( mLayout.offsetVisible == true )
  {
    offset += mOffsetSize * mUnitWidth;
    if( mLayout.leftSeparatorWidth > 0 )
    {
      offset += mLayout.leftSeparatorWidth + mLayout.separatorMarginWidth*2;
    }
    else
    {
      offset += (mLayout.separatorMarginWidth * 3) / 2;
    }
  }


  #if 0
  int offset = mLayout.edgeMarginWidth - sx;
  if( mLayout.offsetVisible == true )
  {
    int s0 = mOffsetSize * mUnitWidth;
    int s1 = s0 + mLayout.separatorMarginWidth + mLayout.edgeMarginWidth - sx;
    if( x1 < s1 && x2 > 0 )
    {
      if( outsideText == true )
      {
	paint.fillRect( 0, 0, s1, lineHeight(), mColor.offsetBg );
      }
      else
      {
	//
	// I want to display the grid here so I cant use lineHeight()
	//
	paint.fillRect( 0, 0, s1, mFontHeight, mColor.offsetBg );
      }
    }

    if( x1 < offset + s0 && x2 >= offset && fileData != 0 )
    {
      paint.setPen( mColor.offsetFg );
      THIS_FPTR(printOffset)( mPrintBuf, fileOffset );
      // ## paint.drawText(offset,mFontAscent,&mPrintBuf[mOffsetIndex],
      // mOffsetSize);
      paint.drawText( offset, mFontAscent, 
		      QString::fromLocal8Bit(&mPrintBuf[mOffsetIndex]),
		      mOffsetSize );
    }
    offset += s0;

    if( mLayout.leftSeparatorWidth > 0 )
    {
      offset += mLayout.separatorMarginWidth;

      int s2 = mLayout.leftSeparatorWidth + mLayout.separatorMarginWidth;
      if( x1 < offset + s2 && x2 >= offset )
      {
	QPen pen( mColor.leftSeparatorFg, mLayout.leftSeparatorWidth );
	paint.setPen( pen );
	int center = offset + mLayout.leftSeparatorWidth/2;
	paint.drawLine( center, 0, center, lineHeight() );
      }
      offset += s2;
    }
    else
    {
      offset += (mLayout.separatorMarginWidth * 3) / 2;
    }
  }
  #endif


  //
  // Draw the primary area
  //
  int localOffset = offset;
  for( uint i = 0; i < dataSize; i++ )
  {
    int s = mNumCell * mUnitWidth +
      ((i+1) % mLayout.columnSize == 0) * mSplitWidth;
    if( x1 < localOffset + s && x2 >= localOffset )
    {
      int flag = THIS_FPTR(printCell)( mPrintBuf, fileData[i] );
      if( mSelect.inside( fileOffset+i ) )
      {
	paint.setPen( mColor.selectFg );
      }
      else if( mMark.inside( fileOffset+i ) )
      {
	paint.setPen( mColor.markFg );
      }
      else
      {
	paint.setPen( flag == 0 ? foregroundColor( i ) : mColor.nonPrintFg );
      }

      // ## paint.drawText( localOffset, mFontAscent, mPrintBuf, mNumCell );
      paint.drawText( localOffset, mFontAscent, 
		      QString::fromLocal8Bit(mPrintBuf), mNumCell );
    }
    localOffset += s;

    if( mLayout.vertGridWidth > 0 && i+1 < dataSize )
    {
      if( (i+1) % mLayout.columnSize == 0 )
      {
	paint.setPen( mColor.gridFg );
	int x = localOffset - (mSplitWidth+1) / 2;
	paint.drawLine( x, 0, x, mFontHeight );
      }
    }
  }

  //
  // Draw the secondary area
  //
  offset += mPrimaryWidth;
  if( mLayout.secondaryMode != SDisplayLayout::hide )
  {
    if( mLayout.rightSeparatorWidth > 0 )
    {
      offset += mLayout.separatorMarginWidth;
      int s = mLayout.separatorMarginWidth + mLayout.rightSeparatorWidth;
      if( x1 < offset + s && x2 >= offset )
      {
	QPen pen( mColor.rightSeparatorFg, mLayout.rightSeparatorWidth );
	paint.setPen( pen );
	int center = offset + mLayout.rightSeparatorWidth/2;
	paint.drawLine( center, 0, center, lineHeight() );
      }
      offset += s;
    }
    else
    {
      offset += (mLayout.separatorMarginWidth * 3) / 2;
    }

    int s = mUnitWidth;
    for( uint i = 0; i < dataSize; i++ )
    {
      if( x1 < offset + s && x2 >= offset )
      {
	int flag = printAsciiCell( mPrintBuf, fileData[i] );
	if( mSelect.inside( fileOffset+i ) )
	{
	  paint.setPen( mColor.selectFg );
	}
	else if( mMark.inside( fileOffset+i ) )
	{
	  paint.setPen( mColor.markFg );
	}
	else
	{
	  paint.setPen( flag == 0 ? mColor.secondaryFg : mColor.nonPrintFg );
	}

	// ## paint.drawText( offset, mFontAscent, mPrintBuf, 1 );
	paint.drawText( offset, mFontAscent, 
			QString::fromLocal8Bit(mPrintBuf), 1 );
      }
      offset += s;
    }
  }

  //
  // Draw the bookmark identifiers on this line (if any).  We use the
  // bitmask to minimize the number of times we try to draw the bookmarks.
  //
  int bookmarkPosition = 0;
  if( mBookmarkMap.testBit(fileOffset/200) ||
      mBookmarkMap.testBit((fileOffset+mLayout.lineSize-1)/200 ) )
  {
    // Returns a bookmark postion state
    bookmarkPosition = drawBookmarks( paint, line, sx );
  }

  //
  // Draw the offset area. We have delayed the drawing until now because
  // it is possible to draw a bookmark indicator in this area.
  //
  offset = mLayout.edgeMarginWidth - sx;
  if( mLayout.offsetVisible == true )
  {
    int s0 = mOffsetSize * mUnitWidth;
    int s1 = s0 + mLayout.separatorMarginWidth + mLayout.edgeMarginWidth - sx;
    if( x1 < s1 && x2 > 0 )
    {
      QColor bg = mShowBookmarkInOffsetColumn &&
	(bookmarkPosition & BookmarkOnLine) ?
	mColor.bookmarkBg : mColor.offsetBg;
      if( outsideText == true )
      {
	paint.fillRect( 0, 0, s1, lineHeight(), bg );
      }
      else
      {
	//
	// I want to display the grid here so I cant use lineHeight()
	//
	paint.fillRect( 0, 0, s1, mFontHeight, bg );
      }
    }

    if( x1 < offset + s0 && x2 >= offset && fileData != 0 )
    {
      paint.setPen( mShowBookmarkInOffsetColumn &&
		    bookmarkPosition & BookmarkOnLine ?
		    mColor.bookmarkFg : mColor.offsetFg );
      THIS_FPTR(printOffset)( mPrintBuf, fileOffset );
      // ## paint.drawText(offset,mFontAscent,&mPrintBuf[mOffsetIndex],
      // mOffsetSize);
      paint.drawText( offset, mFontAscent, 
		      QString::fromLocal8Bit(&mPrintBuf[mOffsetIndex]),
		      mOffsetSize );
    }

    offset += s0;

    if( mLayout.leftSeparatorWidth > 0 )
    {
      offset += mLayout.separatorMarginWidth;

      int s2 = mLayout.leftSeparatorWidth + mLayout.separatorMarginWidth;
      if( x1 < offset + s2 && x2 >= offset )
      {
	QPen pen( mColor.leftSeparatorFg, mLayout.leftSeparatorWidth );
	paint.setPen( pen );
	int center = offset + mLayout.leftSeparatorWidth/2;
	paint.drawLine( center, 0, center, lineHeight() );
      }
    }
  }


  //
  // If the cursors are located on the line we have drawn we redraw
  // them unless they have been disabled.
  //
  if( mDisableCursor == false )
  {
    if( mCursor.curr.inside( fileOffset, fileOffset + mLayout.lineSize ) )
    {
      drawCursor( paint, line, sx, bookmarkPosition & BookmarkOnCursor );
    }
  }

}




void CHexBuffer::drawText( QPainter &paint, uint line, int x1, int x2, int y,
			   bool useBlackWhite )
{
  uint fileOffset = line * mLayout.lineSize;

  bool outsideText;
  if( size() == 0 || fileOffset > documentSize() || fileOffset >= mMaximumSize)
  {
    outsideText = true;
  }
  else
  {
    outsideText = false;
  }

  if( (line+1) % 2 || outsideText == true )
  {
    paint.fillRect( x1, y, x2, lineHeight(),
		    useBlackWhite == true ? Qt::white : mColor.textBg );
  }
  else
  {
    paint.fillRect( x1, y, x2, lineHeight(),
		    useBlackWhite == true ? Qt::white : mColor.secondTextBg );
  }

  if( mLayout.horzGridWidth > 0 && outsideText == false )
  {
    QPen pen( useBlackWhite == true ? Qt::black : mColor.gridFg,
	      mLayout.horzGridWidth );
    paint.setPen( pen );
    paint.drawLine( x1, y+mFontHeight, x2+x1, y+mFontHeight );
  }

  uint dataSize;
  unsigned char *fileData;
  if( outsideText == true )
  {
    if( size() == 0 )
    {
      return;
    }
    dataSize = 0;
    fileData = 0;
  }
  else
  {
    dataSize = documentSize() - fileOffset;
    if( dataSize > mLayout.lineSize ) { dataSize = mLayout.lineSize; }
    fileData = (unsigned char*)&(data()[ fileOffset ]);
  }

  int offset = mLayout.edgeMarginWidth + x1;

  if( mLayout.offsetVisible == true )
  {
    int s1 = mOffsetSize * mUnitWidth;
    if( fileData != 0 )
    {
      paint.setPen( useBlackWhite == true ? Qt::black : mColor.offsetFg );
      THIS_FPTR(printOffset)( mPrintBuf, fileOffset );
      // ## paint.drawText( offset, mFontAscent+y, &mPrintBuf[mOffsetIndex],
      // mOffsetSize );
      paint.drawText( offset, mFontAscent+y, 
		      QString::fromLocal8Bit(&mPrintBuf[mOffsetIndex]), 
 		      mOffsetSize );
    }
    offset += s1;

    if( mLayout.leftSeparatorWidth > 0 )
    {
      offset += mLayout.separatorMarginWidth;

      int s2 = mLayout.leftSeparatorWidth + mLayout.separatorMarginWidth;
      QPen pen( useBlackWhite == true ? Qt::black : mColor.leftSeparatorFg,
		mLayout.leftSeparatorWidth );
      paint.setPen( pen );
      int center = offset + mLayout.leftSeparatorWidth/2;
      paint.drawLine( center, y, center, mFontHeight+y );
      offset += s2;
    }
    else
    {
      offset += (mLayout.separatorMarginWidth * 3) / 2;
    }
  }

  int localOffset = offset;
  for( uint i = 0; i < dataSize; i++ )
  {
    int s = mNumCell * mUnitWidth +
      ((i+1) % mLayout.columnSize == 0) * mSplitWidth;
    int flag = THIS_FPTR(printCell)( mPrintBuf, fileData[i] );
    if( useBlackWhite == true )
    {
      paint.setPen( Qt::black );
    }
    else
    {
      paint.setPen( flag == 0 ? foregroundColor( i ) : mColor.nonPrintFg );
    }
    // ## paint.drawText( localOffset, mFontAscent+y, mPrintBuf, mNumCell );
    paint.drawText( localOffset, mFontAscent+y, 
		    QString::fromLocal8Bit(mPrintBuf), mNumCell );
    localOffset += s;

    if( mLayout.vertGridWidth > 0 && i+1 < dataSize )
    {
      if( (i+1) % mLayout.columnSize == 0 )
      {
	QPen pen( useBlackWhite == true ? Qt::black : mColor.gridFg,
		  mLayout.vertGridWidth );
	paint.setPen( pen );
	int x = localOffset - (mSplitWidth+1) / 2;
	paint.drawLine( x, y, x, y+mFontHeight );
      }
    }

  }

  offset += mPrimaryWidth;

  if( mLayout.secondaryMode != SDisplayLayout::hide )
  {
    if( mLayout.rightSeparatorWidth > 0 )
    {
      offset += mLayout.separatorMarginWidth;
      int s = mLayout.separatorMarginWidth + mLayout.rightSeparatorWidth;
      QPen pen( useBlackWhite == true ? Qt::black : mColor.rightSeparatorFg,
		mLayout.rightSeparatorWidth );
      paint.setPen( pen );
      int center = offset + mLayout.rightSeparatorWidth/2;
      paint.drawLine( center, y, center, mFontHeight+y );
      offset += s;
    }
    else
    {
      offset += (mLayout.separatorMarginWidth * 3) / 2;
    }


    int s = mUnitWidth;
    for( uint i = 0; i < dataSize; i++ )
    {
      int flag = printAsciiCell( mPrintBuf, fileData[i] );
      if( useBlackWhite == true )
      {
	paint.setPen( Qt::black );
      }
      else
      {
	paint.setPen( flag == 0 ? mColor.secondaryFg : mColor.nonPrintFg );
      }
      // ## paint.drawText( offset, mFontAscent+y, mPrintBuf, 1 );
      paint.drawText( offset, mFontAscent+y, 
		      QString::fromLocal8Bit(mPrintBuf), 1 );
      offset += s;
    }
  }

}


int CHexBuffer::headerHeight( QPainter &paint )
{
  QFont font( paint.font() );
  paint.setFont( QString("helvetica") );
  const QFontMetrics &fm = paint.fontMetrics();

  int height = fm.height();
  paint.setFont( font );
  return( height );
}

int CHexBuffer::headerMargin( QPainter &paint )
{
  QFont font( paint.font() );
  paint.setFont( QString("helvetica") );
  const QFontMetrics &fm = paint.fontMetrics();

  int margin = fm.height() / 2;
  paint.setFont( font );
  return( margin );
}


void CHexBuffer::drawHeader( QPainter &paint, int sx, int width, int y,
			     bool isFooter, const SPageHeader &header,
			     const SPagePosition &position )
{
  QFont font( paint.font() );
  paint.setFont( QFont( QString("helvetica"), 12, QFont::Normal, false) );
  const QFontMetrics &fm = paint.fontMetrics();

  paint.fillRect( sx, y, width, fm.height(), Qt::white );
  paint.setPen( Qt::black );
  if( header.line == SPageHeader::SingleLine )
  {
    if( isFooter == false )
    {
      paint.drawLine( sx, y+fm.height(), sx+width, y+fm.height() );
    }
    else
    {
      paint.drawLine( sx, y, sx+width, y );
    }
  }
  else if( header.line == SPageHeader::Rectangle )
  {
    paint.drawRect( sx, y, width, fm.height() );
  }

  int pos[3] =
  {
    QPainter::AlignLeft, QPainter::AlignHCenter, QPainter::AlignRight
  };

  QString msg;
  for( int i=0; i<3; i++ )
  {
    if( header.pos[i] == SPageHeader::DateTime )
    {
      QDateTime datetime;
      datetime.setTime_t( position.now );
      msg = KGlobal::locale()->formatDateTime(datetime);
    }
    else if( header.pos[i] == SPageHeader::PageNumber )
    {
      msg = i18n("Page %1 of %2")
        .arg(KGlobal::locale()->formatNumber(position.curPage, 0))
        .arg(KGlobal::locale()->formatNumber(position.maxPage, 0));
    }
    else if( header.pos[i] == SPageHeader::FileName )
    {
      msg = mUrl;
    }
    else
    {
      continue;
    }

    if( 0 && pos[i] == QPainter::AlignRight )
    {
      //const QFontMetrics &f = QFontMetrics( QFont(QString("helvetica")) );
      //QRect r = paint.boundingRect(sx, y, width, fm.height(), pos[i], msg );
      //printf("R: %d, %d, %d, %d\n", r.x(), r.y(), r.width(), r.height() );

      int x = sx + width - /*r.width();*/ fm.width(msg);
      paint.drawText( x, y+fm.height(), msg );
      //printf("paint at %d\n", x );
    }
    else
    {
      paint.drawText( sx, y, width, fm.height(), pos[i], msg );
    }
  }

  //
  // restore original font.
  //
  paint.setFont( font );
}




int CHexBuffer::drawBookmarks( QPainter &paint, uint line, int startx )
{
  if( documentPresent() == false || mLoadingData == true )
  {
    return( 0 );
  }

  uint start = line*mLayout.lineSize;
  uint stop  = start+mLayout.lineSize;
  QColor bg  = mColor.bookmarkBg;
  QColor fg  = mColor.bookmarkFg;

  int bookmarkPosition = 0;

  for( SCursorOffset *c=mBookmarkList.first(); c!=0; c=mBookmarkList.next() )
  {
    if( c->offset >= start && c->offset < stop )
    {
      int x = c->offset - start;
      int x1 = mTextStart1 + x * mUnitWidth * mNumCell;
      x1 += (x / mLayout.columnSize) * mSplitWidth;
      int x2 = mTextStart2 + x * mUnitWidth;

      bookmarkPosition |= BookmarkOnLine;

      if( mShowBookmarkInEditor == false )
      {
	continue;
      }

      uint offset = line*mLayout.lineSize+x;
      if( offset == mCursor.curr.offset )
      {
	bookmarkPosition |= BookmarkOnCursor;
      }

      if( mSelect.inside( offset ) || mMark.inside( offset ) )
      {
	paint.fillRect( x1-startx, 2, mUnitWidth*mNumCell, mFontHeight-4, bg );
	if( mLayout.secondaryMode != SDisplayLayout::hide )
	{
	  paint.fillRect( x2-startx, 2, mUnitWidth, mFontHeight-4, bg );
	}
      }
      else
      {
	paint.fillRect( x1-startx, 1, mUnitWidth*mNumCell, mFontHeight-2, bg );
	if( mLayout.secondaryMode != SDisplayLayout::hide )
	{
	  paint.fillRect( x2-startx, 1, mUnitWidth, mFontHeight-2, bg );
	}
      }

      unsigned char c = (data()[ line*mLayout.lineSize+x]);

      int flag = THIS_FPTR(printCell)( mPrintBuf, c );
      paint.setPen( flag == 0 ? fg : mColor.nonPrintFg );
      // ## paint.drawText( x1-startx, mFontAscent, mPrintBuf, mNumCell );
      paint.drawText( x1-startx, mFontAscent, 
		      QString::fromLocal8Bit(mPrintBuf), mNumCell );
      if( mLayout.secondaryMode != SDisplayLayout::hide )
      {
	flag = printAsciiCell( mPrintBuf, c );
	paint.setPen( flag == 0 ? fg : mColor.nonPrintFg );
	// ## paint.drawText( x2-startx, mFontAscent, mPrintBuf, 1 );	
	paint.drawText( x2-startx, mFontAscent, 
			QString::fromLocal8Bit(mPrintBuf), 1 );
      }
    }
  }

  return bookmarkPosition;
}



void CHexBuffer::drawCursor( QPainter &paint, uint line, int startx,
			     bool onBookmark )
{
  if( documentPresent() == false || mLoadingData == true )
  {
    return;
  }

  SCursorSpec &c = mCursor.curr;

  //
  // Draw the cursor in primary edit area.
  //
  QColor bg, fg;
  bool useFg;
  if( mMark.inside( c.offset ) == true )
  {
    bg = mColor.markBg;
    fg = mSelect.inside( c.offset ) ? mColor.selectFg : mColor.markFg;
    useFg = true;
  }
  else if( mSelect.inside( c.offset ) == true )
  {
    bg = mColor.selectBg;
    fg = mColor.selectFg;
    useFg = true;
  }
  else
  {
    bg = (line+1) % 2 ? mColor.textBg : mColor.secondTextBg;
    fg = foregroundColor( c.offset % mLayout.lineSize );
    useFg = false; // Can be true later.
  }

  QColor cbg = mColor.cursorBg;
  QColor cfg = mColor.cursorFg;

  //
  // Fill in the general backround color
  //
  paint.fillRect( c.x1 - startx, 0, mUnitWidth, mFontHeight, bg );
  if( onBookmark == true )
  {
    int w = mUnitWidth * (mNumCell-c.cell); // Rest of cell
    if( useFg == true )
    {
      paint.fillRect( c.x1-startx, 2, w, mFontHeight-4, mColor.bookmarkBg );
    }
    else
    {
      paint.fillRect( c.x1-startx, 1, w, mFontHeight-2, mColor.bookmarkBg );
    }
  }

  //
  // Draw the cursor shape
  //
  bool transparent = false;
  if( mActiveEditor == edit_primary )
  {
    if( mShowCursor == true ) // Cursor blink on
    {
      if( c.mPrimaryShape == SCursorSpec::thin )
      {
	paint.setPen( cbg );
	int center = c.x1 - startx - 1;
	transparent = true;
	
	if( c.thickState == true )
	{
	  paint.drawLine( center, 0, center, mFontHeight - 1 );
	  paint.drawLine( center+1, 0, center+1, mFontHeight - 1 );
	}
	else
	{
	  paint.drawLine( center, 0, center, mFontHeight - 1 );
	  paint.drawLine( center-2, 0, center+2, 0 );
	  paint.drawLine( center-2, mFontHeight-1, center+2, mFontHeight-1 );
	}
      }
      else // Solid block shape
      {
	paint.fillRect( c.x1 - startx, 0, mUnitWidth, mFontHeight, cbg );
	useFg = true;
	fg = cfg;
      }
    }
  }
  else
  {
    transparent = true;
    paint.setPen( cbg );
    paint.drawRect( c.x1 - startx, 0, mUnitWidth*mNumCell, mFontHeight );
  }

  //
  // Draw the text on the cursor position and to the end of the cell.
  //
  if( c.offset < documentSize() )
  {
    int flag = THIS_FPTR(printCell)( mPrintBuf, (unsigned char)c.data );
    if( onBookmark == true )
    {
      // Inside bookmark. Draw text with bookmark foreground.
      paint.setPen( mColor.bookmarkFg );
      // ## paint.drawText( c.x1-startx, mFontAscent, &mPrintBuf[c.cell],
      // mNumCell-c.cell );
      paint.drawText( c.x1-startx, mFontAscent, 
		      QString::fromLocal8Bit(&mPrintBuf[c.cell]), 
 		      mNumCell-c.cell );
    }

    if( transparent == false || onBookmark == false )
    {
      paint.setPen( flag == 0 || useFg == true ? fg : mColor.nonPrintFg );
      // ## paint.drawText( c.x1 - startx, mFontAscent, &mPrintBuf[c.cell], 1);
      paint.drawText( c.x1 - startx, mFontAscent, 
		      QString::fromLocal8Bit(&mPrintBuf[c.cell]), 1 );
    }
  }

  //
  // Draw the cursor in secodary edit area.
  //
  if( mLayout.secondaryMode == SDisplayLayout::hide )
  {
    return;
  }


  if( mMark.inside( c.offset ) == true )
  {
    bg = mColor.markBg;
    fg = mSelect.inside( c.offset ) ? mColor.selectFg : mColor.markFg;
    useFg = true;
  }
  else if( mSelect.inside( c.offset ) == true )
  {
    bg = mColor.selectBg;
    fg = mColor.selectFg;
    useFg = true;
  }
  else
  {
    bg = (line+1) % 2 ? mColor.textBg : mColor.secondTextBg;
    fg = mColor.secondaryFg;
    useFg = false; // Can be true later.
  }



  //
  // Fill in the general backround color
  //
  if( onBookmark == true )
  {
    if( useFg == true )
    {
      paint.fillRect( c.x2-startx, 2, mUnitWidth, mFontHeight-4,
		      mColor.bookmarkBg );
    }
    else
    {
      paint.fillRect( c.x2-startx, 1, mUnitWidth, mFontHeight-2,
		      mColor.bookmarkBg );
    }
  }
  else
  {
    paint.fillRect( c.x2 - startx, 0, mUnitWidth, mFontHeight, bg );
  }

  //
  // Draw the cursor shape
  //
  transparent = false;
  if( mActiveEditor == edit_secondary )
  {
    if( mShowCursor == true ) // Cursor blink on
    {
      if( c.mSecondaryShape == SCursorSpec::thin )
      {
	paint.setPen( cbg );
	int center = c.x2 - startx - 1;
	transparent = true;

	if( c.thickState == true )
	{
	  paint.drawLine( center, 0, center, mFontHeight - 1 );
	  paint.drawLine( center+1, 0, center+1, mFontHeight - 1 );
	}
	else
	{
	  paint.drawLine( center, 0, center, mFontHeight - 1 );
	  paint.drawLine( center-2, 0, center+2, 0 );
	  paint.drawLine( center-2, mFontHeight-1, center+2, mFontHeight-1 );
	}
      }
      else
      {
	paint.fillRect( c.x2 - startx, 0, mUnitWidth, mFontHeight, cbg );
	useFg = true;
	fg = cfg;
      }
    }
  }
  else
  {
    transparent = true;
    paint.setPen( cbg );
    paint.drawRect( c.x2 - startx, 0, mUnitWidth, mFontHeight );
  }

  //
  // Draw the text on the cursor position and to the end of the cell.
  //
  if( c.offset < documentSize() )
  {
    int flag = printAsciiCell( mPrintBuf, (unsigned char)c.data );
    if( onBookmark == true )
    {
      // Inside bookmark. Draw text with bookmark foreground.
      paint.setPen( flag == 0 ? mColor.bookmarkFg : mColor.nonPrintFg );
      // ## paint.drawText( c.x2-startx, mFontAscent, mPrintBuf, 1 );
      paint.drawText( c.x2-startx, mFontAscent, 
		      QString::fromLocal8Bit(mPrintBuf), 1 );
    }
    if( transparent == false || onBookmark == false )
    {
      paint.setPen( flag == 0 || useFg == true ? fg : mColor.nonPrintFg );
      // ## paint.drawText( c.x2 - startx, mFontAscent, mPrintBuf, 1 );
      paint.drawText( c.x2 - startx, mFontAscent, 
		      QString::fromLocal8Bit(mPrintBuf), 1 );
    }
  }

}




void CHexBuffer::cursorReset( void )
{
  mCursor.reset();
  cursorCompute();
}

void CHexBuffer::cursorCompute( void )
{
  mCursor.prev = mCursor.curr;

  if( mCursor.next.offset >= documentSize() )
  {
    if( documentSize() == 0 )
    {
      mCursor.curr.offset  = 0;
      mCursor.curr.data    = 0;
      mCursor.curr.cell    = 0;
      mCursor.curr.maxCell = mNumCell;

      int x = mCursor.curr.offset % mLayout.lineSize;
      mCursor.curr.x1 = mTextStart1;
      mCursor.curr.x1 += (x * mNumCell + mCursor.curr.cell) * mUnitWidth;
      mCursor.curr.x1 += (x / mLayout.columnSize) * mSplitWidth;
      mCursor.curr.x2 = mTextStart2 + x * mUnitWidth;
      mCursor.curr.y  = (mCursor.curr.offset/mLayout.lineSize) *
	(mFontHeight+mLayout.horzGridWidth);
      return;

    }
    if( mFixedSizeMode == true )
    {
      uint max = mMaximumSize - 1;
      uint off = mCursor.curr.offset % mLayout.lineSize;
      uint end = max % mLayout.lineSize;
      if( off > end )
      {
	uint diff = off - end;
	if( max + diff > mLayout.lineSize )
	{
	  mCursor.next.offset = max + diff - mLayout.lineSize;
	}
	else
	{
	  mCursor.next.offset = 0;
	}
      }
      else
      {
	uint diff = end - off;
	mCursor.next.offset = diff > max ? max : max - diff;
      }
    }
    else
    {
      mCursor.next.offset = documentSize();
    }
  }

  mCursor.curr.offset  = mCursor.next.offset;
  mCursor.curr.data    = data()[ mCursor.curr.offset ];
  mCursor.curr.cell    = mCursor.next.cell;
  mCursor.curr.maxCell = mNumCell;

  int x = mCursor.curr.offset % mLayout.lineSize;

  mCursor.curr.x1 = mTextStart1;
  mCursor.curr.x1 += (x * mNumCell + mCursor.curr.cell) * mUnitWidth;
  mCursor.curr.x1 += (x / mLayout.columnSize) * mSplitWidth;
  mCursor.curr.x2 = mTextStart2 + x * mUnitWidth;
  mCursor.curr.y  = (mCursor.curr.offset/mLayout.lineSize) *
    (mFontHeight + mLayout.horzGridWidth);
}


bool CHexBuffer::setCursorPosition( int x, int y, bool init, bool cellLevel )
{
  if( documentPresent() == false )
  {
    return( false );
  }

  uint line  = y < 0 ? 0 : y / lineHeight();
  uint entry = 0;
  int  bit   = 7;

  if( init == false )
  {
    if( mCursor.area() == edit_primary )
    {
      int start = mTextStart1;
      if( x < start - (int)mLayout.separatorMarginWidth )
      {
	return( false );
      }
      else
      {
	int stop  = mTextStart1 + mPrimaryWidth + mLayout.separatorMarginWidth;
	int width = mNumCell * mUnitWidth;
	int space = mSplitWidth;

	for( int position = start, i=0; position < stop; i++ )
	{
	  if( x <= position + width )
	  {
	    if( cellLevel == true )
	    {
	      while( bit > 0 )
	      {
		if( x <= position + mUnitWidth )
		{
		  break;
		}
		bit -= mCursor.cellWeight();
		position += mUnitWidth;
	      }
	    }
	    break;
	  }
	  position += width + (((i+1) % mLayout.columnSize) ? 0 : space);
	  entry    += 1;
	}
      }
    }
    else
    {
      int start = mTextStart2;
      if( x < start - (int)mLayout.separatorMarginWidth ||
	  mLayout.secondaryMode == SDisplayLayout::hide )
      {
	return( false );
      }
      int stop  = mTextStart2 + mLayout.lineSize * mUnitWidth;
      int width = mUnitWidth * 1;
      int space = 0;

      for( int position = start; position < stop; )
      {
	if( x <= position + width )
	{
	  break;
	}
	position += width + space;
	entry    += 1;
      }
    }
  }
  else
  {
    int start = mTextStart1;
    int stop  = start + mPrimaryWidth + mLayout.separatorMarginWidth;
    if( x >= start - (int)mLayout.separatorMarginWidth && x <= stop )
    {
      int width = mUnitWidth * mNumCell;
      int space = mSplitWidth;

      for( int position = start, i=0; position < stop; i++ )
      {
	if( x <= position + width )
	{
	  if( cellLevel == true )
	  {
	    while( bit > 0 )
	    {
	      if( x <= position + mUnitWidth )
	      {
		break;
	      }
	      bit -= mCursor.cellWeight();
	      position += mUnitWidth;
	    }
	  }
	  break;
	}
	position += width + (((i+1) % mLayout.columnSize) ? 0 : space);
	entry    += 1;
      }

      mActiveEditor = edit_primary;
    }
    else if( mLayout.secondaryMode != SDisplayLayout::hide )
    {
      start = mTextStart2;
      stop  = mTextStart2 + mLayout.lineSize * mUnitWidth +
	mLayout.edgeMarginWidth;
      if( x >= start - (int)mLayout.separatorMarginWidth && x <= stop )
      {
	int width = mUnitWidth * 1;
	int space = 0;

	for( int position = start; position < stop; )
	{
	  if( x <= position + width )
	  {
	    break;
	  }
	  position += width + space;
	  entry    += 1;
	}

	mActiveEditor = edit_secondary;
      }
    }
    else
    {
      return( false );
    }
  }

  uint offset = line * mLayout.lineSize + entry;
  if( offset > documentSize() )
  {
    offset = documentSize();
  }

  mCursor.setOffset( offset );
  mCursor.setBit( bit < 0 ? 0 : bit );

  cursorCompute();
  if( mActiveEditor != mCursor.area() )
  {
    mCursor.setArea( mActiveEditor );
    setEditMode( mEditMode );
  }

  return( true );
}




bool CHexBuffer::inputAtCursor( QChar c )
{
  if( documentPresent() == false || mInputMode.noInput() == true )
  {
    if( mInputMode.noInput() == true ) { inputSound(); }
    return( false );
  }

  if( c.isPrint() == false )
  {
    inputSound();
    return( false );
  }

  unsigned char dest;
  bool insert;
  if( mEditMode == EditReplace || mCursor.curr.cell > 0 )
  {
    if( mCursor.curr.offset >= documentSize() )
    {
      dest   = 0;
      insert = true;
    }
    else
    {
      dest   = (unsigned char)data()[ mCursor.curr.offset ];
      insert = false;
    }
  }
  else
  {
    dest   = 0;
    insert = true;
  }

  if( insert == true && mInputMode.allowResize == false )
  {
    inputSound();
    return( false );
  }

  if( mActiveEditor == edit_primary )
  {
    // ## if( THIS_FPTR(inputCell)( &dest, c.latin1(), mCursor.curr.cell ) 
    //== false )
    if( THIS_FPTR(inputCell)( &dest, QString(c).local8Bit()[0], 
			      mCursor.curr.cell ) == false )
    {
      inputSound();
      return( false );
    }
  }
  else if( mActiveEditor == edit_secondary )
  {
    // ## if( inputAscii( &dest, c.latin1(), mCursor.curr.cell ) == false )
    if( !inputAscii( &dest, QString(c).local8Bit()[0], mCursor.curr.cell ) )
    {
      inputSound();
      return( false );
    }
  }
  else
  {
    return( false );
  }

  recordStart( mCursor );
  recordReplace( mCursor, insert == true ? 0 : 1, (char*)&dest, 1 );
  cursorRight( cursorPrimaryEdit() );
  recordEnd( mCursor );

  computeNumLines();
  return( true );
}



int CHexBuffer::inputAtCursor( const QByteArray &buf, uint oldSize )
{
  if( documentPresent() == false )
  {
    return( Err_NoActiveDocument );
  }
  if( buf.isNull() == true )
  {
    return( Err_EmptyArgument );
  }

  if( mInputMode.noInput() == true )
  {
    inputSound();
    return( Err_WriteProtect );
  }

  if( mInputMode.allowResize == false )
  {
    inputSound();
    return( Err_NoResize );
  }

  recordStart( mCursor );
  recordReplace( mCursor, oldSize, (char*)&buf[0], buf.size() );
  cursorStep( buf.size(), true, false );
  recordEnd( mCursor );

  computeNumLines();
  return( Err_Success );
}


bool CHexBuffer::removeAtCursor( bool beforeCursor )
{
  if( documentPresent() == false )
  {
    return( false );
  }

  if( mInputMode.noInput() == true || mInputMode.allowResize == false )
  {
    inputSound();
    return( false );
  }


  if( mSelect.valid() == true )
  {
    cutSelection();
    return( true );
  }


  if( beforeCursor == true )
  {
    if( mCursor.curr.offset == 0 )
    {
      return( false );
    }

    recordStart( mCursor );
    cursorLeft( false );
    recordReplace( mCursor, 1, 0, 0 );
    recordEnd( mCursor );

    computeNumLines();
    return( true );
  }
  else
  {
    if( mCursor.curr.offset + 1 > documentSize() )
    {
      return( false );
    }

    recordStart( mCursor );
    recordReplace( mCursor, 1, 0, 0 );
    recordEnd( mCursor );

    computeNumLines();
    return( true );
  }
}



int CHexBuffer::locateRange(const SExportRange &range, uint &start, uint &stop)
{
  if( range.mode == SExportRange::All )
  {
    start = 0;
    stop  = documentSize();
  }
  else if( range.mode == SExportRange::Selection )
  {
    if( mSelect.valid() == false )
    {
      return( Err_NoSelection );
    }
    start = mSelect.curr.start;
    stop  = mSelect.curr.stop;
  }
  else if( range.mode == SExportRange::Range )
  {
    start = range.start;
    stop  = range.stop;
  }
  else
  {
    return( Err_IllegalMode );
  }

  if( start >= stop )
  {
    return( Err_IllegalRange );
  }

  return( Err_Success );
}


int CHexBuffer::exportText( const SExportText &ex, CProgress &p )
{
  uint start, stop;
  int errCode = locateRange( ex.range, start, stop );
  if( errCode != Err_Success )
  {
    p.finish();
    return( errCode );
  }

  QFile file( ex.destFile );
  if( file.open( IO_WriteOnly ) == false )
  {
    p.finish();
    return( Err_OpenWriteFailed );
  }

  uint startLine = calculateLine( start );
  if( startLine >= (uint)numLines() )
  {
    startLine = numLines() == 0 ? 0 : numLines() - 1;
  }

  uint stopLine  = calculateLine( stop );
  if( stopLine >= (uint)numLines() )
  {
    stopLine = numLines() == 0 ? 0 : numLines() - 1;
  }

  uint totalSize   = stopLine - startLine + 1;
  uint remaining   = stopLine - startLine + 1;
  uint bytePerLine = mOffsetSize + 1 + (mNumCell + 2)*mLayout.lineSize + 1;
  uint linePerStep = 20;

  QByteArray array( bytePerLine * linePerStep + 1 ); // Line is 0 terminated
  if( array.isEmpty() == true )
  {
    p.finish();
    return( Err_NoMemory );
  }

  while( remaining > 0 )
  {
    uint blockSize = remaining > linePerStep ? linePerStep : remaining;
    uint printSize = 0;

    for( uint i = 0; i < blockSize; i++, startLine++ )
    {
      printSize += printLine( &array[printSize], startLine );
    }

    int writeSize = file.writeBlock( &array[0], printSize );
    if( writeSize == -1 )
    {
      p.finish();
      return( Err_WriteFailed );
    }

    remaining -= blockSize;
    if( p.expired() == true )
    {
      int errCode = p.step( (float)(totalSize-remaining)/(float)totalSize );
      if( errCode == Err_Stop && remaining > 0 )
      {
	p.finish();
	return( Err_OperationAborted );
      }
    }
  }

  p.finish();
  return( Err_Success );
}



int CHexBuffer::exportHtml( const SExportHtml &ex, CProgress &p )
{
  uint start, stop;
  int errCode = locateRange( ex.range, start, stop );
  if( errCode != Err_Success )
  {
    p.finish();
    return( errCode );
  }

  uint startLine = calculateLine( start );
  if( startLine >= (uint)numLines() )
  {
    startLine = numLines() == 0 ? 0 : numLines() - 1;
  }

  uint stopLine  = calculateLine( stop );
  if( stopLine >= (uint)numLines() )
  {
    stopLine = numLines() == 0 ? 0 : numLines() - 1;
  }

  uint totalSize = stopLine - startLine + 1;
  uint remaining = stopLine - startLine + 1;

  if( ex.linePerPage == 0 )
  {
    p.finish();
    return( Err_IllegalArgument );
  }

  uint linePerPage = ex.linePerPage;
  uint numFiles    = remaining/linePerPage + (remaining%linePerPage ? 1 : 0);
  uint fileCount   = 0;

  QStringList fileNames, offsets;
  QString name, offset;
  for( uint i=0; i < numFiles; i++ )
  {
    name.sprintf( "%08d.html", i+1 );
    fileNames.append( QString("%1/%2%3").arg(ex.package).arg(ex.prefix).
		      arg(name));
  }
  name.sprintf( "%08d.html", 0 );
  QString tocName =QString("%1/%2%3").arg(ex.package).arg(ex.prefix).arg(name);

  QString linkName;
  if( ex.symLink == true )
  {
    linkName = QString("%1/%2").arg(ex.package).arg("index.html");
  }

  while( remaining  > 0 )
  {
    THIS_FPTR(printOffset)( mPrintBuf, startLine*mLayout.lineSize );
    mPrintBuf[mOffsetSize]=0;
    offset.sprintf("[%s]", mPrintBuf );

    uint pageSize = remaining > linePerPage ? linePerPage : remaining;
    printHtmlDataPage( tocName, fileNames, fileCount, ex, startLine, pageSize);

    remaining -= pageSize;
    startLine += pageSize;
    fileCount += 1;

    THIS_FPTR(printOffset)( mPrintBuf, (startLine-1)*mLayout.lineSize );
    mPrintBuf[mOffsetSize]=0;
    offset += QString(" %1 [%2]").arg(i18n("to")).arg(mPrintBuf);
    offsets.append(offset);

    if( p.expired() == true )
    {
      int errCode = p.step( (float)(totalSize-remaining)/(float)totalSize );
      if( errCode == Err_Stop && remaining > 0 )
      {
	printHtmlTocPage( tocName, linkName, fileNames, offsets, fileCount );
	p.finish();
	return( Err_OperationAborted );
      }
    }
  }

  printHtmlTocPage( tocName, linkName, fileNames, offsets, fileCount );

  p.finish();
  return( Err_Success );
}


int CHexBuffer::exportCArray( const SExportCArray &ex, CProgress &p )
{
  uint start, stop;
  int errCode = locateRange( ex.range, start, stop );
  if( errCode != Err_Success )
  {
    p.finish();
    return( errCode );
  }

  QFile file( ex.destFile );
  if( file.open( IO_WriteOnly ) == false )
  {
    p.finish();
    return( Err_OpenWriteFailed );
  }

  uint startLine = calculateLine( start );
  if( startLine >= (uint)numLines() )
  {
    startLine = numLines() == 0 ? 0 : numLines() - 1;
  }

  uint stopLine  = calculateLine( stop );
  if( stopLine >= (uint)numLines() )
  {
    stopLine = numLines() == 0 ? 0 : numLines() - 1;
  }

  uint elementSize = ex.elementSize();
  uint elementOnThisLine = 0;

  QTextStream dest( &file );

  dest << ex.variableName(stop-start).latin1() << "={" << endl;
  for( unsigned int i=start; i<stop; i+=elementSize )
  {
    dest << ex.printFormatted( (const char*)&data()[i], stop-i );
    if( i + elementSize < stop )
    {
      dest << ",";
    }

    if( ++elementOnThisLine >= ex.elementPerLine )
    {
      dest << endl;
      elementOnThisLine = 0;
    }

    if( p.expired() == true )
    {
      int errCode = p.step( (float)(i-start)/(float)(stop-start) );
      if( errCode == Err_Stop && (i+elementSize) < stop)
      {
	p.finish();
	return( Err_OperationAborted );
      }
    }
  }
  dest << "};" << endl;

  p.finish();
  return( Err_Success );
}







int CHexBuffer::copySelectedText( QByteArray &array, int columnSegment )
{
  SExportRange range;
  range.mode = SExportRange::Selection;
  return( copyText( array, range, columnSegment ) );
}


int CHexBuffer::copyAllText( QByteArray &array )
{
  SExportRange range;
  range.mode = SExportRange::All;
  return( copyText( array, range, VisibleColumn ) );
}


int CHexBuffer::copyText( QByteArray &array, const SExportRange &range,
			  int columnSegment )
{
  uint start, stop;
  int errCode = locateRange( range, start, stop );
  if( errCode != Err_Success )
  {
    return( errCode );
  }

  uint startLine = calculateLine( start );
  uint stopLine  = calculateLine( stop );
  if( startLine >= (uint)numLines() )
  {
    startLine = numLines() == 0 ? 0 : numLines() - 1;
  }
  if( stopLine >= (uint)numLines() )
  {
    stopLine = numLines() == 0 ? 0 : numLines() - 1;
  }

  uint bytePerLine = mOffsetSize + 1 + (mNumCell + 2)*mLayout.lineSize + 1;
  uint size = (stopLine - startLine + 1)*bytePerLine;
  if( array.resize( size+1 ) == false )
  {
    return( Err_NoMemory );
  }

  if( columnSegment == VisibleColumn )
  {
    columnSegment = PrimaryColumn; // Always visible
    if( mLayout.offsetVisible == true )
    {
      columnSegment |= OffsetColumn;
    }
    if( mLayout.secondaryMode != SDisplayLayout::hide )
    {
      columnSegment |= SecondaryColumn;
    }
  }

  uint offset = 0;
  for( uint i = startLine; i <= stopLine; i++ )
  {
    offset += printLine( &array[offset], i, columnSegment );
  }
  array[size] = 0;

  return( Err_Success );
}


int CHexBuffer::copySelectedData( QByteArray &array )
{
  uint start = mSelect.start();
  uint stop  = mSelect.stop();

  if( mSelect.valid() == false || start >= stop )
  {
    return( Err_IllegalRange );
  }

  uint size = stop - start;
  if( array.resize( size ) == false )
  {
    return( Err_NoMemory );
  }

  //unsigned char *src = (unsigned char*)data();
  //char *dst = (char*)array.data();

  memcpy( &array[0], &data()[start], size );
  return( Err_Success );
}


uint CHexBuffer::numPage( CHexPrinter &printer )
{
  if( printer.asText() == true )
  {
    //
    // Assume 80 lines per page
    //

    if( printer.all() == true )
    {
      return( numLines() / 80 );
    }
    else if( printer.selection() == true )
    {
      if( mSelect.valid() == false )
      {
	return( 0 );
      }

      uint line = calculateLine( mSelect.curr.start );
      uint remaining = calculateLine( mSelect.curr.stop ) - line + 1;
      return( remaining / 80 );
    }
    else if( printer.range() == true )
    {
      uint line = calculateLine( printer.startRange() );
      uint remaining = calculateLine( printer.stopRange() ) - line + 1;
      return( remaining / 80 );
    }
    else
    {
      return( 0 );
    }
  }

  QPainter paint( &printer );
  paint.setFont( font() );

  SPageMargin margin = printer.pageMargin();
  SPageSize size = printer.pageUsableSize();
  int headHeight, footHeight, headMargin, footMargin, freeHeight;

  headHeight = footHeight = headMargin = footMargin = 0;
  if( printer.pageHeader().enable == true )
  {
    headHeight = headerHeight( paint );
    headMargin = headerMargin( paint );
  }
  if( printer.pageFooter().enable == true )
  {
    footHeight = headerHeight( paint );
    footMargin = headerMargin( paint );
  }
  freeHeight = size.height - headHeight - footHeight - headMargin - footMargin;

  float scale = 1.0;
  if( (uint)mLineWidth > size.width && printer.scaleToFit() == true )
  {
    scale = (float)size.width / (float)mLineWidth;
  }
  uint linePerPage = (uint) ((float)freeHeight/((float)lineHeight()*scale));

  uint remaining, line;
  if( printer.all() == true )
  {
    line = 0;
    remaining = numLines();
  }
  else if( printer.selection() == true )
  {
    if( mSelect.valid() == false )
    {
      return( 0 );
    }
    line = calculateLine( mSelect.curr.start );
    remaining = calculateLine( mSelect.curr.stop ) - line + 1;
  }
  else if( printer.range() == true )
  {
    line = calculateLine( printer.startRange() );
    remaining = calculateLine( printer.stopRange() ) - line + 1;
  }
  else
  {
    return( 0 );
  }

  return( remaining / linePerPage + (remaining % linePerPage ? 1 : 0) );
}



int CHexBuffer::print( CHexPrinter &printer, CProgress &p )
{
  printer.setDocName( mUrl );

  QPainter paint( &printer );
  paint.setFont( font() );

  SPageMargin margin = printer.pageMargin();
  SPageSize size = printer.pageUsableSize();
  paint.setClipRect( margin.left, margin.top, size.width, size.height );

  //printf("%d,%d,%d,%d\n", margin.left, margin.top, size.width, size.height );

  int headHeight, footHeight, headMargin, footMargin, freeHeight;

  headHeight = footHeight = headMargin = footMargin = 0;
  if( printer.pageHeader().enable == true )
  {
    headHeight = headerHeight( paint );
    headMargin = headerMargin( paint );
  }
  if( printer.pageFooter().enable == true )
  {
    footHeight = headerHeight( paint );
    footMargin = headerMargin( paint );
  }
  freeHeight = size.height - headHeight - footHeight - headMargin - footMargin;

  float scale = 1.0;
  if( (uint)mLineWidth > size.width && printer.scaleToFit() == true )
  {
    scale = (float)size.width / (float)mLineWidth;
    paint.scale( scale, scale );
  }

  uint linePerPage = (uint) ((float)freeHeight/((float)lineHeight()*scale));
  uint sx = (uint) ((float)margin.left/scale);
  uint sy = (uint) ((float)(margin.top+headHeight+headMargin)/scale);

  uint remaining = numLines();
  uint line = 0;

  if( printer.all() == true )
  {
    line = 0;
    remaining = numLines();
  }
  else if( printer.selection() == true )
  {
    if( mSelect.valid() == false )
    {
      p.finish();
      return( Err_NoSelection );
    }
    line = calculateLine( mSelect.curr.start );
    remaining = calculateLine( mSelect.curr.stop ) - line + 1;
  }
  else if( printer.range() == true )
  {
    line = calculateLine( printer.startRange() );
    remaining = calculateLine( printer.stopRange() ) - line + 1;
  }
  else
  {
    p.finish();
    return( Err_IllegalMode );
  }

  #ifdef PRINTER_TEST
  remaining = remaining > linePerPage * 10 ? linePerPage * 10 : remaining;
  #endif


  SPagePosition pageData( time(0), remaining, linePerPage );
  while( remaining > 0 )
  {
    uint lineInPage = remaining > linePerPage ? linePerPage : remaining;
    uint y = sy;

    //
    // Draw header and footer. Reset scaling during that operation.
    //
    paint.scale( 1.0/scale, 1.0/scale );
    if( printer.pageHeader().enable == true )
    {
      drawHeader( paint, margin.left, size.width, margin.top, false,
		  printer.pageHeader(), pageData );
    }
    if( printer.pageFooter().enable == true )
    {
      drawHeader( paint, margin.left, size.width,
		  margin.top+size.height-footHeight, true,
		  printer.pageFooter(), pageData );
    }
    paint.scale( scale, scale );

    //
    // Draw actual data
    //
    for( uint i=0; i < lineInPage; i++, line++ )
    {
      drawText( paint, line, sx, mLineWidth, y, printer.printBlackWhite() );
      y += lineHeight();// - mLayout.horzGridWidth; // FIXME not really nice :)

      if( p.expired() == true )
      {
	int errCode = p.step( pageData.current(), pageData.max() );
	if( errCode == Err_Stop )
	{
	  p.finish();
	  return( Err_Success ); // Success here, even if we cancel
	}
      }
    }

    if( p.expired() == true )
    {
      int errCode = p.step( pageData.current(), pageData.max() );
      if( errCode == Err_Stop )
      {
	p.finish();
	return( Err_Success );// Success here, even if we cancel
      }
    }

    remaining -= lineInPage;
    if( remaining > 0 )
    {
      printer.newPage();
    }

    pageData.step();
  }

  p.finish();
  return( Err_Success );
}





uint CHexBuffer::printLine( char *dst, uint line )
{
  uint offset = line * mLayout.lineSize;
  unsigned char *src;
  char *start = dst;

  uint dataSize;
  if( offset >= documentSize() )
  {
    src = 0;
    dataSize = 0;
  }
  else
  {
    src = (unsigned char*)&data()[ offset ];
    dataSize = documentSize() - offset;
  }

  if( mLayout.offsetVisible == true )
  {
    THIS_FPTR(printOffset)( dst, offset ); dst += mOffsetSize;
    sprintf( dst, " " ); dst += 1;
  }
  for( uint i=0; i < mLayout.lineSize; i++ )
  {
    if( i<dataSize )
    {
      THIS_FPTR(printCell)( dst, src[i] ); dst += mNumCell;
    }
    else
    {
      memset( dst, ' ', mNumCell ); dst += mNumCell;
    }
    if( mSplitWidth != 0 )
    {
      sprintf( dst, " " ); dst += 1;
    }
  }
  if( mLayout.secondaryMode != SDisplayLayout::hide )
  {
    for( uint i=0; i < mLayout.lineSize; i++ )
    {
      if( i < dataSize )
      {
	printAsciiCell( dst, src[i] ); dst += 1;
      }
      else
      {
	memset( dst, ' ', 1 ); dst += 1;
      }
    }
  }
  sprintf( dst, "\n" ); dst += 1;
  return( (uint)(dst-start) );
}


uint CHexBuffer::printLine( char *dst, uint line, int columnSegment )
{
  uint offset = line * mLayout.lineSize;
  unsigned char *src;
  char *start = dst;

  uint dataSize;
  if( offset >= documentSize() )
  {
    src = 0;
    dataSize = 0;
  }
  else
  {
    src = (unsigned char*)&data()[ offset ];
    dataSize = documentSize() - offset;
  }

  if( columnSegment & OffsetColumn )
  {
    THIS_FPTR(printOffset)( dst, offset ); dst += mOffsetSize;
    sprintf( dst, " " ); dst += 1;
  }

  if( columnSegment & PrimaryColumn )
  {
    for( uint i=0; i < mLayout.lineSize; i++ )
    {
      if( i<dataSize )
      {
	THIS_FPTR(printCell)( dst, src[i] ); dst += mNumCell;
      }
      else
      {
	memset( dst, ' ', mNumCell ); dst += mNumCell;
      }
      if( mSplitWidth != 0 )
      {
	sprintf( dst, " " ); dst += 1;
      }
    }
  }

  if( columnSegment & SecondaryColumn )
  {
    for( uint i=0; i < mLayout.lineSize; i++ )
    {
      if( i < dataSize )
      {
	printAsciiCell( dst, src[i] ); dst += 1;
      }
      else
      {
	memset( dst, ' ', 1 ); dst += 1;
      }
    }
  }

  sprintf( dst, "\n" ); dst += 1;
  return( (uint)(dst-start) );
}











bool CHexBuffer::cutSelection( void )
{
  if( documentPresent() == false || mSelect.size() == 0 )
  {
    return( false );
  }

  if( mInputMode.noInput() == true || mInputMode.allowResize == false )
  {
    inputSound();
    return( false );
  }

  recordStart( mCursor );
  cursorGoto( mSelect.start(), 0 );
  recordReplace( mCursor, mSelect.size(), 0, 0 );
  recordEnd( mCursor );

  mSelect.reset();

  computeNumLines();
  return( true );
}



bool CHexBuffer::undo( void )
{
  if( documentPresent() == false || mUndoIndex == 0 ||
      mInputMode.noInput() == true )
  {
    if( mInputMode.noInput() == true ) { inputSound(); }
    return( false );
  }

  CHexActionGroup *group = mUndoList.at( mUndoIndex-1 );
  if( group == 0 )
  {
    return( false );
  }

  mUndoIndex -= 1;
  doActionGroup( group );

  cursorGoto( group->mStartOffset, group->mStartBit );

  return( true );
}


bool CHexBuffer::redo( void )
{
  if( documentPresent() == false || mUndoIndex >= mUndoList.count() ||
      mInputMode.noInput() == true )
  {
    if( mInputMode.noInput() == true ) { inputSound(); }
    return( false );
  }

  CHexActionGroup *group = mUndoList.at( mUndoIndex );
  if( group == 0 )
  {
    return( false );
  }

  mUndoIndex += 1;
  doActionGroup( group );

  cursorGoto( group->mStopOffset, group->mStopBit );

  return( true );
}


int CHexBuffer::addBookmark( int position )
{
  if( documentPresent() == false )
  {
    return( Err_NoData );
  }

  if( mBookmarkList.count() >= 9 && position == -1 )
  {
    return( Err_ListFull );
  }

  SCursorOffset *co = new SCursorOffset;
  if( co == 0 )
  {
    fatalSound();
    return( Err_NoMemory );
  }

  co->offset = mCursor.curr.offset;
  co->bit    = mCursor.bit();

  if( position == -1 || position > (int)mBookmarkList.count() )
  {
    mBookmarkList.append( co );
  }
  else
  {
    mBookmarkList.remove( (uint)position );
    mBookmarkList.insert( (uint)position, co );
  }

  updateBookmarkMap(false);
  return( Err_Success );
}


bool CHexBuffer::removeBookmark( int position )
{
  if( position < 0 )
  {
    if( mBookmarkList.count() == 0 )
    {
      return( false );
    }
    mBookmarkList.clear();
  }
  else
  {
    if( (uint)position >= mBookmarkList.count() )
    {
      return( false );
    }
    mBookmarkList.remove( position );
  }

  updateBookmarkMap(false);
  return( true );
}


void CHexBuffer::updateBookmarkMap( bool resize )
{
  if( resize == true )
  {
    mBookmarkMap.resize( documentSize()/200 + 3 );
  }
  mBookmarkMap.fill(0);

  int bookmarkMapSize = mBookmarkMap.size();
  for( SCursorOffset *c=mBookmarkList.first(); c!=0; c=mBookmarkList.next() )
  {
    int bookmarkOffset = c->offset / 200;
    if( bookmarkOffset < bookmarkMapSize )
    {
      //
      // Espen 2000-05-16:
      // I do this test to avoid some Qt warnings when I have closed
      // or reduced the size of the documnet while the (now invalid)
      // bookmarks still exist.
      //
      mBookmarkMap.setBit(bookmarkOffset);
    }
  }
}


int CHexBuffer::findFirst( SSearchControl &sc )
{
  mMark.reset();
  int errCode = scanData( sc, true );
  return( errCode );
}

int CHexBuffer::findNext( SSearchControl &sc )
{
  sc.fromCursor = true;
  int errCode = scanData( sc, false );
  return( errCode );
}

int CHexBuffer::findWrap( SSearchControl &sc )
{
  if( sc.wrapValid == false )
  {
    return( Err_NoMatch );
  }
  sc.wrapValid = false;

  sc.fromCursor = false;
  sc.wrapActive = true;
  int errCode = scanData( sc, false );
  sc.fromCursor = true;
  return( errCode );
}




int CHexBuffer::replaceAll( SSearchControl &sc, bool init )
{
  if( init == true )
  {
    initScanData( sc );
  }

  if( sc.key.isEmpty() == true )
  {
    return( Err_EmptyArgument );
  }

  if( documentSize() == 0 )
  {
    return( Err_EmptyDocument );
  }

  uint head, tail;
  if( sc.inSelection == true )
  {
    if( mSelect.valid() == false )
    {
      return( Err_NoSelection );
    }

    head = mSelect.start();
    tail = mSelect.stop();
  }
  else
  {
    head = 0;
    tail = documentSize();
  }

  uint start, stop;
  if( sc.fromCursor == false )
  {
    if( sc.wrapActive == true )
    {
      start = sc.forward == true ? head : sc.wrapMark;
      stop  = sc.forward == true ? sc.wrapMark+sc.key.size() : tail;
    }
    else
    {
      start = head;
      stop  = tail;
    }
  }
  else if( sc.forward == true )
  {
    start = cursorOffset() < head ? head : cursorOffset();
    stop  = sc.wrapActive == true ? sc.wrapMark+sc.key.size() : tail;
  }
  else
  {
    start = sc.wrapActive == true ? sc.wrapMark : head;
    stop  = cursorOffset() > tail ? tail : cursorOffset();
  }

  if( sc.forward == true && start + sc.key.size() > stop )
  {
    //
    // When searching backwards "stop" is the last offset from where
    // we do a memcmp() upward in memory. An overflow for that
    // situation is taken care of below.
    //
    return( Err_NoMatch );
  }

  if( stop + sc.key.size() > tail )
  {
    uint diff = stop + sc.key.size() - tail;
    stop = stop > diff ? stop - diff : 0;
  }

  if( mInputMode.noInput() == true )
  {
    inputSound();
    return( Err_WriteProtect );
  }

  recordStart( mCursor );
  uint numReplaced = 0;

  if( sc.forward == true )
  {
    for( uint i = start; i <= stop ; )
    {
      if( memcmp( &data()[i], sc.key.data(), sc.key.size() ) != 0 )
      {
	i++;
      }
      else
      {
	cursorGoto( i, 7 );
	recordReplace( mCursor, sc.key.size(), sc.val.data(), sc.val.size() );
	numReplaced += 1;

	if( sc.inSelection == true )
	{
	  if( sc.key.size() > sc.val.size() )
	  {
	    mSelect.shrink( sc.key.size() - sc.val.size() );
	  }
	  else
	  {
	    mSelect.expand( sc.val.size() - sc.key.size() );
	  }
	}
	
	if( sc.key.size() > sc.val.size() )
	{
	  uint diff = sc.key.size() - sc.val.size();
	  stop -= QMIN( stop, diff );
	}
	else if( sc.key.size() < sc.val.size() )
	{
	  stop += sc.val.size() - sc.key.size();
	}

	i += sc.val.size();
	cursorStep( sc.val.size(), true, false );
      }
    }
  }
  else
  {
    for( uint i = stop; i >= start; )
    {
      if( memcmp( &data()[i], sc.key.data(), sc.key.size() ) != 0 )
      {
	if( i == 0 ) { break; }
	i--;
      }
      else
      {
	cursorGoto( i, 7 );
	recordReplace( mCursor, sc.key.size(), sc.val.data(), sc.val.size() );
	numReplaced += 1;

	if( sc.inSelection == true )
	{
	  if( sc.key.size() > sc.val.size() )
	  {
	    mSelect.shrink( sc.key.size() - sc.val.size() );
	  }
	  else
	  {
	    mSelect.expand( sc.val.size() - sc.key.size() );
	  }
	}

	i -= QMIN( i, sc.key.size() );
	if( i == 0 ) { break; }
      }
    }
  }

  recordEnd( mCursor );
  computeNumLines();

  if( numReplaced == 0 )
  {
    return( Err_NoMatch );
  }

  sc.numReplace += numReplaced;
  sc.match = true;
  mMark.reset();

  return( Err_Success );
}


int CHexBuffer::replaceMarked( SSearchControl &sc )
{
  if( documentSize() == 0 )
  {
    return( Err_EmptyDocument );
  }

  if( mMark.valid() == false )
  {
    return( Err_NoMark );
  }

  bool inSelection;
  if( mSelect.valid() == true )
  {
    if( mMark.start() >= mSelect.start() && mMark.stop() <= mSelect.stop() )
    {
      inSelection = true;
    }
    else
    {
      inSelection = false;
    }
  }
  else
  {
    inSelection = false;
  }

  if( mInputMode.noInput() == true )
  {
    inputSound();
    return( Err_WriteProtect );
  }

  recordStart( mCursor );
  cursorGoto( mMark.start(), 7 );
  recordReplace( mCursor, mMark.size(), sc.val.data(), sc.val.size() );
  sc.numReplace += 1;

  if( inSelection == true )
  {
    if( mMark.size() > sc.val.size() )
    {
      mSelect.shrink( mMark.size() - sc.val.size() );
    }
    else
    {
      sc.wrapMark += sc.val.size() - mMark.size();
      mSelect.expand( sc.val.size() - mMark.size() );
    }
  }


  if( sc.wrapActive == false )
  {
    if( sc.forward == false )
    {
      sc.wrapMark += mMark.size() > sc.val.size() ?
	mMark.size() - sc.val.size() : sc.val.size() - mMark.size();
    }
  }


  recordEnd( mCursor );
  computeNumLines();

  if( sc.forward == true )
  {
    //
    // We must step over the area we have just altered. This is
    // vital if the search key contains a pattern that exists in
    // the replace data buffer.
    //
    cursorStep( sc.val.size(), true, false );
  }
  mMark.reset();
  return( Err_Success );
}


#if 0

int CHexBuffer::replaceAll( SSearchControl &sc, bool init )
{
  if( init == true )
  {
    initScanData( sc );
  }

  if( sc.key.isEmpty() == true )
  {
    return( Err_EmptyArgument );
  }

  if( documentSize() == 0 )
  {
    return( Err_EmptyDocument );
  }

  uint head, tail;
  if( sc.inSelection == true )
  {
    if( mSelect.valid() == false )
    {
      return( Err_NoSelection );
    }

    head = mSelect.start();
    tail = mSelect.stop();
  }
  else
  {
    head = 0;
    tail = documentSize();
  }

  uint start, stop;
  if( sc.fromCursor == false )
  {
    if( sc.wrapActive == true )
    {
      start = sc.forward == true ? head : sc.wrapMark;
      stop  = sc.forward == true ? sc.wrapMark : tail;
    }
    else
    {
      start = head;
      stop  = tail;
    }
  }
  else if( sc.forward == true )
  {
    start = cursorOffset() < head ? head : cursorOffset();
    stop  = sc.wrapActive == true ? sc.wrapMark : tail;
  }
  else
  {
    start = sc.wrapActive == true ? sc.wrapMark : head;
    stop  = cursorOffset() > tail ? tail : cursorOffset();
  }


  if( start + sc.key.size() > stop )
  {
    return( Err_NoMatch );
  }

  if( stop + sc.key.size() > tail )
  {
    uint diff = stop + sc.key.size() - tail;
    stop = stop > diff ? stop - diff : 0;
  }

  if( mInputMode.noInput() == true )
  {
    inputSound();
    return( Err_WriteProtect );
  }

  recordStart( mCursor );
  uint numReplaced = 0;

  if( sc.forward == true )
  {
    for( uint i = start; i <= stop; i++ )
    {
      if( memcmp( &data()[i], sc.key.data(), sc.key.size() ) == 0 )
      {
	cursorGoto( i, 7 );
	recordReplace( mCursor, sc.key.size(), sc.val.data(), sc.val.size() );
	numReplaced += 1;

	if( sc.inSelection == true )
	{
	  if( sc.key.size() > sc.val.size() )
	  {
	    mSelect.shrink( sc.key.size() - sc.val.size() );
	  }
	  else
	  {
	    mSelect.expand( sc.val.size() - sc.key.size() );
	  }
	}

	if( sc.key.size() > sc.key.size() )
	{
	  uint diff = sc.key.size() - sc.val.size();
	  i += diff - 1;
	}
	else if( sc.key.size() < sc.val.size() )
	{
	  uint diff = sc.val.size() - sc.key.size();
	  stop += diff;
	}
	else
	{
	  i += sc.val.size() - 1;
	}
	cursorStep( sc.val.size(), true, false );
      }
    }
  }
  else
  {
    for( uint i = stop; i >= start; i-- )
    {
      if( memcmp( &data()[i], sc.key.data(), sc.key.size() ) == 0 )
      {
	cursorGoto( i, 7 );
	recordReplace( mCursor, sc.key.size(), sc.val.data(), sc.val.size() );
	numReplaced += 1;

	if( sc.inSelection == true )
	{
	  if( sc.key.size() > sc.val.size() )
	  {
	    mSelect.shrink( sc.key.size() - sc.val.size() );
	  }
	  else
	  {
	    mSelect.expand( sc.val.size() - sc.key.size() );
	  }
	}

      }
      if( i == 0 ) { break; }
    }
  }

  recordEnd( mCursor );
  computeNumLines();

  if( numReplaced == 0 )
  {
    return( Err_NoMatch );
  }

  sc.numReplace += numReplaced;
  sc.match = true;
  mMark.reset();

  return( Err_Success );
}


int CHexBuffer::replaceMarked( SSearchControl &sc )
{
  if( documentSize() == 0 )
  {
    return( Err_EmptyDocument );
  }

  if( mMark.valid() == false )
  {
    return( Err_NoMark );
  }

  bool inSelection;
  if( mSelect.valid() == false )
  {
    if( mMark.start() >= mSelect.start() && mMark.stop() <= mSelect.stop() )
    {
      inSelection = true;
    }
    else
    {
      inSelection = false;
    }
  }
  else
  {
    inSelection = false;
  }

  if( mInputMode.noInput() == true )
  {
    inputSound();
    return( Err_WriteProtect );
  }

  recordStart( mCursor );
  cursorGoto( mMark.start(), 7 );
  recordReplace( mCursor, mMark.size(), sc.val.data(), sc.val.size() );
  sc.numReplace += 1;

  if( inSelection == true )
  {
    if( mMark.size() > sc.val.size() )
    {
      mSelect.shrink( mMark.size() - sc.val.size() );
    }
    else
    {
      mSelect.expand( sc.val.size() - mMark.size() );
    }
  }

  recordEnd( mCursor );
  computeNumLines();
  mMark.reset();

  return( Err_Success );
}

#endif


int CHexBuffer::initScanData( SSearchControl &sc )
{
  sc.wrapValid  = false;
  sc.wrapActive = false;
  sc.wrapMark   = 0;
  sc.match      = false;
  sc.numReplace = 0;

  uint head, tail;
  if( sc.inSelection == true )
  {
    if( mSelect.valid() == false )
    {
      return( Err_NoSelection );
    }

    head = mSelect.start();
    tail = mSelect.stop();
  }
  else
  {
    head = 0;
    tail = documentSize();
  }

  if( sc.fromCursor == false )
  {
    sc.wrapValid  = false;
    sc.wrapActive = false;
    sc.wrapMark   = 0;
  }
  else if( sc.forward == true )
  {
    if( cursorOffset() > tail )
    {	
      sc.wrapValid  = true;
      sc.wrapActive = false;
      sc.wrapMark   = tail;
    }
    else if( cursorOffset() <= head )
    {
      sc.wrapValid  = false;
      sc.wrapActive = false;
      sc.wrapMark   = 0;
    }
    else
    {
      sc.wrapValid  = true;
      sc.wrapActive = false;
      sc.wrapMark   = cursorOffset();
    }
  }
  else
  {
    if( cursorOffset() >= tail )
    {
      sc.wrapValid  = false;
      sc.wrapActive = false;
      sc.wrapMark   = 0;
    }
    else if( cursorOffset() < head )
    {
      sc.wrapValid  = true;
      sc.wrapActive = false;
      sc.wrapMark   = head;
    }
    else
    {
      sc.wrapValid  = true;
      sc.wrapActive = false;
      sc.wrapMark   = cursorOffset();
    }
  }

  return( Err_Success );
}



int CHexBuffer::scanData( SSearchControl &sc, bool init )
{
  if( init == true )
  {
    int errCode = initScanData( sc );
    if( errCode != Err_Success )
    {
      return( errCode );
    }
  }

  if( sc.key.isEmpty() == true )
  {
    return( Err_EmptyArgument );
  }

  if( documentSize() == 0 )
  {
    return( Err_EmptyDocument );
  }

  uint head, tail;
  if( sc.inSelection == true )
  {
    if( mSelect.valid() == false )
    {
      return( Err_NoSelection );
    }

    head = mSelect.start();
    tail = mSelect.stop();
  }
  else
  {
    head = 0;
    tail = documentSize();
  }

  uint start, stop;
  if( sc.fromCursor == false )
  {
    if( sc.wrapActive == true )
    {
      start = sc.forward == true ? head : sc.wrapMark;
      stop  = sc.forward == true ? sc.wrapMark+sc.key.size() : tail;
    }
    else
    {
      start = head;
      stop  = tail;
    }
  }
  else if( sc.forward == true )
  {
    start = cursorOffset() < head ? head : cursorOffset();
    stop  = sc.wrapActive == true ? sc.wrapMark : tail;
  }
  else
  {
    start = sc.wrapActive == true ? sc.wrapMark : head;
    stop  = cursorOffset() > tail ? tail : cursorOffset();
  }

  if( sc.forward == true && start + sc.key.size() > stop )
  {
    //
    // When searching backwards "stop" is the last offset from where
    // we do a memcmp() upward in memory. An overflow for that
    // situation is taken care of below.
    //
    return( stop + sc.key.size() < tail ? Err_WrapBuffer : Err_NoData );
  }

  if( stop + sc.key.size() > tail )
  {
    uint diff = stop + sc.key.size() - tail;
    stop = stop > diff ? stop - diff : 0;
  }

  if( sc.forward == true )
  {
    for( uint i = start; i <= stop; i++ )
    {
      int result;
      if( sc.ignoreCase == true )
      {
	result = strncasecmp( &data()[i], sc.key.data(), sc.key.size() );
      }
      else
      {
	result = memcmp( &data()[i], sc.key.data(), sc.key.size() );
      }
      if( result == 0 )
      {
	if( i != cursorOffset() || mMark.size() != sc.key.size() )
	{
	  sc.match = true;
	  cursorGoto( i, 7 );
	  markSet( i, sc.key.size() );
	  return( Err_Success );
	}
      }
    }
    return( start > head ? Err_WrapBuffer : Err_NoData );
  }
  else
  {
    for( uint i = stop; i >= start; i-- )
    {
      int result;
      if( sc.ignoreCase == true )
      {
	result = strncasecmp( &data()[i], sc.key.data(), sc.key.size() );
      }
      else
      {
	result = memcmp( &data()[i], sc.key.data(), sc.key.size() );
      }
      if( result == 0 )
      {	
	if( i != cursorOffset() || mMark.size() != sc.key.size() )
	{
	  sc.match = true;
	  cursorGoto( i, 7 );
	  markSet( i, sc.key.size() );
	  return( Err_Success );
	}
      }
      if( i == 0 ) { break; }
    }

    return( stop + sc.key.size() <= tail ? Err_WrapBuffer : Err_NoData );
  }
}





int CHexBuffer::filter( SFilterControl &fc )
{
  uint head, tail;
  if( fc.inSelection == true )
  {
    if( mSelect.valid() == false )
    {
      return( Err_NoSelection );
    }

    head = mSelect.start();
    tail = mSelect.stop();
  }
  else
  {
    head = 0;
    tail = documentSize();
  }

  uint start, stop;
  if( fc.fromCursor == false )
  {
    start = head;
    stop  = tail;
  }
  else if( fc.forward == true )
  {
    start = cursorOffset() < head ? head : cursorOffset();
    stop  = tail;
  }
  else
  {
    start = head;
    stop  = cursorOffset() > tail ? tail : cursorOffset();
  }

  if( mInputMode.noInput() == true )
  {
    inputSound();
    return( Err_WriteProtect );
  }

  if( start >= stop ) { return( Err_IllegalRange ); }
  QByteArray buf( stop - start );
  if( buf.isEmpty() == true ) { return( Err_NoMemory ); }

  int errCode = fc.execute((uchar*)&buf[0],(uchar*)&data()[start],buf.size());
  if( errCode == Err_Success )
  {
    recordStart( mCursor );
    cursorGoto( start, 7 );
    recordReplace( mCursor, buf.size(), buf.data(), buf.size() );
    recordEnd( mCursor );
  }

  return( errCode );
}



int CHexBuffer::collectStrings( CStringCollectControl &sc )
{
  uint startOffset = 0;
  uint start, i;
  bool on = false;

  if( sc.minLength < 1 ) { sc.minLength = 1; }

  start = startOffset;
  for( i = startOffset; i<documentSize(); i++ )
  {
    unsigned char item = data()[i];
    if( isprint( item ) == 0 || item >= 128 )
    {
      if( on == true && i-start >= sc.minLength )
      {
	QByteArray a( i-start );
	for( uint j=0; j<(i-start); a[j]=data()[start+j], j++ );
	sc.add( start, a );
      }
      on = false;
    }
    else
    {
      if( on == false ) { start = i; }
      on = true;
    }
  }

  if( on == true && i-start >= sc.minLength )
  {
    QByteArray a( i-start );
    for( uint j=0; j<(i-start); a[j]=data()[start+j], j++ );
    sc.add( start, a );
  }

  return( Err_Success );
}


int CHexBuffer::collectStatistic( SStatisticControl &sc, CProgress &p )
{
  sc.documentSize = documentSize();
  sc.documentName = mUrl;

  for( uint i = 0; i<documentSize(); i++ )
  {
    sc.occurence[ (unsigned char)data()[i] ] += 1;

    //
    // The expired() function introduces too much overhead in this case
    // so it is only executed every 100'th character
    //
    if( i % 100 == 0 && p.expired() == true )
    {
      int errCode = p.step( (float)i/(float)documentSize() );
      if( errCode == Err_Stop && i+1 < documentSize() )
      {
	p.finish();
	return( Err_OperationAborted );
      }
    }
  }
  p.finish();
  return( Err_NoErr );
}


void CHexBuffer::doActionGroup( CHexActionGroup *group )
{
  if( group == 0 )
  {
    return;
  }

  CHexAction *action = group->mHexAction;
  group->mHexAction = 0;

  while( action != 0 )
  {
    doAction( action );
    CHexAction *next = action->mNext;
    group->insertAction( action );
    action = next;
  }

  computeNumLines();
}


void CHexBuffer::doAction( CHexAction *action )
{
  if( action->mAction == CHexAction::replace )
  {
    doReplace( action, true );
  }
}





void CHexBuffer::recordStart( SCursor &cursor )
{
  //
  // Step 1: Remove any undo element that is more recent than the
  // current undo index
  //
  while( mUndoList.count() > mUndoIndex )
  {
    mUndoList.removeLast();
  }

  //
  // Step 2: Make sure the undo list is no larger than the undo limit.
  // We remove the oldest elements in the list.
  //
  while( mUndoList.count() >= mUndoLimit )
  {
    mUndoList.removeFirst();
    mUndoIndex -= 1;
  }

  CHexActionGroup *group = new CHexActionGroup( cursor.curr.offset,
						cursor.bit() );
  if( group == 0 )
  {
    return;
  }

  mUndoList.append( group );
  mUndoIndex += 1;
}


void CHexBuffer::recordReplace( SCursor &cursor, uint size, char *data1,
				uint data1Size )
{
  CHexAction *hexAction = new CHexAction( CHexAction::replace,
					  cursor.curr.offset );
  if( hexAction == 0 )
  {
    return;
  }

  hexAction->mSize = size;
  hexAction->mData = data1;
  hexAction->mDataSize = data1Size;

  doReplace( hexAction, false );
  mUndoList.getLast()->insertAction( hexAction );

  if( mCursor.curr.offset < documentSize() )
  {
    mCursor.curr.data = data()[ mCursor.curr.offset ];
  }

}

void CHexBuffer::recordEnd( SCursor &cursor )
{
  mUndoList.getLast()->mStopOffset = cursor.curr.offset;
  mUndoList.getLast()->mStopBit = cursor.bit();
}


//
// This method is the only place where the doucument data can be changed.
//
void CHexBuffer::doReplace( CHexAction *hexAction, bool removeData )
{
  uint offset   = hexAction->mOffset;
  uint oldSize  = hexAction->mSize;
  char *newData = hexAction->mData;
  uint newSize  = hexAction->mDataSize;

  hexAction->setData( newSize, &data()[offset], oldSize );

  //
  // Input new data. Resize buffer first if necessary. We always mark the
  // data as changed (dirty) when the buffer is resized, otherwise only
  // when the new data differ from the current. Nice feature :-)
  //
  int errCode;
  if( newSize > oldSize )
  {
    errCode = moveBuffer( offset + newSize - oldSize, offset );
    mDocumentModified = true;
  }
  else if( newSize < oldSize )
  {
    errCode = moveBuffer( offset, offset + oldSize - newSize );
    mDocumentModified = true;
  }
  else
  {
    errCode = Err_Success;
    if( memcmp( &data()[offset], newData, newSize ) != 0 )
    {
      mDocumentModified = true;
    }
  }

  if( errCode == Err_Success )
  {
    memcpy( &data()[offset], newData, newSize );
  }

  //
  // Data is removed regardless of success or not. Otherwise we will
  // have a mmeory leak. The single reason why the operation above could
  // fail is because there was that no more memory that could be
  // allocated.
  //
  if( removeData == true )
  {
    delete [] newData;
  }

}


bool CHexBuffer::inputDummy( unsigned char *dest, int value, uint cell )
{
  (void)dest;
  (void)value;
  (void)cell;
  return( false );
}


bool CHexBuffer::inputHexadecimal( unsigned char *dest, int value, uint cell )
{
  if( value >= '0' && value <= '9' )
  {
    value = value - '0';
  }
  else if( value >= 'A' && value <= 'F' )
  {
    value = value - 'A' + 10;
  }
  else if( value >= 'a' && value <= 'f' )
  {
    value = value - 'a' + 10;
  }
  else
  {
    return( false );
  }

  if( cell > 1 )
  {
    return( false );
  }

  uint shift = 1 - cell;
  *dest = (*dest & ~(0xF<<(shift*4)) ) | (value<<(shift*4));
  return( true );
}


bool CHexBuffer::inputDecimal( unsigned char *dest, int value, uint cell )
{
  //
  // 2000-01-22 Espen Sand
  // I do the insertion a bit different here since decimal is special
  // with respect to bitwidths.
  //
  if( value < '0' || value > '9' || cell > 2 )
  {
    return( false );
  }

  char buf[4];
  printDecimalCell( buf, *dest );
  buf[cell]=value;
  buf[3]=0;

  int tmp = atoi(buf);
  if( tmp > 255 )
  {
    return( false );
  }

  *dest = tmp;
  return( true );
}


bool CHexBuffer::inputOctal( unsigned char *dest, int value, uint cell )
{
  if( value >= '0' && value <= '7' )
  {
    value = value - '0';
    if( cell == 0 && value > 3 )
    {
      return( false );
    }
  }
  else
  {
    return( false );
  }

  if( cell > 3 )
  {
    return( false );
  }

  uint shift = 2 - cell;
  *dest = (*dest & ~(0x7<<(shift*3)) ) | (value<<(shift*3));
  return( true );
}


bool CHexBuffer::inputBinary( unsigned char *dest, int value, uint cell )
{
  if( value >= '0' && value <= '1' )
  {
    value = value - '0';
  }
  else
  {
    return( false );
  }

  if( cell > 7 )
  {
    return( false );
  }

  uint shift = 7 - cell;
  *dest = (*dest & ~(1<<shift)) | (value<<shift);
  return( true );
}



bool CHexBuffer::inputAscii( unsigned char *dest, int value, uint )
{
  *dest = value;
  return( true );
}



int CHexBuffer::moveBuffer( uint destOffset, uint srcOffset )
{
  if( srcOffset > documentSize() || destOffset == srcOffset )
  {
    return( Err_Success );
  }

  if( destOffset < srcOffset )
  {
    char *dest = &data()[ destOffset ];
    char *src  = &data()[ srcOffset ];

    memmove( dest, src, documentSize() - srcOffset );
    setDocumentSize( documentSize() - (srcOffset - destOffset) );
    return( Err_Success );
  }
  else
  {
    uint s = documentSize() - srcOffset;
    if( destOffset + s >= size() )
    {
      int errCode = resizeBuffer( destOffset + s );
      if( errCode != Err_Success )
      {
	fatalSound();
	return( errCode );
      }
    }
    else
    {
      setDocumentSize( documentSize() + (destOffset - srcOffset) );
    }

    char *dest = &data()[ destOffset ];
    char *src  = &data()[ srcOffset ];

    memmove( dest, src, s );
    memset( src, 0, destOffset - srcOffset );
    return( Err_Success );
  }
}




int CHexBuffer::resizeBuffer( uint newSize )
{
  if( newSize < documentSize() )
  {
    return( Err_Success );
  }

  if( newSize >= size() )
  {
    QByteArray tmp;
    tmp.duplicate( data(), size() );
    if( tmp.isNull() == true )
    {
      return( Err_NoMemory );
    }

    if( fill( '\0', newSize + 100 ) == false )
    {
      return( Err_NoMemory );
    }

    memcpy( data(), &tmp[0], tmp.size() );
  }

  setDocumentSize( newSize );
  return( Err_Success );
}


void CHexBuffer::inputSound( void )
{
  if( mInputErrorSound == true )
  {
    KNotifyClient::beep( QObject::tr("Edit operation failed") );
  }
}


void CHexBuffer::fatalSound( void )
{
  if( mFatalErrorSound == true )
  {
    KNotifyClient::beep( QObject::tr("Could not allocate memory") );
  }
}


int CHexBuffer::printHtmlDataPage( const QString &tocName,
				   const QStringList &fileNames, uint index,
				   const SExportHtml &ex,
				   uint line, uint numLine )
{
  if( fileNames.count() == 0 )
  {
    return( Err_NullArgument );
  }

  if( index >= fileNames.count() )
  {
    index = fileNames.count()-1;
  }

  QFile file( fileNames[index] );
  if( file.open( IO_WriteOnly ) == false )
  {
    return( Err_OperationAborted );
  }

  QTextStream os( &file );
  const QString *next = index+1 >= fileNames.count() ? 0 : &fileNames[index+1];
  const QString *prev = index == 0 ? 0 : &fileNames[index-1];
  const QString *toc  = tocName.length() == 0 ? 0 : &tocName;

  printHtmlHeader( os, true );
  if( ex.navigator == true )
  {
    printHtmlNavigator( os, next, prev, toc );
  }

  printHtmlCaption( os, ex.topCaption, index+1, fileNames.count() );
  printHtmlTable( os, line, numLine, ex.blackWhite );
  printHtmlCaption( os, ex.bottomCaption, index+1, fileNames.count() );

  if( ex.navigator == true )
  {
    printHtmlNavigator( os, next, prev, toc );
  }
  printHtmlHeader( os, false );

  return( Err_Success );
}


void CHexBuffer::printHtmlTocPage( const QString &tocName,
				   const QString &linkName,
				   const QStringList &fileNames,
				   const QStringList &offsets,
				   uint numPage )
{
  if( numPage == 0 || fileNames.count() == 0 )
  {
    return;
  }
  if( numPage >= fileNames.count() )
  {
    numPage = fileNames.count() - 1;
  }

  QFile file( tocName );
  if( file.open( IO_WriteOnly ) == false )
  {
    return;
  }

  QTextStream os( &file );
  printHtmlHeader( os, true );

  os << "<P ALIGN=\"CENTER\">" << endl;
  os << "<B><FONT COLOR=BLACK>" << endl;
  os << mUrl << endl;
  os << "</FONT></B></CAPTION>" << endl;
  os << "</P>" << endl;

  os << "<P ALIGN=\"CENTER\"><TT>" << endl;
  for( uint i=0; i<=numPage; i++ )
  {
    QString n( fileNames[i].right( fileNames[i].length() -
				   fileNames[i].findRev('/') - 1) );
    os << "<A HREF=\"" << n << "\">" << i18n("Page") << i+1;
    os << "</A>";
    os << " " << offsets[i];
    os << "<br>" << endl;
  }
  os << "</P>" << endl;

  printHtmlHeader( os, false );

  if( linkName.isEmpty() == false )
  {
    //
    // Make a symlink. We ignore any error here. I don't consider
    // it to be fatal.
    //
    QString n( tocName.right( tocName.length() - tocName.findRev('/') - 1) );
    symlink( n.latin1(), linkName.latin1() );
  }

}



void CHexBuffer::printHtmlCaption( QTextStream &os, uint captionType,
				   uint curPage, uint numPage )
{
  QString caption;
  switch( captionType )
  {
    case 0:
      return;
    break;

    case 1:
      caption = mUrl;
    break;

    case 2:
      caption = mUrl.right( mUrl.length() - mUrl.findRev('/') - 1);
    break;

    case 3:
      caption = i18n("Page %1 of %2").arg(curPage).arg(numPage);
    break;
  }

  os << "<P ALIGN=\"CENTER\">" << endl;
  os << "<B><FONT COLOR=BLACK>" << endl;
  os << caption << endl;
  os << "</FONT></B></CAPTION>" << endl;
  os << "</P>" << endl;
}



void CHexBuffer::printHtmlNavigator( QTextStream &os, const QString *next,
				     const QString *prev, const QString *toc )
{
  os << "<TABLE BORDER=\"0\" CELLSPACING=\"0\" WIDTH=\"100%\">" << endl;
  os << "<TR>" << endl;
  os << "<TD>" << endl;
  if( next == 0 )
  {
    os << i18n("Next") << " ";
  }
  else
  {
    QString n( next->right( next->length() - next->findRev('/') - 1) );
    os << "<A HREF=\"" << n << "\">" << i18n("Next") << "</A>" << " ";
  }

  if( prev == 0 )
  {
    os << i18n("Previous") << " ";
  }
  else
  {
    QString p( prev->right( prev->length() - prev->findRev('/') - 1) );
    os << "<A HREF=\"" << p << "\">" << i18n("Previous") << "</A>" << " ";
  }

  if( toc == 0 )
  {
    os << i18n("Contents") << " ";
  }
  else
  {
    QString t( toc->right( toc->length() - toc->findRev('/') - 1) );
    os << "<A HREF=\"" << t << "\">" << i18n("Contents");
    os << "</A>" << " ";
  }

  os << "</TD>" << endl;

  os << "<TD ALIGN=\"RIGHT\">" << endl;
  os << "<A HREF=\"" << "http://home.sol.no/~espensa/khexedit" << "\">";
  os << i18n("Generated by khexedit");
  os << "</A>" << " ";

  os << "</TD>" << endl;
  os << "</TR>" << endl << "</TABLE>" << endl;
}


int CHexBuffer::printHtmlHeader( QTextStream &os, bool isFront )
{
  if( isFront == true )
  {
    os << "<HTML>" << endl << "<HEAD>" << endl;
    os << "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; ";
    os << "charset=iso-8859-1\">" << endl;
    os << "<META NAME=\"hexdata\" CONTENT=\"khexedit dump\">" << endl;
    os << "</HEAD>" << endl << "<BODY>" << endl;
  }
  else
  {
    os << "</BODY>" << endl << "</HTML>" << endl;
  }

  return( Err_Success );
}


int CHexBuffer::printHtmlTable( QTextStream &os, uint line, uint numLine,
				bool bw )
{
  uint i;
  QColor color;


  int numCol = 1;
  if( mLayout.offsetVisible == true ) { numCol += 1; }
  if( mLayout.secondaryMode != SDisplayLayout::hide ) { numCol += 1; }

  os << "<TABLE BORDER=1 COLS=" << numCol << " WIDTH=\"100%\" ";
  os << "CELLSPACING=0 CELLPADDING=2>" << endl;
  if( mLayout.offsetVisible == true )
  {
    color = bw == true ? Qt::white : mColor.offsetBg;
    os << "<TD BGCOLOR=" << color.name().latin1() << ">" << endl;
    os << "<TABLE BORDER=0 COLS=1 WIDTH=\"100%\" ";
    os << "CELLSPACING=0 CELLPADDING=2>" << endl;

    color = bw == true ? Qt::black : mColor.offsetFg;
    for( i=0; i<numLine; i++ )
    {
      os << "<TR><TD><TT><b><FONT COLOR=" << color.name().latin1() << ">";
      THIS_FPTR(printOffset)( mPrintBuf, (line+i)*mLayout.lineSize );
      mPrintBuf[mOffsetSize]=0;
      os << mPrintBuf << "</TD></TR>" << endl;
    }
    os << "</TABLE>" << endl << "</TD>" << endl;
  }

  color = bw == true ? Qt::white : mColor.textBg;
  os << "<TD BGCOLOR=" << color.name().latin1()  << ">" << endl;
  os << "<TABLE BORDER=0 COLS=1 WIDTH=\"100%\" ";
  os << "CELLSPACING=0 CELLPADDING=2>" << endl;
  for( i=0; i<numLine; i++ )
  {
    printHtmlLine( os, line+i, true, bw );
  }
  os << "</TABLE>" << endl << "</TD>" << endl;

  if( mLayout.secondaryMode != SDisplayLayout::hide )
  {
    color = bw == true ? Qt::white : mColor.textBg;
    os << "<TD BGCOLOR=" << color.name().latin1() << ">" << endl;
    os << "<TABLE BORDER=0 COLS=1 WIDTH=\"100%\" ";
    os << "CELLSPACING=0 CELLPADDING=2>" << endl;
    for( i=0; i<numLine; i++ )
    {
      printHtmlLine( os, line+i, false, bw );
    }
    os << "</TABLE>" << endl << "</TD>" << endl;
  }

  os << "</TR>" << endl << "</TABLE>" << endl;
  return( Err_Success );
}


int CHexBuffer::printHtmlLine( QTextStream &os, uint line, bool isPrimary,
			       bool bw )
{
  uint offset  = line * mLayout.lineSize;
  QColor prevColor;

  QColor color;
  if( bw == true )
  {
    color = Qt::white;
  }
  else
  {
    color = (line+1) % 2 ? mColor.textBg : mColor.secondTextBg;
  }

  os << "<TR><TD NOWRAP BGCOLOR=" << color.name().latin1() << "><TT><B>"
     << endl;
  if( offset >= documentSize() )
  {
    os << "<BR></TD></TR>" << endl;
    return( Err_Success );
  }

  for( uint i=0; i < mLayout.lineSize; i++ )
  {
    if( isPrimary == true )
    {
      if( offset+i >= documentSize() )
      {
	memset(mPrintBuf, ' ', mNumCell );	
	mPrintBuf[mNumCell] = 0;
	if( i == 0 )
	{
	  color = bw == true ? Qt::black : foregroundColor(i);
	}
	else
	{
	  color = prevColor;
	}
      }
      else
      {
	unsigned char val = (unsigned char)data()[offset+i];
	if( THIS_FPTR(printCell)( mPrintBuf, val ) == 0 )
	{
	  color = bw == true ? Qt::black : foregroundColor(i);
	}
	else
	{
	  color = bw == true ? Qt::black : mColor.nonPrintFg;
	}
      }
      mPrintBuf[mNumCell] = 0;
      if( i == 0 )
      {
	os << "<FONT COLOR=" << color.name().latin1() << ">";
      }
      else if( color != prevColor )
      {
	os << "</FONT><FONT COLOR=" << color.name().latin1() << ">";
      }
      prevColor = color;

      if( mPrintBuf[0] == '<' )
      {
	os << "&lt;";
      }
      else
      {
	os << mPrintBuf;
	if( (i+1) % mLayout.columnSize == 0 && (i+1) != mLayout.lineSize )
	{
	  os << " ";
	}
      }
    }
    else
    {
      if( offset+i >= documentSize() )
      {
	memset(mPrintBuf, ' ', 1 );
	if( i == 0 )
	{
	  color = bw == true ? Qt::black : mColor.secondaryFg;
	}
	else
	{
	  color = prevColor;
	}
      }
      else
      {
	unsigned char val = (unsigned char)data()[offset+i];
	if( printAsciiCell( mPrintBuf, val ) == 0 )
	{
	  color = bw == true ? Qt::black : mColor.secondaryFg;
	}
	else
	{
	  color = bw == true ? Qt::black : mColor.nonPrintFg;
	}
	mPrintBuf[1] = 0;

	if( i == 0 )
	{
	  os << "<FONT COLOR=" << color.name().latin1() << ">";
	}
	else if( color != prevColor )
	{
	  os << "</FONT><FONT COLOR=" << color.name().latin1()  << ">";
	}
	prevColor = color;
	
	mPrintBuf[1] = 0;
	os << (mPrintBuf[0] == '<' ? "&lt;" : mPrintBuf);
      }

    }
  }
  os << "</TD></TR>" << endl;
  return( Err_Success );
}





