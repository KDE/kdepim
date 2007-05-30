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
#ifndef _KMOBILETOOLSDEVICEPART_H_
#define _KMOBILETOOLSDEVICEPART_H_

#include <kdebug.h>
#include <kabc/addressbook.h>
#include <kabc/addressee.h>
#include <qfile.h>
#include <k3listview.h>

#include <kcal/calendarlocal.h>

#include <k3listviewsearchline.h>
#include "kmobiletools_mainpart.h"
//#include "deviceIFace.h"
#include "ui_mainWidget.h"
#include "statusbar.h"

#include <libkmobiletools/engine.h>
#include <libkmobiletools/sms.h>
#include <libkmobiletools/homepage.h>
#include <libkmobiletools/enginedata.h> //@TODO remove me

class QWidget;
// class mainWidget;
class QString;
class K3ListView;
class Q3ListViewItem;
class StatusBar;
class KStatusBar;
class addressDetails;
class smsPart;
class QProgressBar;
class QLabel;
class kmobiletoolsMainPart;
class StatusBarProgressBox;
class KAction;
namespace KParts
{
    class StatusBarExtension;
    class ReadOnlyPart;
}
namespace KMobileTools
{
    class ContactsList;
}

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Device Part
 * @author Marco Gulino <marco.gulino@gmail.com>
 * @version 0.5.0
 */

class DeviceListViewItem : public K3ListViewItem
{
    public:
        explicit DeviceListViewItem (Q3ListView *parent, QString s1) : K3ListViewItem(parent,s1) {}
        QString deviceName() const { return devicename; }
        void setDeviceName(const QString &devname) { devicename=devname; }
    private:
        QString devicename;
};

class ContactListViewItem : public K3ListViewItem
{
    public:
        explicit ContactListViewItem(Q3ListView *parent, const KABC::Addressee &contact, bool readOnly=false);
        KABC::Addressee contact() { return p_contact; }
        bool readOnly() { return b_ro; }
    private:
        KABC::Addressee p_contact;
        bool b_ro;
};

class ContactsSearchLine : public K3ListViewSearchLine
{
    Q_OBJECT
    public:
        explicit ContactsSearchLine (QWidget *parent=0, K3ListView *listView=0)
    : K3ListViewSearchLine(parent, listView) {}
    protected:
        virtual bool itemMatches (const Q3ListViewItem *item, const QString &s) const;
};

class ContactsSearchLineWidget : public K3ListViewSearchLineWidget
{
    Q_OBJECT
    public:
        explicit ContactsSearchLineWidget(K3ListView *listView=0, QWidget *parent=0)
    : K3ListViewSearchLineWidget(listView, parent), p_searchline(NULL) {};
        K3ListViewSearchLine *   searchLine () const { return p_searchline; }
        virtual K3ListViewSearchLine * createSearchLine (K3ListView *listView);
    private:
        ContactsSearchLine *p_searchline;
};

class DeviceHome : public QObject/*, virtual public DeviceIFace*/
{
    Q_OBJECT
public:
    explicit DeviceHome(QWidget *parentWidget, const QString &devicename,kmobiletoolsMainPart *parent);
    virtual ~DeviceHome();
    virtual K3ListViewItem *listViewItem() { return p_listViewItem; }
    bool isConnected() { return devIsConnected; }
    void stopDevice();
    void resumeDevice();
    QString deviceVendor() { return engine->engineData()->manufacturerString(); } // @TODO remove me
    QString deviceModel() { return engine->engineData()->model(); } // @TODO remove me
    QString friendlyName();
    static QStringList parseAddressee(const KABC::Addressee &addressee);
    void raiseDevice();
    bool openFile() { return false; }
    QWidget *widget() { return m_widget;}
    QList<QAction*> actionList() { return l_actionList;}
    SMSList *smsList() { return engine->engineData()->smsList(); }
    void setupWidgets();
    kmobiletoolsMainPart *parent() { return (kmobiletoolsMainPart *) QObject::parent(); }
    KCal::CalendarLocal * calendar() { return p_calendar; }
private:
    Ui::mainWidget ui;
    QWidget *m_widget;
    ContactsSearchLineWidget *slwidget;
    StatusBar *m_statusbar;
    KMobileTools::Engine* engine;
    K3ListViewItem *p_smsItem;
    DeviceListViewItem *p_listViewItem;
    KMobileTools::homepagePart *home;
    QTimer *statusPollTimer;
    QTimer *smsPollTimer;
    addressDetails *p_addressDetails;
    smsPart *p_smsPart;
    bool devIsConnected;
    int suspends_count;
    int smsnotifynum;
//     QFile f_pidfile;
    KParts::StatusBarExtension *statusBarExtension;
    QList<QAction*> l_actionList;
    int memslotSelected, smsTypeSelected;
    KParts::ReadOnlyPart *korgpart;
    kmobiletoolsMainPart *p_mainPart;
    KCal::CalendarLocal *p_calendar;
    /// StatusBar items
    StatusBarProgressBox *statusBarBox;
    QProgressBar *signalProgress;
    QProgressBar *batteryProgress;
    QLabel *networkLabel;
    QLabel *devNameLabel;
    QLabel *voidLabel;
    QLabel *batteryLabel;
    QLabel *signalLabel;
    bool b_dialing, b_dialing_called;

    protected:
//         void partActivateEvent( KParts::PartActivateEvent *event );
//         void guiActivateEvent  ( KParts::GUIActivateEvent *event);
public slots:
    void errNotConnected();
    void addSMSFolders();
    void updatePB();
    void updatePB(int slot, const KMobileTools::ContactsList&);
    void updateAllContacts();
    void updateAllContacts(KMobileTools::ContactsList *addressBook);
    void updateSMSList();
    void clicked ( Q3ListViewItem * item );
    void printInfoPage(int i) { home->printInfoPage(i, engine); }
    void pb_clicked ( Q3ListViewItem * item );
    void devDisconnected();
    void devConnected();
    void disableWidgets();
    void enableWidgets(bool fetch=true);
    void smsSelected(Q3ListViewItem *smsItem);
    void setupStatusBar();
    void clearStatusBar();
    void sendSMS(const QString&, const QString& );
    void storeSMS(const QString&, const QString& );
    void loadEngine();
    void jobDone(KMobileTools::Job::JobType);

    /*!
        \fn DeviceHome::encodings()
     */
    QStringList encodings()
    {
        return engine->encodings();
    }
    void slotAddContact();
    void slotEditContact(const KABC::Addressee&);
    void slotDeleteContact();
    void slotFetchPhonebook();
    void slotSavePhonebook();
    void slotUploadAddressBook();
    void addresseeListRightClick(Q3ListViewItem *item, const QPoint &point, int column);
    void smsListRightClick(Q3ListViewItem *item, const QPoint &point, int column);
    void smsModified(const QByteArray& smsUID);
    void smsAdded(const QByteArray& smsUID);
    void smsRemoved(const QByteArray& smsUID);
    void smsFolderClicked( Q3ListViewItem * item );
    void slotNewSMS() { slotNewSMS(QString() ); }
    void slotNewSMS(const QString &);
    void openURL(const KUrl &url);
    void updateSMSCount();
    void fullPhonebook();
    void slotFetchCalendar();
    void slotCalendarFetched();
    void slotExportSMSList();
    void slotExportSMSListToCSV();
    void slotStatusBar();
    void slotSaveAddressBook();
    void slotSendStoredSMS(SMS*);
    void slotRing(bool);
    void slotDial();
    void slotDialNumber(const QString &);
    void raisePage(int);
    QString currentDeviceName() const;
    void switch2filesystem();
    void contactsTabChanged();

signals:
        void disconnected();
        void connected();
        void error();
        void phonebookUpdated();
        void command(const QString &);
        void deleteThis(const QString &);
};

class KInstance;
class KAboutData;

#endif // _KMOBILETOOLSPART_H_
