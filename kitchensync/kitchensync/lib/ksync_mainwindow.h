/*
� � � �This file is part of the OPIE Project
� � � �Copyright (c)  2002 Holger Freyther <zecke@handhelds.org>
� �                   2002 Maximilian Rei� <harlekin@handhelds.org>
� � � � � �

� � � � � � � �=.
� � � � � � �.=l.
� � � � � �.>+-=
�_;:, � � .> � �:=|.         This library is free software; you can
.> <`_, � > �. � <=          redistribute it and/or  modify it under
:`=1 )Y*s>-.-- � :           the terms of the GNU General Public
.="- .-=="i, � � .._         License as published by the Free Software
�- . � .-<_> � � .<>         Foundation; either version 2 of the License,
� � �._= =} � � � :          or (at your option) any later version.
� � .%`+i> � � � _;_.
� � .i_,=:_. � � �-<s.       This program is distributed in the hope that
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

#ifndef ksync_mainwindow_h
#define ksync_mainwindow_h

//#include <qptrlist.h>

//#include <qwidgetstack.h>

#include <kparts/mainwindow.h>

//#include "ksync_systemtray.h"

class Konnector;
//class PartBar;
namespace KitchenSync {

    class KSyncMainWindow : public KParts::MainWindow {
        Q_OBJECT
    public:
        KSyncMainWindow(QWidget *widget =0l,
                        const char *name = 0l,
                        WFlags f = WType_TopLevel );
        ~KSyncMainWindow();
        //KSyncSystemTray *tray();
        //Konnector*  konnector();
   };
};


#endif
