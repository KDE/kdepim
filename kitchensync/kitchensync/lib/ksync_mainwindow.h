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



#include <qptrlist.h>
#include <qmap.h>

#include <kparts/mainwindow.h>


#include <manipulatorpart.h>
#include <ksync_systemtray.h>
#include <ksync_profile.h>

class PartBar;
class QHBox;
class QWidgetStack;

class Konnector;

namespace KitchenSync {
    // no idea why we have this window
    enum SyncStatus {SYNC_START=0, SYNC_SYNC, SYNC_STOP };
    enum KonnectorMode { KONNECTOR_ONLINE=0,  KONNECTOR_OFFLINE };
    class KSyncMainWindow : public KParts::MainWindow {
       Q_OBJECT
    public:
        KSyncMainWindow(QWidget *widget =0l, const char *name = 0l, WFlags f = WType_TopLevel );
        ~KSyncMainWindow();
        KSyncSystemTray *tray();
        Konnector*  konnector();
        QString  currentId()const;
        QMap<QString,QString> ids()const;
    private:
        virtual void initActions();
        void saveCurrentProfile();
        void addModPart( ManipulatorPart * );
        void initSystray ( void );
        void setupKonnector(const KDevice &udi,  const QString &id);
        PartBar *m_bar;
        QHBox *m_lay;
        QWidgetStack *m_stack;
        QPtrList<ManipulatorPart> m_parts;
        KSyncSystemTray *m_tray;
        Konnector *m_konnector;
        QString m_currentId;
        // udi + Identify
        QMap<QString, QString> m_ids;
        Profile m_profile; //  QValueList if we support more than opie
    signals:
        void konnectorChanged( const QString & );
        void konnectorStateChanged( const QString &,  int mode );
   private slots:
        void initKonnector();
        void initPlugins();
        void slotSync();
        void slotBackup();
        void slotRestore();
        void slotConfigure();
        void slotActivated(ManipulatorPart *);
        void slotQuit();
        void slotSync(const QString &udi, QPtrList<KSyncEntry> );
        void slotStateChanged( const QString& udi,  bool connected );
        void slotKonnectorError( const QString& udi,
                                 int error,
                                 const QString &id );
    };
};
