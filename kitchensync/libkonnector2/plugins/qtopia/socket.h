/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>

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
#ifndef QTOPIA_OPIE_SOCKET_H
#define QTOPIA_OPIE_SOCKET_H

#include <qobject.h>

#include "qtopiakonnector.h"

class KURL;

namespace KSync {
class AddressBookSyncee;
class CalendarSyncee;

class QtopiaSocket : public QObject
{
  Q_OBJECT

  public:
    QtopiaSocket( QObject *obj, const char *name );
    ~QtopiaSocket();

    QString storagePath()const;

    void setUser( const QString &user );
    void setPassword( const QString &pass );
    void setSrcIP( const QString & );
    void setDestIP( const QString & );
    void setModel( const QString &model, const QString &name );

    void startUp();
    void hangUp();

    bool startSync();
    bool isConnected();

    void write( SynceeList );
    void download( const QString &res );
    void setResources( const QStringList & );
    QString metaId() const;

  signals:
    void sync( SynceeList );

  public slots:
    void setStoragePath(const QString&);

  private slots:
    void slotError(int);
    void slotConnected();
    void slotClosed();
    void slotNOOP();
    void process();
    void slotStartSync();

  private:
    enum Type
    {
      AddressBook,
      TodoList,
      DateBook
    };


    KURL url( Type );
    KURL url( const QString &path );
    void writeCategory();
    void writeAddressbook( AddressBookSyncee * );
    void writeDatebook( CalendarSyncee * );
    void writeTodoList( CalendarSyncee * );
    void writeUnknown( KSync::UnknownSyncee * );

    void readAddressbook();
    void readDatebook();
    void readTodoList();
    void doCalendarMeta();

    CalendarSyncee* defaultCalendarSyncee();

    /* for processing the connection and authentication */
    void start(const QString & );
    void user( const QString & );
    void pass( const QString & );
    void call( const QString & );
    void flush( const QString & );
    void noop( const QString & );

    void handshake( const QString & );
    void download();
    void initSync( const QString & );

    void initFiles();
    QString partnerIdPath() const;
    void readTimeZones();

    void sendCommand( const QString& cmd );

    /* download relative from the home dir */
    bool downloadFile( const QString &str, QString &newDest );
    int m_flushedApps;

    KPIM::ProgressItem *mProgressItem;

    class Private;
    Private *d;
};

}

#endif
