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
    Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
    02111-1307, USA.
*/

// $Id$

#ifndef _DATESET_H
#define _DATESET_H

#include <qdatetime.h>
#include <qpair.h>
#include <qptrlist.h>

/*
class DateRange {
  public:
    DateRange() { }
    DateRange( QDate const& from, QDate const& to )
	    : mFrom( from ), mTo( to ) { }
    bool operator< ( const DateRange& r ) { return mFrom < r.from(); }
    bool contains( QDate const& d ) { return ( mFrom <= d && d <= mTo ); }
    bool contains( QDate const& from, QDate const& to ) { return ( mFrom <= from && to <= mTo ); }

    QDate from() { return mFrom; }
    QDate to() { return mTo; }

  private:
    QDate mFrom;
    QDate mTo;
}
*/

class RangeList : public QPtrList< QPair<QDate, QDate> > {
  protected:
    virtual int compareItems(QPtrCollection::Item item1, QPtrCollection::Item item2) {
       QPair<QDate,QDate> *i1 = static_cast<QPair<QDate,QDate> *> (item1);
       QPair<QDate,QDate> *i2 = static_cast<QPair<QDate,QDate> *> (item2);
       if ( *i1 < *i2 ) return -1;
       if ( *i2 < *i1 ) return 1;
       return 0;
    }
};

class DateSet {
  public:
    DateSet();
    ~DateSet();

    void add( QDate const& date );
    void add( QDate const& from, QDate const& to );

    void remove( QDate const& date );
    void remove( QDate const& from, QDate const& to );
    
    bool contains( QDate const& date );
    // returns true if and only if the whole range is in the set
    bool contains( QDate const& from, QDate const& to );

    int find( QDate const &date );
    void print();

  protected:
  private:
    bool tryMerge( int i );
    RangeList *mDates; 

    QDate mOldestDate;
    QDate mNewestDate;
};  

#endif
