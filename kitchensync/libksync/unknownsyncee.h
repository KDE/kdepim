/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KSYNC_UNKNOWNSYNCEE_H
#define KSYNC_UNKNOWNSYNCEE_H

#include <qdatetime.h>
#include <qcstring.h>

#include <syncee.h>

namespace KSync {

/**
  UnknownSyncEntry is if a Konnector was requested
  to download a file and is unable to convert.
  Basically the UnknownSyncEntry either holds the
  fileName and the tempName or the ByteArray.
*/
class UnknownSyncEntry : public SyncEntry
{
  public:
    /**
      A Konnector can be asked to download a file on sync.
      Either it can download the file to temporary place or
      it can copy the file to a bytearray.
      DownLoadMode defines the mode
    */
    enum DownLoadMode { Tempfile = 0, ByteArray };
    typedef QPtrList<UnknownSyncEntry> PtrList;

    UnknownSyncEntry( Syncee *parent );

    /**
      c'tor
      @param array the ByteArray
      @param path the path where the file was downloaded from
      @param parent the parent Syncee
    */
    UnknownSyncEntry( const QByteArray& array, const QString& path,
                      Syncee *parent );

    /**
      c'tor
      @param fileName the place where the temp file is stored
      @param path The path where the files comes from
      @param parent is parent Syncee
    */
    UnknownSyncEntry( const QString& fileName, const QString& path,
                      Syncee *parent );

    /**
      c'tor
    */
    UnknownSyncEntry( const UnknownSyncEntry& entry );

    ~UnknownSyncEntry();

    /**
      the bytearray
    */
    QByteArray array()const;

    /**
      path to the original place
    */
    QString path()const;

    /**
      the fileName of the temp file
    */
    QString fileName()const;

    /**
      the mode of the SyncEntry
    */
    int mode()const;

    /**
      if it's possible we can keep a ::stat here
    */
    QDateTime lastAccess()const;

    /**
      set the last access
    */
    void setLastAccess(const QDateTime& time);

    QString name();
    QString id();
    QString timestamp();
    bool equals( SyncEntry* entry );
    SyncEntry* clone();

  private:
    int mMode;
    bool mHasAccess : 1;
    QByteArray mArray;
    QString mPath;
    QString mFileName;
    QDateTime mTime;

    class UnknownSyncEntryPrivate;
    UnknownSyncEntryPrivate *d;
};

class UnknownSyncee : public Syncee
{
  public:
    UnknownSyncee( Merger* m = 0);
    ~UnknownSyncee();

    UnknownSyncEntry* firstEntry();
    UnknownSyncEntry* nextEntry();
    QString type()const;
    void addEntry( SyncEntry*  );
    void removeEntry( SyncEntry* );

    bool writeBackup( const QString & );
    bool restoreBackup( const QString & );

  private:
    /** voidi returns an empty PtrList */
    UnknownSyncEntry::PtrList mList;

    class UnknownSynceePrivate;
    UnknownSynceePrivate *d;
};

}

#endif
