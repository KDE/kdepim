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

#include <qptrlist.h>
#include <qmap.h>

#include <kdebug.h>
#include <kparts/mainwindow.h>

#include <manipulatorpart.h>
#include <systemtray.h>

#include "profilemanager.h"
#include "konnectorprofilemanager.h"
#include "manpartservice.h"
#include "core.h"

class PartBar;
class QHBox;
class QWidgetStack;
class KSelectAction;

namespace KSync {

class KonnectorBar;

/**
 * The KitchenSync UI Shell
 * It's the MainWindow of the application. It'll load all parts
 * and do the basic communication between all parts
 */
class KSyncMainWindow : public Core
{
    Q_OBJECT
  public:

    /**
     * The KSyncMainWindow C'tor
     * @param widget parent widget
     * @param name The name
     * @param flags the flags
     */
    KSyncMainWindow( QWidget *widget = 0,
                     const char *name = 0,
                     WFlags f = WType_TopLevel );
    ~KSyncMainWindow();

    /**
     * @return the parent for KPart widgets
     */
    QWidget* widgetStack();

    /**
     * @return the SystemTray of KitchenSync
     */
    KSyncSystemTray *tray();

    /**
     * REMOVE
     * @deprecated
     */
    QString  currentId() const;

    /**
     * REMOVE
     * @deprecated
     */
    QMap<QString,QString> ids() const;

    /**
     * @return the currently enabled Profile
     */
    Profile currentProfile() const;

    /**
     * @return access to the profilemanager
     * @FIXME make const pointer to const object
     */
    ProfileManager *profileManager() const;

    /**
     * @return the current KonnectorProfile
     */
    KonnectorProfile currentKonnectorProfile() const;

    /**
     * @return the KonnectorProfileManager
     * @FIXME make const pointer to const object
     */
    KonnectorProfileManager *konnectorProfileManager() const;

    /**
     * @return a SyncUi
     */
    SyncUi *syncUi();

    /**
     * @return the preferred syncAlgorithm of KitchenSync
     */
    SyncAlgorithm *syncAlgorithm();

    /**
     * @return the all loaded ManipulatorParts
     */
    const QPtrList<ManipulatorPart> parts() const;

  private:
    virtual void initActions();
    void addPart( const ManPartService& );
    void addModPart( ManipulatorPart * );
    void initSystray ( void );
    void removeDeleted( const KonnectorProfile::ValueList& );
    void unloadLoaded( const KonnectorProfile::ValueList& toUnload,
                       KonnectorProfile::ValueList& items );
    void loadUnloaded( const KonnectorProfile::ValueList& toLoad,
                       KonnectorProfile::ValueList& items );
    void updateEdited( const KonnectorProfile::ValueList& edited );

  private slots:
    void slotKonnectorProfile();
    void slotProfile();
    void initProfileList();
    void initKonnectorList();
    void switchProfile( const Profile& prof );
    void switchProfile( KonnectorProfile& prof );
    void slotConfigProf();
    void slotConfigCur();
    void initKonnector();
    void initPlugins();
    void initProfiles();
    void slotSync();
    void slotBackup();
    void slotRestore();
    void slotConfigure();
    void slotActivated(ManipulatorPart *);
    void slotQuit();
    void slotKonnectorBar(bool );

    void slotPreferences();
    void updateConfig();

    /* slots for the KonnectorManager */
  private slots:
    void slotSync( Konnector *, SynceeList );
    void slotKonnectorProg( Konnector *, const Progress & );
    void slotKonnectorErr( Konnector *, const Error & );

    /* slots for the ManipulatorParts */
    void slotPartProg( ManipulatorPart *, int );
    void slotPartProg( ManipulatorPart *, const Progress & );
    void slotPartErr( ManipulatorPart *, const Error & );
    void slotPartSyncStatus( ManipulatorPart *, int );

  private:
    PartBar *m_bar;
    QHBox *m_lay;
    QWidgetStack *m_stack;
    // loaded parts
    QPtrList<ManipulatorPart> m_parts;
    QPtrListIterator<ManipulatorPart> *m_partsIt;
    SynceeList m_outSyncee;
    SynceeList m_inSyncee;
    bool m_isSyncing;

    ManPartService::ValueList m_partsLst;
    KSyncSystemTray *m_tray;

    KonnectorProfileManager *m_konprof;
    KSelectAction *m_konAct;
    KSelectAction *m_profAct;
    ProfileManager *m_prof;
    SyncUi *m_syncUi;
    SyncAlgorithm *m_syncAlg;
    QString m_currentId;
    KonnectorBar *m_konBar;
};

}

#endif
