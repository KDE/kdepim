/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef KDEPIM_EXCHANGE_UPLOAD_H
#define KDEPIM_EXCHANGE_UPLOAD_H

#include <qstring.h>
#include <kio/job.h>

#include <libkcal/calendar.h>
#include <libkcal/event.h>

class DwString;
class DwEntity;

namespace KPIM {

class ExchangeAccount;

class ExchangeUpload : public QObject {
    Q_OBJECT
  public:
    ExchangeUpload( KCal::Event* event, ExchangeAccount* account );
    ~ExchangeUpload();

  private slots:
    void slotPatchResult( KIO::Job * );
    void slotPropFindResult( KIO::Job * );
    void slotFindUidResult( KIO::Job * );

  signals:
    void startDownload();
    void finishDownload();
    void uploadFinished( ExchangeUpload* worker );

  private:
    void tryExist();
    void startUpload( KURL& url );
    void findUid( QString const& uid );
    
    KCal::Calendar *mCalendar;
    ExchangeAccount* mAccount;
    KCal::Event* m_currentUpload;
    int m_currentUploadNumber;
};

}

#endif
