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

#ifndef _HEX_EDIT_STATE_H_
#define _HEX_EDIT_STATE_H_

#include <kapplication.h>
#include <qfont.h> 
#include <qpalette.h>


class SDisplayLine
{
  public:
    enum EViewMode
    {
      hexadecimal = 0,
      decimal,
      octal,
      binary,
      textOnly
    };

    SDisplayLine( void )
    {
      lineSize[hexadecimal] = 16;
      lineSize[decimal]     = 16;
      lineSize[octal]       = 16;
      lineSize[binary]      = 8;
      lineSize[textOnly]    = 64;

      columnSize[hexadecimal] = 1;
      columnSize[decimal]     = 1;
      columnSize[octal]       = 1;
      columnSize[binary]      = 1;
      columnSize[textOnly]    = 64;
    }

    uint getLineSize( int index )
    {
      return( lineSize[ index > textOnly ? 0 : index ] );
    }

    uint getColumnSize( int index )
    {
      return( columnSize[ index > textOnly ? 0 : index ] );
    }
 
    void setLineSize( int index, uint value )
    {
      if( index <= textOnly ) { lineSize[ index ] = value; }
    }

    void setColumnSize( int index, uint value )
    {
      if( index <= textOnly ) { columnSize[ index ] = value; }
    }

    uint lineSize[5];
    uint columnSize[5];
};


class SDisplayLayout
{
  public:
    enum EViewMode
    {
      hexadecimal = 0,
      decimal,
      octal,
      binary,
      textOnly,
      hide
    };

    SDisplayLayout( void )
    {
      offsetMode       = hexadecimal;
      primaryMode      = hexadecimal;
      secondaryMode    = textOnly;
      offsetUpperCase  = false;
      primaryUpperCase = false;
      offsetVisible    = true;
      lineSize         = 16;
      columnSize       = 1;
      lockLine         = true;
      lockColumn       = true;
      columnCharSpace  = true;
      columnSpacing    = 5;
      separatorMarginWidth = 5;
      edgeMarginWidth      = 5;
      leftSeparatorWidth   = 1;
      rightSeparatorWidth  = 1;
      horzGridWidth    = 0;
      vertGridWidth    = 0;
    }

    void verify( void )
    {
      if( lineSize < 1 ) { lineSize  = 1; }
      if( columnSize < 1 ) { columnSize = 1; }
      if( columnSize > lineSize ) { columnSize = lineSize; }
      if( primaryMode == textOnly ) { secondaryMode = hide; columnSpacing=0; }
      if( columnSpacing == 0 ) { columnSize = lineSize; }
      if( horzGridWidth > 1 ) { horzGridWidth = 1; }
      if( vertGridWidth > 1 ) { vertGridWidth = 1; }
    }

    bool showSecondary( void )
    {
      if( primaryMode == textOnly || secondaryMode == hide )
      {
	return( false );
      }
      else
      {
	return( true );
      }
    }
  
    QString modeStrings( uint index )
    {
      if( index == hexadecimal )
      {
	return( "hexadecimal" );
      }
      else if( index == decimal )
      {
	return( "decimal" );
      }
      else if( index == octal )
      {
	return( "octal" );
      }
      else if( index == binary )
      {
	return( "binary" );
      }
      else if( index == textOnly )
      {
	return( "textOnly" );
      }
      else 
      {
	return( "hide" );
      }
    }


    QString primaryModeString( void )
    {
      return( modeStrings( primaryMode > textOnly ? 
			   hexadecimal : primaryMode ));
    }

    QString secondaryModeString( void )
    {
      return( modeStrings( secondaryMode == textOnly ? textOnly : hide ) );
    }

    QString offsetModeString( void )
    {
      return( modeStrings( offsetMode == hexadecimal ? hexadecimal : hide ) );
    }

    QString gridModeString( void )
    {
      if( horzGridWidth == 0 && vertGridWidth == 0 )
      {
	return( "none");
      }
      else if( horzGridWidth != 0 && vertGridWidth != 0 )
      {
	return( "both");
      }
      else if( horzGridWidth != 0 )
      {
	return( "horizontal");
      }
      else
      {
	return( "vertical");
      }
    }

    void setPrimaryMode( const QString & str )
    {
      if( str.isNull() || str == "hexadecimal" )
      {
	primaryMode = hexadecimal;
      }
      else if( str == "decimal" )
      {
	primaryMode = decimal;
      }
      else if( str == "octal" )
      {
	primaryMode = octal;
      }
      else if( str == "binary" )
      {
	primaryMode = binary;
      }
      else if( str == "textOnly" )
      {
	primaryMode = textOnly;
      }
      else
      {
	primaryMode = hexadecimal;
      }
    }

    void setSecondaryMode( const QString & str )
    {
      if( str.isNull() || str == "textOnly" )
      {
	secondaryMode = textOnly;
      }
      else
      {
	secondaryMode = hide;
      }
    }

    void setOffsetMode( const QString & str )
    {
      if( str.isNull() || str == "hexadecimal" )
      {
	offsetMode = hexadecimal;
      }
      else
      {
	offsetMode = decimal;
      }
    }

    void setGridMode( const QString & str )
    {
      if( str.isNull() || str == "none" )
      {
	horzGridWidth = vertGridWidth = 0;
      }
      else if( str == "vertical" )
      {
	horzGridWidth = 0;
	vertGridWidth = 1;
      }
      else if( str == "horizontal" )
      {
	horzGridWidth = 1;
	vertGridWidth = 0;
      }
      else if( str == "both" )
      {
	horzGridWidth = vertGridWidth = 1;
      }
      else
      {
	horzGridWidth = vertGridWidth = 0;
      }
    }

    EViewMode offsetMode;
    EViewMode primaryMode;
    EViewMode secondaryMode;
    bool offsetUpperCase;
    bool primaryUpperCase;
    bool offsetVisible;
    bool lockLine;
    bool lockColumn;
    uint lineSize;
    uint columnSize;
    bool columnCharSpace;
    uint columnSpacing;
    uint separatorMarginWidth;
    uint edgeMarginWidth;
    uint leftSeparatorWidth;
    uint rightSeparatorWidth;
    uint horzGridWidth;
    uint vertGridWidth;
};




class SDisplayCursor
{
  public:
    enum EFocusMode
    {
      stopBlinking = 0,
      hide,
      ignore
    };

    SDisplayCursor( void )
    {
      focusMode   = hide;
      interval    = 500;
      alwaysVisible = false;
      alwaysBlockShape = false;
      thickInsertShape = true;
    }

    QString modeStrings( uint index )
    {
      if( index == hide )
      {
	return( "hide" );
      }
      else if( index == ignore )
      {
	return( "ignore" );
      }
      else
      {
	return( "stopBlinking" );
      }
    }

    QString focusModeString( void )
    {
      return( modeStrings( focusMode > ignore ? stopBlinking : focusMode ));
    }

    void setFocusMode( const QString & str )
    {
      if( str.isNull() || str == "hide" )
      {
	focusMode = hide;
      }
      else if( str == "stopBlinking" )
      {
	focusMode = stopBlinking;
      }
      else
      {
	focusMode = ignore;
      }
    }

    EFocusMode focusMode;
    uint interval;
    bool alwaysVisible;
    bool alwaysBlockShape;
    bool thickInsertShape;
};


class SDisplayColor
{
  public:

    SDisplayColor( void )
    {
      //
      // Default colors. The selection colors will always be the one
      // choses in Control Center.
      //
      useSystemColor   = false;
      offsetBg         = Qt::white;
      textBg           = Qt::white;
      secondTextBg     = Qt::white;
      inactiveBg       = Qt::gray;
      selectBg         = kapp->palette().active().highlight();
      selectFg         = kapp->palette().active().highlightedText();
      markBg           = Qt::blue;
      markFg           = Qt::white;
      primaryFg[0]     = Qt::black;
      primaryFg[1]     = Qt::blue;
      offsetFg         = Qt::red;
      secondaryFg      = Qt::black;
      nonPrintFg       = Qt::red;
      gridFg           = Qt::darkCyan;
      leftSeparatorFg  = Qt::darkGreen;
      rightSeparatorFg = Qt::darkGreen;
      cursorBg         = Qt::red;
      cursorFg         = Qt::black;
      bookmarkBg       = Qt::green;
      bookmarkFg       = Qt::black;
    }

    bool useSystemColor;
    QColor offsetBg;
    QColor textBg;
    QColor secondTextBg;
    QColor inactiveBg;
    QColor selectBg;
    QColor selectFg;
    QColor markBg;
    QColor markFg;
    QColor primaryFg[2];
    QColor offsetFg;
    QColor secondaryFg;
    QColor nonPrintFg;
    QColor gridFg;
    QColor leftSeparatorFg;
    QColor rightSeparatorFg;
    QColor cursorBg;
    QColor cursorFg;
    QColor bookmarkBg;
    QColor bookmarkFg;
};


class SDisplayFontInfo
{
  public:

    SDisplayFontInfo &init( void )
    {
      font.setFamily( "courier" );
      font.setPointSize( 12 );
      font.setWeight( QFont::Bold );
      font.setItalic( false );
      nonPrintChar = '.';
      return( *this );
    }

    QFont font;
    QChar nonPrintChar;
};

class SDisplayFont
{
  public:
    SDisplayFont( void )
    {
      useSystemFont = false;
      localFont.setFamily( "courier" );
      localFont.setPointSize( 12 );
      localFont.setWeight( QFont::Bold );
      localFont.setItalic( false );
      nonPrintChar = '.';
    }

    bool  useSystemFont;
    QFont localFont;
    QChar nonPrintChar;
};
  

class  SDisplayMisc
{
  public:

    enum EOpenFile
    {
      none = 0,
      mostRecent,
      allRecent
    };

    SDisplayMisc( void )
    {
      undoLevel = 100;
      openFile  = none;
      inputSound = false;
      fatalSound = false;
      autoCopyToClipboard = true;
      insertMode = false;
      writeProtect = false;
      confirmWrap = true;
      cursorJump = true;
      makeBackup = false;
      confirmThreshold = true;
      thresholdValue = 30;
      discardRecent = false;
      gotoOnStartup = false;
      gotoOnReload = true;
      bookmarkOffsetColumn = true;
      bookmarkEditor = true;
    }


    QString fileStrings( uint index )
    {
      if( index == mostRecent )
      {
	return( "mostRecent" );
      }
      else if( index == allRecent )
      {
	return( "allRecent" );
      }
      else
      {
	return( "none" );
      }
    }


    QString openFileString( void )
    {
      return( fileStrings( openFile > allRecent ? none : openFile ));
    }

    void setOpenFile( const QString &str )
    {
      if( str.isNull() == true || str == "none" )
      {
	openFile = none;
      }
      else if( str == "mostRecent" )
      {
	openFile = mostRecent;
      }
      else if( str == "allRecent" )
      {
	openFile = allRecent;
      }
      else
      {
	openFile = none;
      }
    }

    uint undoLevel;
    EOpenFile openFile;
    bool inputSound;
    bool fatalSound;
    bool autoCopyToClipboard;
    bool insertMode;
    bool writeProtect;
    bool confirmWrap;
    bool cursorJump;
    bool makeBackup;
    bool confirmThreshold;
    uint thresholdValue;
    bool discardRecent;
    bool gotoOnStartup;
    bool gotoOnReload;
    bool bookmarkOffsetColumn;
    bool bookmarkEditor;
};


class SDisplayInputMode
{
  public:
    SDisplayInputMode( void )
    {
      inputLock = false;
      readOnly = false;
      allowResize = true;
    };
  
    bool noInput( void )
    {
      return( inputLock || readOnly );
    }

    bool inputLock;  // Set by application only
    bool readOnly;   // Set by user
    bool allowResize;
};


class SDisplayState
{
  public:
    SDisplayLine      line;
    SDisplayLayout    layout;
    SDisplayCursor    cursor;
    SDisplayColor     color;
    SDisplayFont      font;
    SDisplayMisc      misc;
    SDisplayInputMode input;
};





#endif







