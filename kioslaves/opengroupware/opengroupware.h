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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef GROUPWISE_H
#define GROUPWISE_H

#include <kio/slavebase.h>

#include <qobject.h>

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

    void get( const KURL &url );

  protected:
    void debugMessage( const QString & );
    void errorMessage( const QString & );

    void getFreeBusy( const KURL &url );
    void getCalendar( const KURL &url );
    void getAddressbook( const KURL &url );
  protected slots:
    void slotGetCalendarListingResult( KIO::Job* );
    void slotGetCalendarResult( KIO::Job* );
  private:
    KIO::DavJob *mListEventsJob;
};

#endif
