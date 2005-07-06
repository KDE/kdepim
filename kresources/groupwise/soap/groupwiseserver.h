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
#include <qmap.h>
#include <qobject.h>
#include <qstring.h>
#include <qthread.h>
#include <qvaluelist.h>

#include <string>

#include <kabc/addressee.h>
#include <libkcal/freebusy.h>

#include "gwjobs.h"

namespace KABC {
  class AddresseeList;
  class Resource;
}

namespace KCal {
class Calendar;
class Incidence;
class ResourceCached;
}

class ngwt__Settings;

class KExtendedSocket;

struct soap;

class ngwt__Folder;
class ngwt__Item;
class ngwt__Appointment;
class ngwt__Mail;
class ngwt__Task;
class ngwt__Status;
class GroupWiseBinding;

namespace GroupWise {

class AddressBook
{
  public:
    typedef QValueList<AddressBook> List;

    AddressBook() : isPersonal( false ), isFrequentContacts( false ), isSystemAddressBook( false ) {}

    QString id;
    QString name;
    QString description;
    bool isPersonal;
    bool isFrequentContacts;
    bool isSystemAddressBook;
};

class DeltaInfo
{
  public:
    long count;
    long firstSequence;
    long lastSequence;
    long lastTimePORebuild;
};
}

class GroupwiseServer : public QObject
{
  Q_OBJECT

  public:
    enum RetractCause { DueToResend, Other };
    GroupwiseServer( const QString &url, const QString &user,
                     const QString &password, QObject *parent );
    ~GroupwiseServer();

    QString error() const { return mError; }

    bool login();
    bool logout();

    bool addIncidence( KCal::Incidence *, KCal::ResourceCached * );
    bool changeIncidence( KCal::Incidence * );
    bool deleteIncidence( KCal::Incidence * );

    /**
     * @brief Retract a meeting request.
     * This is needed to change a meeting, because you need to retract it from others' mailboxes before resending.
     * @param causedByResend indicate if the retraction is caused by a resend, suppresses the retraction message in favour of the resent meeting.
     */
    bool retractRequest( KCal::Incidence *, RetractCause cause );

    bool readCalendarSynchronous( KCal::Calendar *cal );

    GroupWise::AddressBook::List addressBookList();

    bool readAddressBooksSynchronous( const QStringList &addrBookIds );
    bool updateAddressBooks( const QStringList &addrBookIds,
      const unsigned int startSequenceNumber );

    bool insertAddressee( const QString &addrBookId, KABC::Addressee& );
    bool changeAddressee( const KABC::Addressee& );
    bool removeAddressee( const KABC::Addressee& );

    bool readFreeBusy( const QString &email, const QDate &start,
      const QDate &end, KCal::FreeBusy * );

    bool dumpData();
    void dumpFolderList();

//     bool getDelta();
    GroupWise::DeltaInfo getDeltaInfo( const QStringList & addressBookIds );

    bool getCategoryList();

    int gSoapOpen( struct soap *soap, const char *endpoint, const char *host,
      int port );
    int gSoapClose( struct soap *soap );
    int gSoapSendCallback( struct soap *soap, const char *s, size_t n );
    size_t gSoapReceiveCallback( struct soap *soap, char *s, size_t n );

    void emitReadAddressBookTotalSize( int );
    void emitReadAddressBookProcessedSize( int );
    void emitErrorMessage( const QString &, bool );
    void emitGotAddressees( const KABC::Addressee::List );

    bool readUserSettings( ngwt__Settings *&settings );
    bool modifyUserSettings( QMap<QString, QString> & );

  signals:
    void readAddressBookTotalSize( int );
    void readAddressBookProcessedSize( int );
    void errorMessage( const QString &, bool );
    void gotAddressees( const KABC::Addressee::List );

  protected:
    void dumpCalendarFolder( const std::string &id );

    void dumpFolder( ngwt__Folder * );
    void dumpItem( ngwt__Item * );
    void dumpAppointment( ngwt__Appointment * );
    void dumpTask( ngwt__Task * );
    void dumpMail( ngwt__Mail * );

    bool checkResponse( int result, ngwt__Status *status );

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
    std::string mCheckListFolder;

    struct soap *mSoap;
    GroupWiseBinding *mBinding;
    
    KExtendedSocket *m_sock;

    QString mError;

    QString mLogFile;
};

#endif
