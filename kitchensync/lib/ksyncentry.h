/* This file is part of the KDE libraries
    Copyright (C) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2002 Holger Freyther <freyher@kde.org>
		  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/


#ifndef ksyncentry_h
#define ksyncentry_h

/**
  @short An entry of a dataset ,which is to be synced.
  @author Cornelius Schumacher
  @see Ksyncee, KSyncer
  
  The KSyncEntry class represents the basic unit of syncing. It provides an
  interface for identifying and comparing entries, which has to be implemented
  by concrete subclasses.
  
  KSyncEntry objects are collected by @ref KSyncee objects.
*/

#include <qstring.h>

class QDateTime;
class KSyncEntry
{
  public:
    KSyncEntry();
    virtual ~KSyncEntry();
  
    /**
     * Return a string describing the Type of the SyncEntry (Mail,iCalendar...) 
     */
    virtual QString type() = 0;
    /**
      Return a string describing this entry. This is presented to the user as
      identifier for the entry, when user interaction is required.
    */
    virtual QString name() = 0;
    virtual void setName(const QString & ) = 0;
    /**
      Returns a unique id. This is used to uniquely identify the entry. Two
      entries having the same id are considered to be two variants of the same
      entry. No two entries of the same @ref KSyncee data set must have the same
      id.
    */
    virtual QString id() = 0;
    virtual void setId(const QString & ) = 0;
    virtual QString oldId() = 0;
    virtual void setOldId(const QString &) = 0;
    /**
      Return a time stamp representing the time of the last change. This is only
      used to compare, if an entry has changed or not. It is not used to define
      an order of changes. If an entry has been copied from one KSyncee data set
      to another KSyncee data set, the timestamp has to be the same on both
      entries. If the user has changed the entry in one data set the timestamp
      has to be different.
    */
    virtual QString timestamp() = 0;
    virtual void setTimestamp(const QString &) = 0;
    /**
      Return, if the two entries are equal. Two entries are considered to be
      equal, if they contain exactly the same information, including the same id
      and timestamp.
    */
    // FIXME
    virtual bool equals(KSyncEntry *) = 0;
    virtual KSyncEntry* clone() = 0;

};

#endif




