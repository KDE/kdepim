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

    /**
     * The KitchenSync UI Shell
     * It's the MainWindow of the application. It'll load all parts
     * and do the basic communication between all parts
     */
    class KSyncMainWindow : public KParts::MainWindow {
       Q_OBJECT
    public:

	/**
	 * The KSyncMainWindow C'tor
	 * @param widget parent widget
	 * @param name The name
	 * @param flags the flags
	 */
        KSyncMainWindow(QWidget *widget =0l,
                        const char *name = 0l,
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
	 * @return the KitchenSync KonnectorManager
	 */
        KonnectorManager*  konnector();

	/**
	 * REMOVE
	 * @deprecated
	 */
        QString  currentId()const;

	/**
	 * REMOVE
	 * @deprecated
	 */
        QMap<QString,QString> ids()const;

	/**
	 * @return the currently enabled Profile
	 */
        Profile currentProfile()const;

	/**
	 * @return access to the profilemanager
	 * @FIXME make const pointer to const object
	 */
        ProfileManager *profileManager()const;

	/**
	 * @return the current KonnectorProfile
	 */
        KonnectorProfile konnectorProfile() const;

	/**
	 * @return the KonnectorProfileManager
	 * @FIXME make const pointer to const object
	 */
        KonnectorProfileManager* konnectorManager() const;

	/**
	 * @return a SyncUi
	 */
        SyncUi* syncUi();

	/**
	 * @return the prefered syncAlgorithm of KitchenSync
	 */
        SyncAlgorithm* syncAlgorithm();

	/**
	 * @return the all loaded ManipulatorParts
	 */
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
	void updateEdited( const KonnectorProfile::ValueList& edited );


    signals:
	/**
	 * This signal gets emitted whenever the Profile
	 * is changed.
	 * @param oldProfile the previously enabled profile
	 */
        void profileChanged(const Profile& oldProfile   );

	/**
	 * This signal gets emitted whenever the Profile
	 * is changed.
	 * @param udi The old UDI of the previously enabled profile
	 */
        void konnectorChanged( const UDI& udi);

	/**
	 * This signals gets emitted on KonnectorProfile switch.
	 * @param oldProf the old Profile
	 */
        void konnectorChanged( const KonnectorProfile& oldProf );

	/**
	 * signal emitted when progress from the konnectorProgress arrived
	 * @param udi The UDI of the KonnectorPlugin
	 * @param prog The Progress
	 */
        void konnectorProgress( const UDI& udi , const Progress& prog);

	/**
	 * @param UDI the UDI of the KonnectorPlugin
	 * @param err the error
	 */
        void konnectorError( const UDI&, const Error& err);

	/**
	 * This signal gets emitted when the KonnectorManager
	 * downloaded a list of files
	 * @param udi The UDI where the Syncee comes from
	 * @param lst The downloaded Syncee
	 */
        void konnectorDownloaded( const UDI& udi, Syncee::PtrList lst);

	/**
	 * Whenever the currently activated parts changed
	 * @param newPart the newly activated part
	 */
        void partChanged( ManipulatorPart* newPart );

	/**
	 * progress coming from one part
	 * @param part where the progress comes from, 0 if from MainWindow
	 * @param prog The progress
	 */
        void partProgress( ManipulatorPart* part, const Progress& prog);

	/**
	 * error coming from one part
	 * @param part where the error comes from, 0 if from MainWindow
	 * @param err The error
	 */
        void partError( ManipulatorPart* part, const Error& error);

	/**
	 * emitted when ever sync starts
	 */
        void startSync();

	/**
	 * emitted when a part is asked to sync
	 */
        void startSync(ManipulatorPart*);

	/**
	 * emitted when done with syncing
	 */
        void doneSync();

	/**
	 * emitted when one part is done with syncing
	 */
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
