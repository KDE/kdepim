/*
† † † †This file is part of the OPIE Project
† † † †Copyright (c)  2002,2003 Holger Freyther <zecke@handhelds.org>
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
    typedef QString UDI;
    class KonnectorManager;
    class SyncUi;
    class SyncAlgorithm;
    class KonnectorBar;

    enum KonnectorMode { KONNECTOR_ONLINE=0,  KONNECTOR_OFFLINE };

    class KSyncMainWindow : public KParts::MainWindow {
       Q_OBJECT
    public:
        KSyncMainWindow(QWidget *widget =0l,
                        const char *name = 0l,
                        WFlags f = WType_TopLevel );
        ~KSyncMainWindow();

        QWidget* widgetStack();
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
        const QPtrList<ManipulatorPart> parts()const;


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


    signals:
        void profileChanged(const Profile& oldProfile   );
        void konnectorChanged( const UDI& );
        void konnectorChanged( const KonnectorProfile& oldProf );
        void konnectorProgress( const UDI&, const Progress& );
        void konnectorError( const UDI&, const Error& );
        void konnectorDownloaded( const UDI&, Syncee::PtrList );
        void partChanged( ManipulatorPart* newPart );
        void partProgress( ManipulatorPart* part, const Progress& );
        void partError( ManipulatorPart* part, const Error& );
        void startSync();
        void startSync(ManipulatorPart*);
        void doneSync();
        void doneSync(ManipulatorPart* );

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

        /* slots for the KonnectorManager */
    private slots:
        void slotSync(const QString &udi, Syncee::PtrList );
        void slotKonnectorProg( const UDI&, const Progress& );
        void slotKonnectorErr( const UDI&, const Error& );

        /* slots for the ManipulatorParts */
        void slotPartProg( ManipulatorPart*, int );
        void slotPartProg( ManipulatorPart*, const Progress& );
        void slotPartErr( ManipulatorPart*, const Error& );
        void slotPartSyncStatus( ManipulatorPart*, int );

    private:
        PartBar *m_bar;
        QHBox *m_lay;
        QWidgetStack *m_stack;
        // loaded parts
        QPtrList<ManipulatorPart> m_parts;
        QPtrListIterator<ManipulatorPart>* m_partsIt;
        Syncee::PtrList m_outSyncee;
        Syncee::PtrList m_inSyncee;
        bool m_isSyncing;

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
        KonnectorBar* m_konBar;

        struct Data;
        Data* d;
    };
};

#endif
