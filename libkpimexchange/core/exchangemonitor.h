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
#ifndef KDEPIM_EXCHANGE_MONITOR_H
#define KDEPIM_EXCHANGE_MONITOR_H

#include <qstring.h>
#include <qmap.h>
//#include <qwidget.h>
#include <qhostaddress.h>

#include <kurl.h>
#include <kio/job.h>

#include <libkcal/calendar.h>
#include <libkcal/event.h>

class QSocketDevice;
class QSocketNotifier;
class QTextStream;

namespace KPIM {

class ExchangeAccount;

class ExchangeMonitor : public QObject {
    Q_OBJECT
  public:
    typedef long ID;
    typedef QValueList<ID> IDList;

    enum { CallBack, Poll };
    enum { Delete,  /** Any:              0   The message or folder subscribed to was deleted. 
                        Folder            1   A message or folder was deleted from the folder.  */
           Move,    /** Any:              0   The message or folder was moved. 
                        Folder            1   A message or folder was moved from or to the folder.  */
           Newmail, /** Mailbox or Folder Any Special new mail update. 
                        Message           Any Not valid - return 400 (Bad Request). */
           Update,  /** Message           0   The message was modified (either properties or body). 
                        Folder            0   Properties of the folder were modified. 
                        Folder            1   A message or sub-folder was created in the folder, copied to 
                                              the folder, moved to or from the folder, deleted from the folder, 
                                              modified in the folder, or the folder properties were modified. */
           UpdateNewMember, /**  Any      0   Not valid - return 400 (Bad Request). 
                        Existing Folder   1   A message or sub-folder was created in the folder, copied to 
                                              the folder, or moved to the folder. */
           Any      /** Message           1   Treat as depth = 0. */
    };

    ExchangeMonitor( ExchangeAccount* account, int pollMode, const QHostAddress& ownInterface  );
    ~ExchangeMonitor();
    void addWatch( const KURL &url, int mode, int depth );
    void removeWatch( const KURL &url );
    void removeWatch( ID id );

  signals:
    void notify( const QValueList<long>& IDs, const QValueList<KURL>& urls );
//    void added( ID id, const KURL& url );
//    void removed( ID id, const KURL& url );

    void error( int result, const QString& moreInfo );

  private slots:
    void slotSubscribeResult( KIO::Job * );
    void slotUnsubscribeResult( KIO::Job * );
    void slotPollTimer();
    void poll( const IDList& IDs );
    void slotPollResult( KIO::Job * );
    void slotRenewTimer();
    void slotRenewResult( KIO::Job * );
    void slotActivated(int socket);

  private:
//    void init();

    QMap<ID,KURL> mSubscriptionMap;
    QSocketDevice *mSocket;
    QSocketNotifier* mNotifier;
    QTextStream *mStream;
    ExchangeAccount* mAccount;
    int mSubscriptionLifetime;
    // QString mSubscriptionId;
    QTimer* mPollTimer;
    QTimer* mRenewTimer;
    int mPollMode;
};

}

#endif
