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


#include "hexprinter.h"


CHexPrinter::CHexPrinter( void )
  : KPrinter()
{
}


void CHexPrinter::setAsText( bool state )
{
  mAsText = state;
}


void CHexPrinter::setAll( bool state )
{
  mAll = state;
}


void CHexPrinter::setSelection( bool state )
{
  mInSelection = state;
}


void CHexPrinter::setRange( bool state, uint start, uint stop )
{
  mInRange = state;
  if( mInRange == false ) 
  { 
    start = stop = 0; 
  }
  else
  {
    if( stop < start ) { stop = start; }
    
    mStartOffset = start;
    mStopOffset  = stop;
  }
}


void CHexPrinter::setScaleToFit( bool state )
{
  mScaleToFit = state;
}


void CHexPrinter::setPrintBackWhite( bool state )
{
  mPrintBlackWhite = state;
}


void CHexPrinter::setPageMarginMM( uint top, uint bottom, int right, int left )
{
  mPageMargin.top = top;
  mPageMargin.left = left;
  mPageMargin.bottom = bottom;
  mPageMargin.right = right;
}

void CHexPrinter::setTopMarginMM( uint value )
{
  mPageMargin.top = value;
}

void CHexPrinter::setLeftMarginMM( uint value )
{
  mPageMargin.left = value;
}

void CHexPrinter::setBottomMarginMM( uint value )
{
  mPageMargin.bottom = value;
}

void CHexPrinter::setRightMarginMM( uint value )
{
  mPageMargin.right = value;
}



void CHexPrinter::setPageHeader( bool enable, uint left, uint center, 
				 uint right, uint line )
{
  if( left > SPageHeader::FileName ) { left = SPageHeader::NoString; }
  if( center > SPageHeader::FileName ) { center = SPageHeader::NoString; }
  if( right > SPageHeader::FileName ) { right = SPageHeader::NoString; }
  if( line > SPageHeader::Rectangle ) { line = SPageHeader::NoLine; }

  mHeader.enable = enable;
  mHeader.pos[0] = (SPageHeader::EHeaderString)left; 
  mHeader.pos[1] = (SPageHeader::EHeaderString)center;
  mHeader.pos[2] = (SPageHeader::EHeaderString)right;
  mHeader.line = (SPageHeader::EHeaderLine)line;

  if( mHeader.pos[0] == SPageHeader::NoString && 
      mHeader.pos[1] == SPageHeader::NoString &&
      mHeader.pos[2] == SPageHeader::NoString &&
      mHeader.line == SPageHeader::NoLine )
  {
    mHeader.enable = false;
  }
}

void CHexPrinter::setPageFooter( bool enable, uint left, uint center, 
				 uint right, uint line )
{
  if( left > SPageHeader::FileName ) { left = SPageHeader::NoString; }
  if( center > SPageHeader::FileName ) { center = SPageHeader::NoString; }
  if( right > SPageHeader::FileName ) { right = SPageHeader::NoString; }
  if( line > SPageHeader::Rectangle ) { line = SPageHeader::NoLine; }

  mFooter.enable = enable;
  mFooter.pos[0] = (SPageHeader::EHeaderString)left; 
  mFooter.pos[1] = (SPageHeader::EHeaderString)center;
  mFooter.pos[2] = (SPageHeader::EHeaderString)right;
  mFooter.line = (SPageHeader::EHeaderLine)line;

  if( mFooter.pos[0] == SPageHeader::NoString && 
      mFooter.pos[1] == SPageHeader::NoString &&
      mFooter.pos[2] == SPageHeader::NoString &&
      mFooter.line == SPageHeader::NoLine )
  {
    mFooter.enable = false;
  }
}


SPageMargin CHexPrinter::pageMargin( void )
{
  QPaintDeviceMetrics metric( this );
  float f = (float)metric.width()/(float)metric.widthMM();
    
  SPageMargin margin;
  margin.top = (uint) (f*(float)mPageMargin.top);
  margin.right = (uint) (f*(float)mPageMargin.right);
  margin.bottom = (uint) (f*(float)mPageMargin.bottom);
  margin.left = (uint) (f*(float)mPageMargin.left);

  return( margin );
}


SPageMargin CHexPrinter::pageMarginMM( void )
{
  return( mPageMargin );
}


SPageSize CHexPrinter::pageFullSize( void )
{
  QPaintDeviceMetrics metric( this );
  SPageSize size;
  size.width = metric.width();
  size.width = metric.height();
  
  return( size );
}


SPageSize CHexPrinter::pageUsableSize( void )
{
  QPaintDeviceMetrics metric( this );
  SPageMargin margin = pageMargin();
  SPageSize size;

  uint mw = margin.left + margin.right;
  if( metric.width() <= (int)mw )
  {
    size.width = 1;
  }
  else
  {
    size.width = metric.width() - mw;
  }

  uint mh = margin.top + margin.bottom;
  if( metric.height() <= (int)mh )
  {
    size.height = 1;
  }
  else
  {
    size.height = metric.height() - mh;
  }

  return( size );
}

