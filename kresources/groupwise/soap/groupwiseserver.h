/*
    This file is part of KDE.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef GROUPWISESERVER_H
#define GROUPWISESERVER_H

#include <kio/job.h>
#include <kio/jobclasses.h>
#include <qapplication.h>
#include <qobject.h>
#include <qstring.h>
#include <qthread.h>

#include <string>

#include <kabc/addressee.h>
#include <libkcal/freebusy.h>

#include "gwjobs.h"

namespace KABC {
class ResourceCached;
}

namespace KCal {
class Calendar;
class Incidence;
class ResourceCached;
}

class KExtendedSocket;

struct soap;

class ns1__Folder;
class ns1__Item;
class ns1__Appointment;
class ns1__Mail;
class ns1__Task;
class ns1__Status;

namespace GroupWise {

class AddressBook
{
  public:
    typedef QValueList<AddressBook> List;
  
    AddressBook() : isPersonal( false ), isFrequentContacts( false ) {}
  
    QString id;
    QString name;
    QString description;
    bool isPersonal;
    bool isFrequentContacts;
};

}

class GroupwiseServer : public QObject
{
  Q_OBJECT

  public:
    GroupwiseServer( const QString &url, const QString &user,
                     const QString &password, QObject *parent );
    ~GroupwiseServer();

    QString error() const { return mError; }

    bool login();
    bool logout();

    bool addIncidence( KCal::Incidence *, KCal::ResourceCached * );
    bool changeIncidence( KCal::Incidence * );
    bool deleteIncidence( KCal::Incidence * );

    bool readCalendarSynchronous( KCal::Calendar *cal );

    GroupWise::AddressBook::List addressBookList();

    bool readAddressBooksSynchronous( const QStringList &addrBookIds,
      KABC::ResourceCached * );

    bool insertAddressee( const QString &addrBookId, KABC::Addressee& );
    bool changeAddressee( const KABC::Addressee& );
    bool removeAddressee( const KABC::Addressee& );

    bool readFreeBusy( const QString &email, const QDate &start,
      const QDate &end, KCal::FreeBusy * );

    bool dumpData();
    void dumpFolderList();

    bool getDelta();

    bool getCategoryList();

    int gSoapOpen( struct soap *soap, const char *endpoint, const char *host,
      int port );
    int gSoapClose( struct soap *soap );
    int gSoapSendCallback( struct soap *soap, const char *s, size_t n );
    size_t gSoapReceiveCallback( struct soap *soap, char *s, size_t n );

    void emitReadAddressBookTotalSize( int );
    void emitReadAddressBookProcessedSize( int );

  signals:
    void readAddressBookTotalSize( int );
    void readAddressBookProcessedSize( int );

  protected:
    void dumpCalendarFolder( const std::string &id );

    void dumpFolder( ns1__Folder * );
    void dumpItem( ns1__Item * );
    void dumpAppointment( ns1__Appointment * );
    void dumpTask( ns1__Task * );
    void dumpMail( ns1__Mail * );

    bool checkResponse( int result, ns1__Status *status );

    void log( const QString &prefix, const char *s, size_t n );

  protected slots:
    void slotSslError();

  private:
    QString mUrl;
    QString mUser;
    QString mPassword;
    bool mSSL;

    std::string mSession;

    QString mUserName;
    QString mUserEmail;
    QString mUserUuid;

    std::string mCalendarFolder;
    
    struct soap *mSoap;

    KExtendedSocket *m_sock;

    QString mError;

    QString mLogFile;
};

#endif
