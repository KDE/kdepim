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
#ifndef KSYNC_MAINWINDOW_H
#define KSYNC_MAINWINDOW_H

#include "profilemanager.h"
#include "kitchensync.h"
#include "systemtray.h"

#include <kdebug.h>
#include <kparts/mainwindow.h>

#include <qptrlist.h>
#include <qmap.h>

class PartBar;
class QHBox;
class QWidgetStack;
class KSelectAction;

namespace KSync {

class KonnectorBar;
class KitchenSync;
class ActionManager;

class KDE_EXPORT MainWindow : public KParts::MainWindow
{
    Q_OBJECT
  public:
    MainWindow( QWidget *widget = 0, const char *name = 0 );
    ~MainWindow();

    int currentProfile();
    void setProfiles( const QStringList &profiles );

  private:
    ActionManager *mActionManager;
  
    KitchenSync *mView;

    KSelectAction *m_profAct;

    KonnectorBar *m_konBar;
};

}

#endif
