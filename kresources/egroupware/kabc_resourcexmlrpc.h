/*
    This file is part of libkabc.
    Copyright (c) 2003 - 2004 Tobias Koenig <tokoe@kde.org>

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

#ifndef KABC_RESOURCEXMLRPC_H
#define KABC_RESOURCEXMLRPC_H

#include <qmap.h>

#include <kabc/resource.h>

class KConfig;

namespace KXMLRPC {
class Server;
}

namespace KABC {

class ResourceXMLRPC : public Resource
{
  Q_OBJECT

  public:
    ResourceXMLRPC( const KConfig* );
    ResourceXMLRPC( const KURL &url, const QString &domain,
                    const QString &user, const QString &password );
    ~ResourceXMLRPC();

    virtual void writeConfig( KConfig* );

    virtual bool doOpen();
    virtual void doClose();

    virtual Ticket *requestSaveTicket();
    virtual void releaseSaveTicket( Ticket* );

    virtual bool load();
    virtual bool asyncLoad();
    virtual bool save( Ticket * );
    virtual bool asyncSave( Ticket * );

    virtual void insertAddressee( const Addressee &addr );
    virtual void removeAddressee( const Addressee& addr );

    void setURL( const KURL &url );
    KURL url() const;

    void setDomain( const QString &domain );
    QString domain() const;

    void setUser( const QString &user );
    QString user() const;

    void setPassword( const QString &password );
    QString password() const;

  protected:
    void init( const KURL &url, const QString &domain,
               const QString &user, const QString &password );

  protected slots:
    void loginFinished( const QValueList<QVariant>&, const QVariant& );
    void logoutFinished( const QValueList<QVariant>&, const QVariant& );

    void listEntriesFinished( const QValueList<QVariant>&, const QVariant& );
    void addEntryFinished( const QValueList<QVariant>&, const QVariant& );
    void updateEntryFinished( const QValueList<QVariant>&, const QVariant& );
    void deleteEntryFinished( const QValueList<QVariant>&, const QVariant& );

    void fault( int, const QString&, const QVariant& );

  private:
    QString addrTypesToTypeStr( int );

    void enter_loop();
    void exit_loop();
    void fillArgs( const Addressee&, QMap<QString, QVariant>& );

    KURL mURL;
    QString mDomain;
    QString mUser;
    QString mPassword;

    QString mSessionID;
    QString mKp3;
    QString mLastAddUid;

    QMap<QString, QString> mUidMap;
    QMap<QString, int> mAddrTypes;

    KXMLRPC::Server *mServer;
    bool mSyncComm;

    class ResourceXMLRPCPrivate;
    ResourceXMLRPCPrivate *d;
};

}

#endif
