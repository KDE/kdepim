/*
    This file is part of KDE.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef KIOSLAVE_OPENGROUPWARE_H
#define KIOSLAVE_OPENGROUPWARE_H

#include <kio/slavebase.h>

#include <QObject>

namespace KIO {
  class Job;
  class DavJob;
}


class OpenGroupware : public QObject, public KIO::SlaveBase
{
    Q_OBJECT
  public:
    OpenGroupware( const QCString &protocol, const QCString &pool,
                   const QCString &app );

    void get( const KUrl &url );

  protected:
    void debugMessage( const QString & );
    void errorMessage( const QString & );

    void getFreeBusy( const KUrl &url );
    void getCalendar( const KUrl &url );
    void getAddressbook( const KUrl &url );
  protected slots:
    void slotGetCalendarListingResult( KJob* );
    void slotGetCalendarResult( KJob* );
  private:
    KIO::DavJob *mListEventsJob;
};

#endif // KIOSLAVE_OPENGROUPWARE_H
