/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999  Espen Sand, espensa@online.no
 *   This file is based on the work by F. Zigterman, fzr@dds.nl
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

#ifndef _PROGRESS_H_
#define _PROGRESS_H_

#include <qdatetime.h> 
#include "hexerror.h"


struct SProgressData
{
  SProgressData( float f )
  {
    useFraction = 1;
    fraction = f;
  }

  SProgressData( int cPage, int mPage )
  {
    useFraction = 0;
    curPage = cPage;
    maxPage = mPage;
  }

  int valid( void ) const
  {
    return( (useFraction == 1 && fraction >= 0) ||
	    (useFraction == 0 && curPage >= 0) ? 1 : 0 );
  }

  int   useFraction;
  float fraction;
  int   curPage;
  int   maxPage;
};



typedef int (*ProgressFunc)( void *clientData, SProgressData &pd );

class CProgress
{
  public:
    CProgress( void )
    {
      define( 0, 0 );
      mInterruptTimer.start();
    }

    CProgress( ProgressFunc progressFunc, void *clientData )
    {
      define( progressFunc, clientData );
      mInterruptTimer.start();
    }

    void define( ProgressFunc progressFunc, void *clientData )
    {
      mProgressFunc = progressFunc;
      mClientData = clientData;
    }

    void finish( void ) const
    {
      if( mProgressFunc == 0 )
      {
	return;
      }
      SProgressData pd( -1.0 );
      mProgressFunc( mClientData, pd );
    }

    bool expired( void )
    {
      return( mInterruptTimer.elapsed() > 200 );
    }

    int step( float fraction )
    {
      mInterruptTimer.start();

      if( mProgressFunc == 0 )
      {
	return( Err_Success );
      }
      SProgressData pd( fraction );
      int errCode = mProgressFunc( mClientData, pd );
      return( errCode );
    }

    int step( int curPage, int maxPage )
    {
      mInterruptTimer.start();

      if( mProgressFunc == 0 )
      {
	return( Err_Success );
      }
      SProgressData pd( curPage, maxPage );
      int errCode = mProgressFunc( mClientData, pd );
      return( errCode );
    }

  private:
    ProgressFunc mProgressFunc;
    void *mClientData;
    QTime mInterruptTimer;
};

#endif


