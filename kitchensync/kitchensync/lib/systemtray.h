/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
† † Copyright (c) 2002 Maximilian Reiﬂ <harlekin@handhelds.org>

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

/*
 since it is not clear yet, how the connection notification is done,
 not really useful yet and these part are not yet in.
*/

#ifndef KSYNC_SYSTEMTRAY_H
#define KSYNC_SYSTEMTRAY_H


#include <ksystemtray.h>
#include <kpixmap.h>

class QTimer;
class QPoint;

class KPopupMenu;

namespace KSync {
    /**
     * Our SystemTray but currently not really used
     */
    class KSyncSystemTray : public KSystemTray {

        Q_OBJECT

    public:
        KSyncSystemTray(QWidget* parent = 0,  const char* name = 0);
        ~KSyncSystemTray();

        KPopupMenu *getContextMenu() const { return contextMenu(); };
	
        void gotConnection( QPixmap );
	void setName( QString& );
	void setState( bool );
        void noConnection();

    private:
        QPixmap ksyncIconConnected;
        QPixmap ksyncIconDisconnected;

        //   protected:
        // virtual void mousePressEvent( QMouseEvent *mEvent);

   
        //signals:
        //   void leftClicked ( const QPoint );
        // void rightClicked ( const QPoint );
    };

}

#endif

