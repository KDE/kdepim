/*
    This file is part of libkpimexchange
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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
#ifndef KDEPIM_EXCHANGE_UPLOAD_H
#define KDEPIM_EXCHANGE_UPLOAD_H

#include <qstring.h>
#include <qwidget.h>
#include <kio/job.h>

#include <kdepimmacros.h>

#include <libkcal/calendar.h>
#include <libkcal/event.h>

namespace KPIM {

class ExchangeAccount;

class KDE_EXPORT ExchangeUpload : public QObject {
    Q_OBJECT
  public:
    ExchangeUpload( KCal::Event* event, ExchangeAccount* account, const QString& timeZoneId, QWidget* window=0 );
    ~ExchangeUpload();

  private slots:
    void slotPatchResult( KIO::Job * );
    void slotPropFindResult( KIO::Job * );
    void slotFindUidResult( KIO::Job * );

  signals:
    void startDownload();
    void finishDownload();
    void finished( ExchangeUpload* worker, int result, const QString& moreInfo );

  private:
    void tryExist();
    void startUpload( const KURL& url );
    void findUid( QString const& uid );
    
    ExchangeAccount* mAccount;
    KCal::Event* m_currentUpload;
    int m_currentUploadNumber;
    QString mTimeZoneId;
    QWidget* mWindow;
};

}

#endif
