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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef GROUPWISE_JOBS_H
#define GROUPWISE_JOBS_H

#include <qobject.h>
#include <qthread.h>

#include <string>

#include <kabc/addressee.h>
#include <libkdepim/weaver.h>

namespace KCal {
class Calendar;
}

class GWJob : public KPIM::ThreadWeaver::Job
{
  public:
    enum Type { ReadAddressBooks, ReadCalendar };

    GWJob( Type type, const QString &url, const std::string &session,
           QObject *parent );
    virtual ~GWJob();

    Type type() const { return mType; }

    virtual void processEvent( KPIM::ThreadWeaver::Event* );

  protected:
    Type mType;
    struct soap *mSoap;
    QString mUrl;
    const std::string mSession;
};

class ReadAddressBooksJob : public GWJob
{
  public:
    ReadAddressBooksJob( const QString &url, const std::string &session,
                         QObject *parent );

    void setAddressBookIds( const QStringList& );

    KABC::Addressee::List addresseeList() { return mAddresseeList; }

  protected:
    void run();
    void readAddressBook( std::string& );

  private:
    QStringList mAddressBookIds;
    KABC::Addressee::List mAddresseeList;
};

class ReadCalendarJob : public GWJob
{
  public:
    ReadCalendarJob( const QString &url, const std::string &session,
                     QObject *parent );

    void setCalendar( KCal::Calendar* );
    void setCalendarFolder( std::string* );

  protected:
    void run();
    void readCalendarFolder( const std::string &id );

  private:
    KCal::Calendar *mCalendar;
    std::string *mCalendarFolder;
};

#endif
