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

//
// This is a VERY crude implementation, which serves my requirements and 
// nothing more.
//


#include <string.h>
#include "hexclipboard.h"


static const uchar *base64EncodeTable( void )
{
  static uchar table[64] = 
  {
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
    'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
    'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
    'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/',
  };

  return( table );
}

static const uchar *base64DecodeTable( void )
{
  static uchar table[255];
  static bool init = false;
  if( init == false )
  {
    uint i;
    for( i = 0; i < 255; i++ )    { table[i] = 0x80; }
    for( i = 'A'; i <= 'Z'; i++ ) { table[i] = 0 + (i - 'A'); }
    for( i = 'a'; i <= 'z'; i++ ) { table[i] = 26 + (i - 'a'); }
    for( i = '0'; i <= '9'; i++ ) { table[i] = 52 + (i - '0'); }
    table['+'] = 62;
    table['/'] = 63;
    table['='] = 0;
    init  = true;
  } 
  return( table );
}


static const char *mimeHeader( void )
{
  static const char *data = 
    "Content-Type: application/octet-stream; " \
    "name=\"khexedit_copy\"\r\n" \
    "Content-Transfer-Encoding: base64\r\n\r\n";
  return( data );
}








CHexClipboard::CHexClipboard( void )
{
}


CHexClipboard::~CHexClipboard( void )
{
}


bool CHexClipboard::encode( QByteArray &dst, QByteArray &src )
{
  if( src.size() == 0 )
  {
    return( false );
  }

  uint srcSize  = src.size();
  uint mimeSize = strlen( mimeHeader() );
  uint lineSize = 72;
  uint dstSize  = (srcSize / 3 + (srcSize % 3 ? 1 : 0)) * 4;
  dstSize += ((dstSize/lineSize) + 1)*2 + 1;

  dst.resize( dstSize + mimeSize + 1 );
  if( dst.isNull() == true )
  {
    return( false );
  }

  unsigned char inBuf[3], outBuf[4];
  uint lineLen = 0;
  
  memcpy( dst.data(), mimeHeader(), mimeSize );
  uint j = mimeSize;


  const uchar *table = base64EncodeTable();
  for( uint i=0; i < srcSize; i += 3 )
  {
    memset( inBuf, 0, sizeof(inBuf) );
    uint n;
    for( n=0; n < 3; n++ )
    {
      if( i+n >= srcSize )
      {
	break;
      }
      inBuf[n] = src[i+n];
    }

    if( n > 0 ) 
    {
      outBuf[0] = table[ inBuf[0] >> 2 ];
      outBuf[1] = table[ ((inBuf[0] & 0x3) << 4) | (inBuf[1] >> 4) ];
      outBuf[2] = table[ ((inBuf[1] & 0xF) << 2) | (inBuf[2] >> 6) ];
      outBuf[3] = table[ inBuf[2] & 0x3F ];

      if( n < 3 )
      {
	outBuf[3] = '=';
	if( n < 2 )
	{
	  outBuf[2] = '=';
	}
      }
      
      for( uint m=0; m<4; m++ )
      {
	if( lineLen >= lineSize )
	{
	  dst[j++] = '\r';
	  dst[j++] = '\n';
	  lineLen = 0;
	}
	dst[j++] = outBuf[m];
	lineLen += 1;
      }
    }
  }
  dst[j++] = '\r';
  dst[j++] = '\n';
  dst[j++] = '\0';
  return( true );
} 




bool CHexClipboard::decode( QByteArray &dst, QString &src )
{
  uint mimeSize = strlen( mimeHeader() );
  if( src.length() <= mimeSize )
  {
    return( plainDecode( dst, src ) );
  }

  if( memcmp( src.ascii(), mimeHeader(), mimeSize ) != 0 )
  {
    return( plainDecode( dst, src ) );
  }

  uint srcSize = src.length();
  uint dstSize = (srcSize * 3) / 4;
  uchar srcBuf[4], valBuf[4], dstBuf[3];
  uint sumElement = 0;

  dst.resize( dstSize );
  if( dst.isNull() == true )
  {
    return( plainDecode( dst, src ) );
  }

  uint i,j;
  uint n = 0;

  const uchar *table = base64DecodeTable();
  for( i=mimeSize; i<srcSize; )
  {
    for( j=0; j<4; )
    {
      if( i >= srcSize )
      {
	dst.truncate( sumElement );
	return( j > 0 ? false : true );
      }
      
      int c = src[i++].latin1();
      if( c <= ' ' )
      {
	continue;
      }
      if( table[c] & 0x80 )
      {
	return( plainDecode( dst, src ) );
      }

      srcBuf[j] = c;
      valBuf[j] = table[c];
      j++;
    }
  
    dstBuf[0] = (valBuf[0] << 2) | (valBuf[1] >> 4);
    dstBuf[1] = (valBuf[1] << 4) | (valBuf[2] >> 2);
    dstBuf[2] = (valBuf[2] << 6) | (valBuf[3]);

    uint numElement = srcBuf[2] == '=' ? 1 : (srcBuf[3] == '=' ? 2 : 3);    
    for( uint m=0; m < numElement; m++ )
    {
      dst[n++] = dstBuf[m];
    }
    sumElement += numElement;

    if( numElement < 3 )
    {
      break;
    }
  }

  dst.truncate( sumElement );
  return( true );
} 


bool CHexClipboard::plainDecode( QByteArray &dst, QString &src )
{
  dst.resize( src.length() );
  if( dst.isNull() == true )
  {
    return( false );
  }

  for( uint i=0; i < src.length(); dst[i] = src[i].latin1(), i++ );
  return( true );
}

