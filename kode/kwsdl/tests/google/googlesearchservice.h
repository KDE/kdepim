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
#ifndef GOOGLESEARCHSERVICE_H
#define GOOGLESEARCHSERVICE_H

#include <tqobject.h>
#include <tqstring.h>
#include <transport.h>
#include <tqcstring.h>
#include <googlesearchresult.h>

class GoogleSearchService : public QObject
{
  Q_OBJECT

  public:
    void doGetCachedPage( TQString* key, TQString* url );
    void doSpellingSuggestion( TQString* key, TQString* phrase );
    void doGoogleSearch( TQString* key, TQString* q, int* start, int* maxResults, bool* filter, TQString* restrict, bool* safeSearch, TQString* lr, TQString* ie, TQString* oe );
    GoogleSearchService();
    ~GoogleSearchService();
  
  signals:
    void doGetCachedPageResponse( TQByteArray* value );
    void doSpellingSuggestionResponse( TQString* value );
    void doGoogleSearchResponse( GoogleSearchResult* value );
  
  private slots:
    void doGetCachedPageResponseSlot( const TQString &xml );
    void doSpellingSuggestionResponseSlot( const TQString &xml );
    void doGoogleSearchResponseSlot( const TQString &xml );
  
  private:
    Transport*mDoGetCachedPageTransport;
    Transport*mDoSpellingSuggestionTransport;
    Transport*mDoGoogleSearchTransport;
};

#endif
