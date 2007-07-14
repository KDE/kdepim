/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#ifndef _KMOBILETOOLSMAINPART_H_
#define _KMOBILETOOLSMAINPART_H_

#include <kparts/part.h>
#include <kparts/factory.h>
#include <kparts/statusbarextension.h>

#include <qstringlist.h>
#include <kdebug.h>

#include "errorlogdialog.h"

//#include "mainIFace.h"
#include "deviceslist.h"

class Q3WidgetStack;
class QString;
class K3ListView;
class K3ListViewItem;
class Q3ListViewItem;
class DeviceManager;
class KAboutData;
class KSystemTrayIcon;

namespace KMobileTools {
class homepagePart;
}
// class KParts::StatusBarExtension;
#include <kparts/partmanager.h>
#include <kparts/mainwindow.h>
/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
           * @author Marco Gulino <marco.gulino@gmail.com>
           * @version 0.5.0
 */
/*class DeviceHome : public KParts::Part
{
Q_OBJECT
public:
    DeviceHome(QWidget *parentWidget, const char *widgetName,QObject *parent, const char *name)
        : KParts::Part( parent, name ) {}
    virtual K3ListViewItem *listViewItem() { return p_listViewItem; }
private:
    K3ListViewItem *p_listViewItem;
public slots:
    virtual void clicked ( QListViewItem * item ) =0;
};
*/
class kmobiletoolsMainPart : public KParts::ReadOnlyPart/*, virtual public MainIFace*/
{
    Q_OBJECT
    public:
        kmobiletoolsMainPart(QWidget *parentWidget, QObject *parent, const QStringList &args=QStringList() );
        virtual ~kmobiletoolsMainPart();
        K3ListView *listview() { return p_listview; }
        bool openFile() { return false; }
        static KAboutData *createAboutData();
//        DCOPClient * dcopClient() { return p_dcopClient; }
        KSystemTrayIcon * sysTray() { return p_sysTray; }
//         KParts::PartManager *partmanager(){ return partManager;}
        const DevicesList devicesList() { return l_devicesList;}
        KParts::StatusBarExtension *statusBarExtension() { return p_statusBarExtension;}
        static kmobiletoolsMainPart *staticInstance() { return m_mainpart; }
        bool checkConfigVersion();

    public slots:
    void activePartChanged(KParts::Part *newPart);
    void loadDevicePart(const QString &deviceName, bool setActive=false);
    void updateStatus();
    void switchPart( const QString  &partName );
    void nextPart();
    void goHome();
    void prevPart();
    DeviceManager *deviceManager();
    void configSlot(const QString &command);
    void showErrorLog();
    void showPreference();
    void addDevice(const QString &newDevice);
    void delDevice(const QString &newDevice);
    void deleteDevicePart(const QString &deviceName);
    void listviewClicked(Q3ListViewItem *);
//     void partChanged(KParts::Part *newPart);
    void slotQuit();
    void deviceDisconnected();
    void deviceConnected();
    void widgetStackItemChanged(int item);
    void slotAutoLoadDevices();
    void devicesChanged ();
    bool deviceIsLoaded(const QString &deviceName);
    void slotHide();
    void newSMS();
    QStringList loadedEngines(bool friendly=false);
    void phonebookUpdated();
    void slotConfigNotify();

    private:
        Q3WidgetStack *m_widget;
//         KParts::PartManager *partManager;
        KMobileTools::homepagePart* p_homepage;
        K3ListView *p_listview;
        KParts::MainWindow *mainwindow;
        DevicesList l_devicesList;
        QStringList sl_toloadList;
        KParts::StatusBarExtension *p_statusBarExtension;
        static kmobiletoolsMainPart *m_mainpart;

        ErrorLogDialog* m_errorLogDialog;
    signals:
        void devicesUpdated();
        void deviceChanged(const QString &);
protected:
//    DCOPClient * p_dcopClient;
    KSystemTrayIcon * p_sysTray;
    void guiActivateEvent  ( KParts::GUIActivateEvent *event);

};

#endif // _KMOBILETOOLSPART_H_
