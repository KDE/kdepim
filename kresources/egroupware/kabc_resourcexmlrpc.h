/*
    This file is part of kdepim.
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
#include <kdepimmacros.h>

#include "libkdepim/kabcresourcecached.h"

class KConfig;

namespace KXMLRPC {
class Server;
}

namespace KABC {

class EGroupwarePrefs;

class KDE_EXPORT ResourceXMLRPC : public ResourceCached
{
  Q_OBJECT

  public:
    ResourceXMLRPC( const KConfig* );
    ResourceXMLRPC( const QString &url, const QString &domain,
                    const QString &user, const QString &password );
    ~ResourceXMLRPC();

    virtual void writeConfig( KConfig* );

    EGroupwarePrefs *prefs() const { return mPrefs; }

    virtual bool doOpen();
    virtual void doClose();

    virtual Ticket *requestSaveTicket();
    virtual void releaseSaveTicket( Ticket* );

    virtual bool load();
    virtual bool asyncLoad();
    virtual bool save( Ticket * );
    virtual bool asyncSave( Ticket * );

  protected:
    void init();

  protected slots:
    void loginFinished( const QValueList<QVariant>&, const QVariant& );
    void logoutFinished( const QValueList<QVariant>&, const QVariant& );

    void listContactsFinished( const QValueList<QVariant>&, const QVariant& );
    void addContactFinished( const QValueList<QVariant>&, const QVariant& );
    void updateContactFinished( const QValueList<QVariant>&, const QVariant& );
    void deleteContactFinished( const QValueList<QVariant>&, const QVariant& );
    void loadCategoriesFinished( const QValueList<QVariant>&, const QVariant& );
    void loadCustomFieldsFinished( const QValueList<QVariant>&, const QVariant& );

    void fault( int, const QString&, const QVariant& );
    void addContactFault( int, const QString&, const QVariant& );
    void updateContactFault( int, const QString&, const QVariant& );
    void deleteContactFault( int, const QString&, const QVariant& );

    void addContact( const KABC::Addressee& );
    void updateContact( const KABC::Addressee& );
    void deleteContact( const KABC::Addressee& );

  private:
    void initEGroupware();

    QString addrTypesToTypeStr( int );

    void enter_loop();
    void exit_loop();
    void writeContact( const Addressee&, QMap<QString, QVariant>& );
    void readContact( const QMap<QString, QVariant>&, Addressee &addr, QString& );

    EGroupwarePrefs *mPrefs;

    QString mSessionID;
    QString mKp3;

    QMap<QString, int> mCategoryMap;
    QMap<QString, int> mAddrTypes;
    QMap<QString, QString> mCustomFieldsMap;

    KXMLRPC::Server *mServer;
    bool mSyncComm;

    class ResourceXMLRPCPrivate;
    ResourceXMLRPCPrivate *d;
};

}

#endif
