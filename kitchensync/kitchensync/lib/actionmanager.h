/*
    This file is part of KitchenSync.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KSYNC_ACTIONMANAGER_H
#define KSYNC_ACTIONMANAGER_H

#include <qptrlist.h>
#include <qmap.h>

#include <kdebug.h>
#include <kparts/mainwindow.h>

#include <systemtray.h>

#include "profilemanager.h"
#include "kitchensync.h"

class KSelectAction;

namespace KSync {

class KonnectorBar;
class KitchenSync;

/**
 * The KitchenSync UI Shell
 * It's the MainWindow of the application. It'll load all parts
 * and do the basic communication between all parts
 */
class KDE_EXPORT ActionManager
{
  public:
    ActionManager( KActionCollection * );
    ~ActionManager();

    void setView( KitchenSync * );

    int currentProfile();
    void setProfiles( const QStringList &profiles );

    void initActions();
    
    void readConfig();
    void writeConfig();
    
  private:
    KActionCollection *mActionCollection;
    KitchenSync *mView;

    KSelectAction *m_profAct;
};

}

#endif
