/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
† † Copyright (c) 2002 Maximilian Reiﬂ <harlekin@handhelds.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KSYNC_KITCHENSYNC_H
#define KSYNC_KITCHENSYNC_H

#include "actionpart.h"
#include "profilemanager.h"
#include "actionpartservice.h"
#include "core.h"
#include "systemtray.h"

#include <kdebug.h>
#include <kparts/mainwindow.h>

#include <qptrlist.h>
#include <qmap.h>

class QHBox;
class QWidgetStack;
class KSelectAction;

namespace KSync {

class PartBar;
class KonnectorBar;
class MainWindow;
class ActionManager;
class Engine;

/**
  The KitchenSync UI Shell
  It's the MainWindow of the application. It'll load all parts
  and do the basic communication between all parts
*/
class KitchenSync : public Core
{
    Q_OBJECT
  public:
    /**
      The KitchenSync C'tor
      @param actionManager is the name of the ActionManager
      @param parent is the parent widget
    */
    KitchenSync( ActionManager *actionManager, QWidget *parent );
    ~KitchenSync();

    void readProfileConfig();
    void writeProfileConfig();

    /**
      @return the parent for KPart widgets
    */
    QWidget *widgetStack();

    /**
      @return the SystemTray of KitchenSync
    */
    KSyncSystemTray *tray();

    /**
      @return the currently enabled Profile
    */
    Profile currentProfile() const;

    /**
      @return access to the profilemanager
    */
    // FIXME make const pointer to const object
    ProfileManager *profileManager() const;

    Engine *engine() const;

    /**
      @return a SyncUi
    */
    SyncUi *syncUi();

    /**
      @return the all loaded ActionParts
    */
    const QPtrList<ActionPart> parts() const;

  public slots:
    void initProfiles();
    /**
      Activates the profile selected in the GUI.
    */
    void activateProfile();

  private:
    void addPart( const ActionPartService & );
    void initSystray ( void );

  private slots:
    /**
      Propagate the list of configure profiles to the GUI.
    */
    void initProfileList();
    void activateProfile( const Profile &prof );
    /**
      Open configuration dialog for configuring the profiles.
    */
    void configureProfiles();
    void configureCurrentProfile();
    void slotSync();
    void slotActivated( ActionPart * );
    void slotKonnectorBar( bool );

    void slotPreferences();
    void updateConfig();

    /* slots for the KonnectorManager */
  private slots:
    void slotKonnectorProg( Konnector *, const Progress & );
    void slotKonnectorErr( Konnector *, const Error & );

    /* slots for the ActionParts */
    void slotPartProg( ActionPart *, int );
    void slotPartProg( ActionPart *, const Progress & );
    void slotPartErr( ActionPart *, const Error & );
    void slotPartSyncStatus( ActionPart *, int );

  private:
    ActionManager *mActionManager;

    PartBar *m_bar;
    QWidgetStack *m_stack;
    // loaded parts
    QPtrList<ActionPart> m_parts;
    QPtrListIterator<ActionPart> *m_partsIt;
    bool m_isSyncing;

    KSyncSystemTray *m_tray;

    ProfileManager *m_profileManager;
    SyncUi *m_syncUi;

    Engine *mEngine;
    QMap<ActionPart *, QWidget *> mActionWidgetMap;
};

}

#endif
