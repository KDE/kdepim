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

#include <string>

#include <kabc/addressee.h>

namespace KABC {
class ResourceCached;
}

namespace KCal {
class Calendar;
class ResourceCached;
}

class GroupwiseServer;

class GWJob
{
  public:
    GWJob( struct soap *soap, const QString &url, const std::string &session );

  protected:
    struct soap *mSoap;
    QString mUrl;
    const std::string mSession;
};

class ReadAddressBooksJob : public GWJob
{
  public:
    ReadAddressBooksJob( GroupwiseServer *server, struct soap *soap,
      const QString &url,
      const std::string &session );

    void setAddressBookIds( const QStringList& );

    // we need the resource here for doing uid mapping
    void setResource( KABC::ResourceCached * );

    void run();

  protected:
    void readAddressBook( std::string& );

  private:
    GroupwiseServer *mServer;
    QStringList mAddressBookIds;
    KABC::ResourceCached *mResource;
    int mProgress;
};

class ReadCalendarJob : public GWJob
{
  public:
    ReadCalendarJob( struct soap *soap, const QString &url,
      const std::string &session );

    void setCalendarFolder( std::string* );

    // we need the resource here for doing uid mapping
    void setResource( KCal::ResourceCached * );
    
    void setCalendar( KCal::Calendar * );

    void run();

  protected:
    void readCalendarFolder( const std::string &id );

  private:
    std::string *mCalendarFolder;
    KCal::ResourceCached *mResource;
    KCal::Calendar *mCalendar;
};

#endif
