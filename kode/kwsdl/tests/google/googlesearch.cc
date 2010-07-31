/*
    This file is part of KDE.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
    
    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "googlesearch.hh"
#include "resultelement.h"
#include "resultelementarray.h"

GoogleSearch::GoogleSearch()
  : TQObject( 0, "" )
{
  connect( &mService, TQT_SIGNAL( doGetCachedPageResponse( TQByteArray* ) ),
           this, TQT_SLOT( cachedPageResult( TQByteArray* ) ) );
  connect( &mService, TQT_SIGNAL( doSpellingSuggestionResponse( TQString* ) ),
           this, TQT_SLOT( spellingSuggestionResult( TQString* ) ) );
  connect( &mService, TQT_SIGNAL( doGoogleSearchResponse( GoogleSearchResult* ) ),
           this, TQT_SLOT( googleSearchResult( GoogleSearchResult* ) ) );

  mKey = "";
}

void GoogleSearch::cachedPage( const TQString &url )
{
  mService.doGetCachedPage( new TQString( mKey ), new TQString( url ) );
}

void GoogleSearch::spellingSuggestion( const TQString &phrase )
{
  mService.doSpellingSuggestion( new TQString( mKey ), new TQString( phrase ) );
}

void GoogleSearch::googleSearch( const TQString &query, int start, int maxResults, bool filter,
                                 const TQString &restrict, bool safeSearch, const TQString &lr, const TQString &ie,
                                 const TQString &oe )
{
  mService.doGoogleSearch( new TQString( mKey ), new TQString( query ), new int( start ), new int( maxResults ),
                           new bool( filter ), new TQString( restrict ), new bool( safeSearch ), new TQString( lr ),
                           new TQString( ie ), new TQString( oe ) );
}

void GoogleSearch::cachedPageResult( TQByteArray *array )
{
  qDebug( "--------------- Cached Page Results ---------------------" );
  qDebug( "%s", array->data() );
  qDebug( "---------------------------------------------------------" );

  delete array;
}

void GoogleSearch::spellingSuggestionResult( TQString *word )
{
  qDebug( "--------------- Spelling Suggestion ---------------------" );
  qDebug( "%s", word->latin1() );
  qDebug( "---------------------------------------------------------" );

  delete word;
}

void GoogleSearch::googleSearchResult( GoogleSearchResult *result )
{
  qDebug( "--------------------- Search Results ---------------------" );
  ResultElementArray *array = result->resultElements();
  TQPtrList<ResultElement> *list = array->items();
  TQPtrListIterator<ResultElement> it( *list );
  while ( it.current() != 0 ) {
    qDebug( "%s: %s", it.current()->summary()->latin1(), it.current()->uRL()->latin1() );
    ++it;
  }
  qDebug( "---------------------------------------------------------" );

  delete result;
}

#include "googlesearch.moc"
