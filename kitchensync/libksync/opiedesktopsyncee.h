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
#ifndef KSYNC_OPIE_DESKTOP_SYNCEE
#define KSYNC_OPIE_DESKTOP_SYNCEE

#include <qstringlist.h>
#include <qstring.h>

#include <syncee.h>

namespace KSync {

/**
  OpieDesktopSyncEntry
  Opie and Qtopia are featuring a Documents Tab
  All Documents are available with one click
  Now we've to show these files in KitchenSyncApp
  This is done through the Syncee transportation
  and syncing layer.
*/
class KDE_EXPORT OpieDesktopSyncEntry  : public SyncEntry
{
  public:
    typedef QPtrList<OpieDesktopSyncEntry> PtrList;

    OpieDesktopSyncEntry( Syncee *parent );
    OpieDesktopSyncEntry( const QStringList& category,
                          const QString& file,
                          const QString& name,
                          const QString& type,
                          const QString& size, Syncee * );
    ~OpieDesktopSyncEntry();
    OpieDesktopSyncEntry( const OpieDesktopSyncEntry& );

    QString name() ;
    QString file() const;
    QString fileType() const;
    QString size() const;
    QStringList category() const;

    QString type() const;
    QString id() ;
    QString timestamp();
    bool equals( SyncEntry* );
    SyncEntry* clone();

  private:
    class OpieDesktopSyncEntryPrivate;
    OpieDesktopSyncEntryPrivate* d;
    QStringList mCategory;
    QString mFile;
    QString mName;
    QString mType;
    QString mSize;
};

class KDE_EXPORT OpieDesktopSyncee : public Syncee
{
  public:
    OpieDesktopSyncee( Merger* m= 0);
    ~OpieDesktopSyncee();


    void addEntry( SyncEntry* entry );
    void removeEntry( SyncEntry* entry);
    SyncEntry* firstEntry();
    SyncEntry* nextEntry();

    bool writeBackup( const QString & ) { return false; }
    bool restoreBackup( const QString & ) { return false; }

  private:
    OpieDesktopSyncEntry::PtrList mList;
};

}

#endif
