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

#ifndef _HEX_PRINTER_H_
#define _HEX_PRINTER_H_

#include <kprinter.h>
#include <qpaintdevicemetrics.h> 

struct SPageSize
{
  uint width;
  uint height;
};

struct SPageMargin
{
  uint top;
  uint right;
  uint bottom;
  uint left;
};


struct SPageHeader
{
  enum EHeaderString
  {
    NoString = 0,
    DateTime,
    PageNumber,
    FileName
  };

  enum EHeaderLine
  {
    NoLine = 0,
    SingleLine,
    Rectangle
  };

  bool enable;
  EHeaderString pos[3]; // left, center, right
  EHeaderLine   line;
};




class CHexPrinter : public KPrinter
{
  public:
    CHexPrinter( void );

    void setAsText( bool state );
    void setAll( bool state );
    void setSelection( bool state );
    void setRange( bool state, uint start=0, uint stop=0 );
    void setScaleToFit( bool state );
    void setPrintBackWhite( bool state );
    void setPageMarginMM( uint top, uint bottom, int right, int left );
    void setTopMarginMM( uint value );
    void setLeftMarginMM( uint value );
    void setBottomMarginMM( uint value );
    void setRightMarginMM( uint value );
    void setPageHeader( bool enable, uint left, uint center, uint right, 
			uint line );
    void setPageFooter( bool enable, uint left, uint center, uint right, 
			uint line );

    inline bool asText( void );
    inline bool all( void );
    inline bool selection( void );
    inline bool range( void );
    inline uint startRange( void );
    inline uint stopRange( void );
    inline bool outputToStdout( void );
    inline bool scaleToFit( void );
    inline bool printBlackWhite( void );
    inline SPageHeader pageHeader( void );
    inline SPageHeader pageFooter( void );
    SPageMargin pageMargin( void );
    SPageMargin pageMarginMM( void );
    SPageSize   pageFullSize( void );
    SPageSize   pageUsableSize( void );

  private:
    bool mAsText;
    bool mAll;
    bool mInSelection;
    bool mInRange;
    bool mScaleToFit;
    bool mPrintBlackWhite;
    uint mStartOffset;
    uint mStopOffset;
    SPageMargin mPageMargin;
    SPageHeader mHeader;
    SPageHeader mFooter;


};


inline bool CHexPrinter::asText( void )
{
  return( mAsText );
}

inline bool CHexPrinter::all( void )
{
  return( mAll );
}

inline bool CHexPrinter::selection( void )
{
  return( mInSelection );
}

inline bool CHexPrinter::range( void )
{
  return( mInRange );
}

inline uint CHexPrinter::startRange( void )
{
  return( fromPage() /*mStartOffset*/ );
}

inline uint CHexPrinter::stopRange( void )
{
  return( toPage() /*mStopOffset*/ );
}

inline bool CHexPrinter::scaleToFit( void )
{
  return( mScaleToFit );
}

inline bool CHexPrinter::printBlackWhite( void )
{
  return( mPrintBlackWhite );
}

inline SPageHeader CHexPrinter::pageHeader( void )
{
  return( mHeader );
}

inline SPageHeader CHexPrinter::pageFooter( void )
{
  return( mFooter );
}


#endif
