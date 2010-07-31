/*
    This file is part of libkpimexchange.
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

// $Id$

#ifndef _DATESET_H
#define _DATESET_H

#include <tqdatetime.h>
#include <tqpair.h>
#include <tqptrlist.h>

/*
class DateRange {
  public:
    DateRange() { }
    DateRange( TQDate const& from, TQDate const& to )
	    : mFrom( from ), mTo( to ) { }
    bool operator< ( const DateRange& r ) { return mFrom < r.from(); }
    bool contains( TQDate const& d ) { return ( mFrom <= d && d <= mTo ); }
    bool contains( TQDate const& from, TQDate const& to ) { return ( mFrom <= from && to <= mTo ); }

    TQDate from() { return mFrom; }
    TQDate to() { return mTo; }

  private:
    TQDate mFrom;
    TQDate mTo;
}
*/

class RangeList : public TQPtrList< QPair<TQDate, TQDate> > {
  protected:
    virtual int compareItems(TQPtrCollection::Item item1, TQPtrCollection::Item item2) {
       QPair<TQDate,TQDate> *i1 = static_cast<QPair<TQDate,TQDate> *> (item1);
       QPair<TQDate,TQDate> *i2 = static_cast<QPair<TQDate,TQDate> *> (item2);
       if ( *i1 < *i2 ) return -1;
       if ( *i2 < *i1 ) return 1;
       return 0;
    }
};

class DateSet {
  public:
    DateSet();
    ~DateSet();

    void add( TQDate const& date );
    void add( TQDate const& from, TQDate const& to );

    void remove( TQDate const& date );
    void remove( TQDate const& from, TQDate const& to );
    
    bool contains( TQDate const& date );
    // returns true if and only if the whole range is in the set
    bool contains( TQDate const& from, TQDate const& to );

    int find( TQDate const &date );
    void print();

  protected:
  private:
    bool tryMerge( int i );
    RangeList *mDates; 

    TQDate mOldestDate;
    TQDate mNewestDate;
};  

#endif
