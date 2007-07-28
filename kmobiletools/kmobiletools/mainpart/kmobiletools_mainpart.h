/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>
   by Matthias Lechner <matthias@lmme.de>

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

#include <KParts/Part>
#include <KParts/StatusBarExtension>
#include <KParts/MainWindow>

#include <QStringList>

#include "deviceslist.h"

class QStackedWidget;
class QString;
class K3ListView;
class Q3ListViewItem;
class DeviceManager;
class KAboutData;
class KSystemTrayIcon;
class ErrorLogDialog;
class DeviceHome;

namespace KMobileTools {
    class homepagePart;
}

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author Marco Gulino <marco.gulino@gmail.com>
 * @author Matthias Lechner <matthias@lmme.de>
 * @version 0.5.0
 */

class kmobiletoolsMainPart : public KParts::ReadOnlyPart/*, virtual public MainIFace*/
{
    Q_OBJECT
    public:
        /**
         * Constructs a new part (@see KParts::GenericFactory)
         */
        kmobiletoolsMainPart(QWidget *parentWidget, QObject *parent, const QStringList &args=QStringList() );

        /**
         * Destructs the part
         */
        virtual ~kmobiletoolsMainPart();

        /**
         * This method is needed for @see KParts::GenericFactory
         *
         * @return about data of our part
         */
        static KAboutData *createAboutData();

        /**
         * @see KParts::ReadOnlyPart
         */
        bool openFile() { return false; }

        K3ListView *listview() { return p_listview; }
        KSystemTrayIcon * sysTray() { return p_sysTray; }
        KParts::StatusBarExtension *statusBarExtension() { return p_statusBarExtension;}

//        DCOPClient * dcopClient() { return p_dcopClient; }

    public Q_SLOTS:
        void loadDevicePart(const QString &deviceName, bool setActive=false);
        void configSlot( const QString& command );
        void deleteDevicePart(const QString &deviceName);
        void deviceDisconnected();
        void deviceConnected();
        bool deviceIsLoaded(const QString &deviceName);
        //void newSMS(); /// @TODO remove
        void phonebookUpdated();
        void slotConfigNotify();

    private Q_SLOTS:
        void switchPart( const QString& partName );
        void goHome();
        void nextPart();
        void prevPart();

        void slotQuit();
        void checkShowDeviceList();
        void slotAutoLoadDevices();
        void addDevice(const QString &newDevice);
        void delDevice(const QString &newDevice);
        void updateStatus();

        void widgetStackItemChanged(int item);
        void listviewClicked(Q3ListViewItem *);

    private:
        void setupGUI( QWidget* parent );
        void setupActions();
        void setupDialogs();

        bool checkConfigVersion();

        QStackedWidget *m_widget;
        K3ListView *p_listview;
        KSystemTrayIcon * p_sysTray;

        KParts::MainWindow *mainwindow;
        KParts::StatusBarExtension *p_statusBarExtension;
        KMobileTools::homepagePart* p_homepage;
        DevicesList l_devicesList;

        ErrorLogDialog* m_errorLogDialog;
        DeviceManager* m_deviceManager;

    signals:
        void devicesUpdated();
        void deviceChanged(const QString &);
protected:
//    DCOPClient * p_dcopClient;

};

#endif // _KMOBILETOOLSPART_H_
