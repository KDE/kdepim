/*
† † † †This file is part of the OPIE Project
† † † †Copyright (c)  2002 Holger Freyther <zecke@handhelds.org>
† †                   2002 Maximilian Reiﬂ <harlekin@handhelds.org>
† † † † † †

† † † † † † † †=.
† † † † † † †.=l.
† † † † † †.>+-=
†_;:, † † .> † †:=|.         This library is free software; you can
.> <`_, † > †. † <=          redistribute it and/or  modify it under
:`=1 )Y*s>-.-- † :           the terms of the GNU General Public
.="- .-=="i, † † .._         License as published by the Free Software
†- . † .-<_> † † .<>         Foundation; either version 2 of the License,
† † †._= =} † † † :          or (at your option) any later version.
† † .%`+i> † † † _;_.
† † .i_,=:_. † † †-<s.       This program is distributed in the hope that
† † †+ †. †-:. † † † =       it will be useful,  but WITHOUT ANY WARRANTY;
† † : .. † †.:, † † . . .    without even the implied warranty of
† † =_ † † † †+ † † =;=|`    MERCHANTABILITY or FITNESS FOR A
† _.=:. † † † : † †:=>`:     PARTICULAR PURPOSE. See the GNU
..}^=.= † † † = † † † ;      Library General Public License for more
++= † -. † † .` † † .:       details.
†: † † = †...= . :.=-
†-. † .:....=;==+<;          You should have received a copy of the GNU
† -_. . . † )=. †=           Library General Public License along with
† † -- † † † †:-=`           this library; see the file COPYING.LIB.
                             If not, write to the Free Software Foundation,
                             Inc., 59 Temple Place - Suite 330,
                             Boston, MA 02111-1307, USA.

*/

#ifndef KSYNCMAINWINDOW_H
#define KSYNCMAINWINDOW_H

#include <qptrlist.h>
#include <qmap.h>

#include <kdebug.h>
#include <kparts/mainwindow.h>


#include <manipulatorpart.h>
#include <ksync_systemtray.h>
#include <profilemanager.h>
#include <konnectorprofilemanager.h>
#include <manpartservice.h>

class PartBar;
class QHBox;
class QWidgetStack;
class KSelectAction;

namespace KSync {
    class KonnectorManager;
    class SyncUi;
    class SyncAlgorithm;
    // no idea why we have this window
//    enum SyncStatus {SYNC_START=0, SYNC_SYNC, SYNC_STOP };
    enum KonnectorMode { KONNECTOR_ONLINE=0,  KONNECTOR_OFFLINE };

    class KSyncMainWindow : public KParts::MainWindow {
       Q_OBJECT
    public:
        KSyncMainWindow(QWidget *widget =0l,
                        const char *name = 0l,
                        WFlags f = WType_TopLevel );
        ~KSyncMainWindow();

        KSyncSystemTray *tray();
        KonnectorManager*  konnector();
        QString  currentId()const;
        QMap<QString,QString> ids()const;
        Profile currentProfile()const;
        ProfileManager *profileManager()const;
        KonnectorProfile konnectorProfile() const;
        KonnectorProfileManager* konnectorManager() const;
        SyncUi* syncUi();
        SyncAlgorithm* syncAlgorithm();


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

        //
        PartBar *m_bar;
        QHBox *m_lay;
        QWidgetStack *m_stack;
        // loaded parts
        QPtrList<ManipulatorPart> m_parts;
        ManPartService::ValueList m_partsLst;
        KSyncSystemTray *m_tray;

        KonnectorManager *m_konnector;
        KonnectorProfileManager* m_konprof;
        KSelectAction* m_konAct;
        KSelectAction* m_profAct;
        ProfileManager* m_prof;
        SyncUi *m_syncUi;
        SyncAlgorithm* m_syncAlg;
        QString m_currentId;
        // udi + Identify
        QMap<QString, QString> m_ids;

    signals:
        void profileChanged(const Profile& oldProfile   );
        void konnectorChanged( const QString & );
        void konnectorChanged( const KonnectorProfile& oldProf );
        void konnectorStateChanged( const QString &,  int mode );
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
        void slotSync(const QString &udi, Syncee::PtrList );
        void slotStateChanged( const QString& udi,  bool connected );
        void slotKonnectorError( const QString& udi,
                                 int error,
                                 const QString &id );
    };
};

#endif
