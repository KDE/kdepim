/*
� � � � � � � �=.            This file is part of the OPIE Project
� � � � � � �.=l.            Copyright (c)  2002 Holger Freyther <zecke@handhelds.org>
� � � � � �.>+-=                            2002 Maximilian Rei� <harlekin@handhelds.org>
�_;:, � � .> � �:=|.         This library is free software; you can
.> <`_, � > �. � <=          redistribute it and/or  modify it under
:`=1 )Y*s>-.-- � :           the terms of the GNU Library General Public
.="- .-=="i, � � .._         License as published by the Free Software
�- . � .-<_> � � .<>         Foundation; either version 2 of the License,
� � �._= =} � � � :          or (at your option) any later version.
� � .%`+i> � � � _;_.
� � .i_,=:_. � � �-<s.       This library is distributed in the hope that
� � �+ �. �-:. � � � =       it will be useful,  but WITHOUT ANY WARRANTY;
� � : .. � �.:, � � . . .    without even the implied warranty of
� � =_ � � � �+ � � =;=|`    MERCHANTABILITY or FITNESS FOR A
� _.=:. � � � : � �:=>`:     PARTICULAR PURPOSE. See the GNU
..}^=.= � � � = � � � ;      Library General Public License for more
++= � -. � � .` � � .:       details.
�: � � = �...= . :.=-
�-. � .:....=;==+<;          You should have received a copy of the GNU
� -_. . . � )=. �=           Library General Public License along with
� � -- � � � �:-=`           this library; see the file COPYING.LIB.
                             If not, write to the Free Software Foundation,
                             Inc., 59 Temple Place - Suite 330,
                             Boston, MA 02111-1307, USA.

*/

/*
 since it is not clear yet, how the connection notification is done,
 not really usefull yet and these part are not yet in.
*/

#ifndef KSYNC_SYSTEMTRAY_H
#define KSYNC_SYSTEMTRAY_H


#include <ksystemtray.h>
#include <kpixmap.h>

class QTimer;
class QPoint;

class KPopupMenu;

namespace KitchenSync {

    class KSyncSystemTray : public KSystemTray {

        Q_OBJECT

    public:
        KSyncSystemTray(QWidget* parent = 0,  const char* name = 0);
        ~KSyncSystemTray();

        KPopupMenu *getContextMenu() const { return contextMenu(); };

        void gotConnection( QPixmap );
        void noConnection();

    private:
        QPixmap ksyncIconConnected;
        QPixmap ksyncIconDisconnected;

        bool gotDeviceConnection;

        //   protected:
        // virtual void mousePressEvent( QMouseEvent *mEvent);

    private slots:
        void slotPixmap();

        //signals:
        //   void leftClicked ( const QPoint );
        // void rightClicked ( const QPoint );
    };

};

#endif

