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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KABC_RESOURCEXMLRPC_H
#define KABC_RESOURCEXMLRPC_H

#include <tqmap.h>
#include <kdepimmacros.h>

#include "libkdepim/kabcresourcecached.h"

class KConfig;
class Synchronizer;

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
    ResourceXMLRPC( const TQString &url, const TQString &domain,
                    const TQString &user, const TQString &password );
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
    void loginFinished( const TQValueList<TQVariant>&, const TQVariant& );
    void logoutFinished( const TQValueList<TQVariant>&, const TQVariant& );

    void listContactsFinished( const TQValueList<TQVariant>&, const TQVariant& );
    void addContactFinished( const TQValueList<TQVariant>&, const TQVariant& );
    void updateContactFinished( const TQValueList<TQVariant>&, const TQVariant& );
    void deleteContactFinished( const TQValueList<TQVariant>&, const TQVariant& );
    void loadCategoriesFinished( const TQValueList<TQVariant>&, const TQVariant& );
    void loadCustomFieldsFinished( const TQValueList<TQVariant>&, const TQVariant& );

    void fault( int, const TQString&, const TQVariant& );
    void addContactFault( int, const TQString&, const TQVariant& );
    void updateContactFault( int, const TQString&, const TQVariant& );
    void deleteContactFault( int, const TQString&, const TQVariant& );

    void addContact( const KABC::Addressee& );
    void updateContact( const KABC::Addressee& );
    void deleteContact( const KABC::Addressee& );

  private:
    void initEGroupware();

    TQString addrTypesToTypeStr( int );

    void writeContact( const Addressee&, TQMap<TQString, TQVariant>& );
    void readContact( const TQMap<TQString, TQVariant>&, Addressee &addr, TQString& );

    EGroupwarePrefs *mPrefs;

    TQString mSessionID;
    TQString mKp3;

    TQMap<TQString, int> mCategoryMap;
    TQMap<TQString, int> mAddrTypes;
    TQMap<TQString, TQString> mCustomFieldsMap;

    KXMLRPC::Server *mServer;
    Synchronizer *mSynchronizer;

    class ResourceXMLRPCPrivate;
    ResourceXMLRPCPrivate *d;
};

}

#endif
