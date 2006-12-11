/*
    This file is part of KDE.

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef GROUPWISE_JOBS_H
#define GROUPWISE_JOBS_H

#include <qobject.h>

#include <string>

#include <kabc/addressee.h>

namespace KABC {
class Resource;
}

namespace KCal {
class Calendar;
}

struct ReadItemCounts {
  unsigned int appointments;
  unsigned int notes;
  unsigned int tasks;
};

class GroupwiseServer;

class GWJob
{
  public:
    GWJob( GroupwiseServer *server, struct soap *soap, const QString &url, const std::string &session );
    int error() const { return mError; }
  protected:
    GroupwiseServer *mServer;
    struct soap *mSoap;
    QString mUrl;
    const std::string mSession;
    int mError;
};

class ReadAddressBooksJob : public GWJob
{
  public:
    ReadAddressBooksJob( GroupwiseServer *server, struct soap *soap,
      const QString &url,
      const std::string &session );

    void setAddressBookIds( const QStringList& );

    void run();

  protected:
    void readAddressBook( std::string& );

  private:
    QStringList mAddressBookIds;
    KABC::Resource *mResource;
    int mProgress;
};

class ReadCalendarJob : public GWJob
{
  public:
    ReadCalendarJob( GroupwiseServer *server, struct soap *soap, const QString &url,
      const std::string &session );

    void setCalendarFolder( std::string* );
    void setChecklistFolder( std::string* );

    void setCalendar( KCal::Calendar * );

    void run();

  protected:
    void readCalendarFolder( const std::string &id, ReadItemCounts & counts );

  private:
    std::string *mCalendarFolder;
    std::string *mChecklistFolder;
    KCal::Calendar *mCalendar;
};

class UpdateAddressBooksJob : public GWJob
{
  public:
    UpdateAddressBooksJob( GroupwiseServer *server, struct soap *soap,
      const QString &url,
      const std::string &session );

    /** set the address book IDs to update - at the moment this is only the System Address Book (SAB) */
    void setAddressBookIds( const QStringList& );

    /**
    * set the sequence number to start reading deltas from (usually the last sequenec number
    * we have in the local copy of the System Address Book).
    */
    void setStartSequenceNumber( const unsigned long startSeqNo );
    /**
    * set the time of the last server (PO) rebuild
    */
    void setLastPORebuildTime( const unsigned long lastPORebuildTime);

    void run();
  protected:
    void updateAddressBook( std::string& );

  private:
    QStringList mAddressBookIds;
    KABC::Resource *mResource;
    int mProgress;
    unsigned long mLastPORebuildTime;
    unsigned long mStartSequenceNumber; // first and last sequence numbers define the current state of the system addressbook
                              // and are used to determine which deltas to download
};

#endif
