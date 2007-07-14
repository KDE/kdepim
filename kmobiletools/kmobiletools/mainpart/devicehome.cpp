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
#include "devicehome.h"

#include <kcomponentdata.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kabc/stdaddressbook.h>
#include <kabc/vcardformat.h>
#include <kapplication.h>
#include <qtabwidget.h>

#include <qfile.h>
#include <q3multilineedit.h>
#include <qlabel.h>
#include <Q3PtrList>
#include <kpushbutton.h>
#include <qprogressbar.h>
#include <k3listview.h>
#include <qsplitter.h>
#include <kparts/partmanager.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <q3widgetstack.h>
#include <khtmlview.h>
#include <qtimer.h>
#include <k3activelabel.h>
#include <kabc/phonenumber.h>
#include <qlayout.h>
#include <kglobalsettings.h>
#include <knuminput.h>
#include <klineedit.h>
#include <q3listview.h>
#include <kdeversion.h>
#include <kstandarddirs.h>
//#include <dcopclient.h>
#include <kparts/statusbarextension.h>
#include <kstatusbar.h>
#include <kwindowsystem.h>
#include <kparts/factory.h>
#include <kparts/part.h>
#include <kresources/manager.h>
//#include <knotifyclient.h>
#include <kcombobox.h>
#include <kplugininfo.h>
#include <knotification.h>
#include <ksystemtrayicon.h>

#ifdef HAVE_KCAL
#include <libkcal/resourcelocal.h>
#endif

#include <libkmobiletools/kmobiletools_cfg.h>
#include <libkmobiletools/devicesconfig.h>

#include <libkmobiletools/aboutdata.h>
#include <libkmobiletools/engine.h>
#include <popupaddressee.h>
#include <popupsms.h>
#include <libkmobiletools/contactslist.h>
#include <libkmobiletools/engineslist.h>
#include <libkmobiletools/smslist.h>
#include <libkmobiletools/weaver.h>

#include "addressdetails.h"
#include "editaddressee.h"
#include "smspart.h"
#include "importphonebookdlg.h"
#include "exportphonebookdlg.h"

#include "newsmsdlg.h"
#include "smslistviewitem.h"
#include "calldialogimpl.h"
//#include "mainIFace_stub.h"


#include <libkmobiletools/errorhandler.h>
#include <libkmobiletools/errortypes/baseerror.h>

#define INDEX_WIDGET_ID 0
#define PHONEBOOK_WIDGET_ID 2
#define SMS_WIDGET_ID 1
#define CAL_WIDGET_ID 3

// #define ABCLIST_ID_COL 2

#define PIDFILE KGlobal::dirs()->resourceDirs("tmp").first() + ".kmobiletools-" + name + ".pid"

using namespace KMobileTools;

bool ContactsSearchLine::itemMatches (const Q3ListViewItem *item, const QString &s) const
{
    if(K3ListViewSearchLine::itemMatches(item, s)) return true;
    ContactListViewItem *p_item=(ContactListViewItem*) item;
    // Search telephone numbers
    KABC::PhoneNumber::List list=p_item->contact().phoneNumbers();
    for(KABC::PhoneNumber::List::ConstIterator it=list.begin(); it!=list.end(); ++it)
        if( (*it).number().indexOf(s, 0, caseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive ) >=0) return true;
    return false;
}

K3ListViewSearchLine * ContactsSearchLineWidget::createSearchLine (K3ListView *listView)
{
    if ( searchLine()) return searchLine();
    p_searchline= new ContactsSearchLine(this, listView);
    return p_searchline;
}

ContactListViewItem::ContactListViewItem(Q3ListView *parent, const KABC::Addressee &contact, bool readOnly)
    : K3ListViewItem(parent)
{
    p_contact=contact;
    b_ro=readOnly;
    setText(0, p_contact.formattedName());
}



DeviceHome::DeviceHome( QWidget *parentWidget, const QString &devicename, kmobiletoolsMainPart *parent ) : QObject( parent ), p_mainPart(parent), statusBarExtension(0), memslotSelected(0), signalProgress(0),
    smsTypeSelected(0), networkLabel(0), batteryProgress(0), devNameLabel(0), voidLabel(0),
    batteryLabel(0), signalLabel(0), statusBarBox(0), suspends_count(0), engine(0), p_calendar(0),
    b_dialing(false), b_dialing_called(false)
{
    m_widget = new QWidget(parentWidget);
    ui.setupUi(m_widget);

    setObjectName(devicename);
    setupWidgets();
    home->printInfoPage( 2,  DEVCFG( devicename )->devicename(), 0);
    QTimer::singleShot( 1000, this, SLOT(loadEngine() ) );
    disableWidgets();

    connect(ui.contactsTab, SIGNAL(currentChanged ( QWidget * )), this, SLOT(contactsTabChanged()));
    connect(KMobileTools::EnginesList::instance(), SIGNAL(phonebookUpdated()), this, SLOT(updateSMSList() ) );
    // Setup actions
    QAction* act=parent->actionCollection()->addAction("sms_new", this, SLOT(slotNewSMS()));
    act->setText(i18n("New SMS"));
    act->setIcon(KIcon("mail_generic"));
    l_actionList.append(act);
    act=parent->actionCollection()->addAction("exportsms", this, SLOT(slotExportSMSList()));
    act->setText(i18n("Export SMS List"));
    act->setIcon(KIcon("exportsms"));
    l_actionList.append(act);
    act=parent->actionCollection()->addAction("exportcsv", this, SLOT(slotExportSMSListToCSV()));
    act->setText(i18n("Export SMS List to CSV"));
    act->setIcon(KIcon("mail_get"));
    l_actionList.append(act);
    QTimer::singleShot( 1000, this, SLOT(slotStatusBar() ) );
    updateAllContacts();
}

#include "devicehome.moc"

void DeviceHome::setupWidgets()
{
    QVBoxLayout *layout, *layout2;
//#if KDE_IS_VERSION( 3, 4, 0 )
    slwidget=new ContactsSearchLineWidget( ui.phonebookListView, ui.abc_sl_frame);
    K3ListViewSearchLineWidget *sms_slwidget=new K3ListViewSearchLineWidget( ui.SMSListView, ui.sms_lv_src);
/*#else
    ContactsSearchLine *slwidget=new ContactsSearchLine( ui.abc_sl_frame, ui.phonebookListView);
    ContactsSearchLine *sms_slwidget=new ContactsSearchLine( ui.sms_lv_src, ui.SMSListView);
#endif*/
//     ui.phonebookListView->setColumnWidth(0,999);
    ui.phonebookListView->setShadeSortColumn(false);
    ui.phonebookListViewFull->setShadeSortColumn(false);
//     ui.phonebookListView->resize( 60, ui.phonebookListView->height() );
    layout=new QVBoxLayout(ui.abc_sl_frame);
    layout->addWidget(slwidget);
    layout->setSpacing(2);
    layout->setMargin(2);
    layout2=new QVBoxLayout(ui.sms_lv_src);
    layout2->addWidget(sms_slwidget);
    layout2->setSpacing(2);
    layout2->setMargin(2);
    p_addressDetails=new addressDetails(ui.detailsFrame, objectName() , this );
    p_smsPart=new smsPart(ui.smsFrame, "smsPart", this, DEVCFG(objectName() )->devicename() );
    layout=new QVBoxLayout(ui.detailsFrame);
    layout->addWidget(p_addressDetails->view() );
    layout->setSpacing(2);
    layout->setMargin(2);
    layout=new QVBoxLayout(ui.smsFrame);
    layout->addWidget(p_smsPart->view() );
    layout->setSpacing(2);
    layout->setMargin(2);
    home=new KMobileTools::homepagePart(m_widget, "homepage");
    ui.widgetStack->removeWidget(ui.widgetStack->widget(0) );
    ui.widgetStack->addWidget(home->view(), 0);
    ui.widgetStack->raiseWidget(INDEX_WIDGET_ID);
    m_widget->setFocusPolicy(Qt::ClickFocus);

    p_listViewItem=new DeviceListViewItem(p_mainPart->listview() , DEVCFG(objectName() )->devicename());
    p_listViewItem->setDeviceName( objectName() );
    p_listViewItem->setOpen(true);
    p_listViewItem->setPixmap(0,DEVCFG(objectName() )->deviceTypeIcon(DEVCFG(objectName() )->currentGroup(), K3Icon::NoGroup, K3Icon::SizeSmall ));
//     (new K3ListViewItem(p_listViewItem, "Calls") )
//             ->setPixmap(0,KIconLoader::global()->loadIcon("kaddressbook", K3Icon::NoGroup, K3Icon::SizeSmall) );
    (new K3ListViewItem(p_listViewItem, i18n("PhoneBook") ) )
            ->setPixmap(0,KIconLoader::global()->loadIcon("kontact_contacts", K3Icon::NoGroup, K3Icon::SizeSmall) );
    p_smsItem = new K3ListViewItem(p_listViewItem, i18n("SMS") );
    p_smsItem->setPixmap(0,KIconLoader::global()->loadIcon("mail_generic", K3Icon::NoGroup, K3Icon::SizeSmall) );

    // SMS Tree ListView
    SMSFolderListViewItem *sms_inbox= new SMSFolderListViewItem(ui.SMSFolderView, SMS::SIM | SMS::Phone, SMS::Unread | SMS::Read, i18n("Inbox") );
    SMSFolderListViewItem *sms_inbox_phone= new SMSFolderListViewItem(sms_inbox, SMS::Phone, SMS::Unread | SMS::Read, i18n("Phone") );
    SMSFolderListViewItem *sms_inbox_sim= new SMSFolderListViewItem(sms_inbox, SMS::SIM, SMS::Unread | SMS::Read, i18n("SIM") );
    SMSFolderListViewItem *sms_outbox= new SMSFolderListViewItem(ui.SMSFolderView, SMS::SIM | SMS::Phone, SMS::Sent | SMS::Unsent, i18n("Outgoing") );
    SMSFolderListViewItem *sms_outbox_phone= new SMSFolderListViewItem(sms_outbox, SMS::Phone, SMS::Sent | SMS::Unsent, i18n("Phone") );
    SMSFolderListViewItem *sms_outbox_sim= new SMSFolderListViewItem(sms_outbox, SMS::SIM, SMS::Sent | SMS::Unsent, i18n("SIM") );
    sms_inbox->setPixmap(0,KIconLoader::global()->loadIcon("mail_get", K3Icon::NoGroup, K3Icon::SizeSmall) );
    sms_outbox->setPixmap(0,KIconLoader::global()->loadIcon("mail_send", K3Icon::NoGroup, K3Icon::SizeSmall) );
    sms_inbox_phone->setPixmap(0,KIconLoader::global()->loadIcon("kmobiletools", K3Icon::NoGroup, K3Icon::SizeSmall) );
    sms_outbox_phone->setPixmap(0,KIconLoader::global()->loadIcon("kmobiletools", K3Icon::NoGroup, K3Icon::SizeSmall) );
    sms_inbox_sim->setPixmap(0,KIconLoader::global()->loadIcon("simcard", K3Icon::NoGroup, K3Icon::SizeSmall) );
    sms_outbox_sim->setPixmap(0,KIconLoader::global()->loadIcon("simcard", K3Icon::NoGroup, K3Icon::SizeSmall) );

    sms_inbox->setOpen(true);
    sms_outbox->setOpen(true);

    ui.SMSListView->setSorting(2, false);
    connect(home, SIGNAL(infopage(int)), SLOT(printInfoPage(int) ) );
    connect(ui.phonebookListView, SIGNAL(clicked ( Q3ListViewItem *)), SLOT(pb_clicked( Q3ListViewItem *)) );
    connect(ui.phonebookListViewFull, SIGNAL(clicked ( Q3ListViewItem *)), SLOT(pb_clicked( Q3ListViewItem *)) );
    connect(ui.phonebookListView, SIGNAL(currentChanged ( Q3ListViewItem *)), SLOT(pb_clicked( Q3ListViewItem *)) );
    connect(ui.phonebookListViewFull, SIGNAL(currentChanged ( Q3ListViewItem *)), SLOT(pb_clicked( Q3ListViewItem *)) );
    connect(p_smsPart, SIGNAL(writeNew() ), this, SLOT(slotNewSMS() ) );
    connect(p_smsPart, SIGNAL(exportList()), this, SLOT(slotExportSMSList() ) );
    connect(p_smsPart, SIGNAL(exportList()), this, SLOT(slotExportSMSListToCSV()() ) );
    connect(p_smsPart, SIGNAL(reply(const QString &)), this, SLOT(slotNewSMS( const QString& ) ) );
    connect(p_smsPart, SIGNAL(send( SMS* ) ), this, SLOT(slotSendStoredSMS( SMS* ) ) );
    connect(ui.SMSListView, SIGNAL(clicked ( Q3ListViewItem * ) ), SLOT(smsSelected(Q3ListViewItem *) ) );
    connect(ui.SMSListView, SIGNAL(currentChanged ( Q3ListViewItem * )), SLOT(smsSelected(Q3ListViewItem *) ) );
    connect(p_addressDetails, SIGNAL(exportPB() ), this, SLOT(slotSavePhonebook() ) ); // Fetching addressbook
    connect(p_addressDetails, SIGNAL(importPB() ), this, SLOT(slotUploadAddressBook() ) ); // Uploading addressbook
    connect(p_addressDetails, SIGNAL(addContact() ), this, SLOT(slotAddContact() ) );
    connect(p_addressDetails, SIGNAL(delContact() ), this, SLOT(slotDeleteContact()) );
    connect(p_addressDetails, SIGNAL(refreshClicked() ), this, SLOT(slotFetchPhonebook() ) );
    connect(p_addressDetails, SIGNAL(dial(const QString &)), this, SLOT(slotDialNumber(const QString &) ) );
    connect(p_addressDetails,SIGNAL(editClicked(KABC::Addressee)), this, SLOT(slotEditContact(KABC::Addressee))) ;
    connect(ui.phonebookListView, SIGNAL(rightButtonPressed(Q3ListViewItem*, const QPoint&, int ) ), 
            this, SLOT(addresseeListRightClick(Q3ListViewItem*, const QPoint&, int ) ) );
    connect(ui.phonebookListViewFull, SIGNAL(rightButtonPressed(Q3ListViewItem*, const QPoint&, int ) ), 
            this, SLOT(addresseeListRightClick(Q3ListViewItem*, const QPoint&, int ) ) );
    connect(ui.SMSListView, SIGNAL(rightButtonPressed(Q3ListViewItem*, const QPoint&, int ) ), 
            this, SLOT(smsListRightClick(Q3ListViewItem*, const QPoint&, int ) ) );
    connect(ui.SMSFolderView, SIGNAL(clicked( Q3ListViewItem*)), this, SLOT(smsFolderClicked( Q3ListViewItem*)));
    connect(home, SIGNAL(deviceCMD( const KUrl& )), this, SLOT(openURL( const KUrl& )) );
    connect(ui.b_dial, SIGNAL(clicked()), this, SLOT(slotDial()));
#ifdef HAVE_KCAL
    if(DEVCFG(objectName() )->calendar() )
    {
        (new K3ListViewItem(p_listViewItem, i18n("Calendar") ) )
                ->setPixmap(0,KIconLoader::global()->loadIcon("date", K3Icon::NoGroup, K3Icon::SizeSmall) );
        KParts::Factory *pfactory=(KParts::Factory *) (KLibLoader::self()->factory("libkorganizerpart") );
        QVBoxLayout *l1=new QVBoxLayout(ui.korg_frame);
        l1->setMargin(0);
        if(pfactory)
        {
            korgpart=static_cast<KParts::ReadOnlyPart*>(pfactory->createPart( ui.korg_frame, "KOrganizerPart" , ui.korg_frame, "KOrganizerPart", "KParts::ReadOnlyPart"));
            if(korgpart)
            {
                l1->addWidget( korgpart->widget() );
                p_calendar=new KCal::CalendarLocal("");
                if(QFile::exists( KGlobal::dirs()->saveLocation( "data", "kmobiletools", true).append( "%1.vcs" ).arg(objectName() ) ) )
                    p_calendar->load(KGlobal::dirs()->saveLocation( "data", "kmobiletools", true).append( "%1.vcs" ).arg(objectName() ) );
                else p_calendar->save(KGlobal::dirs()->saveLocation( "data", "kmobiletools", true).append( "%1.vcs" ).arg(objectName() ) );
                korgpart->openUrl( KGlobal::dirs()->saveLocation( "data", "kmobiletools", true).append( "%1.vcs" ).arg(objectName() ) );
                KAction *act=new KAction(KIcon("date"), i18n("Fetch Calendar"), this);
                connect(act, SIGNAL(triggered()), this, SLOT(slotFetchCalendar()));
                parent()->actionCollection()->addAction("get_cal", act);
                l_actionList.append(act);
            }
        }
    }
#endif
}

void DeviceHome::loadEngine()
{
    QString libName;
    devIsConnected=false;
    KPluginInfo *infos=KMobileTools::EnginesList::instance()->engineInfo( DEVCFG(objectName() )->engine() );
    if(!infos)
    {
        engine=0;
        KMessageBox::error(m_widget, i18n("The selected engine could not be found. Please reinstall KMobileTools.") );
        emit deleteThis( objectName() );
        return;
    }
    libName=infos->service()->library();
    kDebug() << "**************** libname printable: " << qPrintable(libName) << endl;
    engine=KMobileTools::Engine::load(libName, this);
    if(!engine)
    {
        engine=0;
        kDebug() << "Library error message: " << KLibLoader::self()->lastErrorMessage() << endl;
        KMessageBox::error(m_widget, i18n("Could not load the device %1.\nIf this error persists, please restart KMobileTools.", objectName() ) );
        emit deleteThis( objectName() );
        return;
    }
    connect(engine->constEngineData(), SIGNAL(connected()), this, SLOT(devConnected()) );
    connect(engine->constEngineData(), SIGNAL(disconnected()), this, SLOT(devDisconnected() ) );
    connect(engine->constEngineData(), SIGNAL(connected()), this, SLOT(enableWidgets() ) );
    connect(engine->constEngineData(), SIGNAL(disconnected() ), this, SLOT(disableWidgets() ) );
    connect(engine, SIGNAL(phoneBookChanged() ), SLOT(updatePB()) );
    //connect(engine, SIGNAL(phoneBookChanged(int, const ContactsList& ) ), SLOT(updatePB(int, const ContactsList& ) ) );
    connect(engine, SIGNAL(smsFoldersAdded() ), SLOT(addSMSFolders()) );
    connect(engine->constEngineData(), SIGNAL(smsAdded( const QString & )), SLOT(smsAdded( const QString &) ) );
    connect(engine->constEngineData(), SIGNAL(smsDeleted( const QString & )), SLOT(smsRemoved(const QString &) ) );
    connect(engine->constEngineData(), SIGNAL(smsModified( const QString & )), SLOT(smsModified( const QString & )) );
    connect(engine->constEngineData(), SIGNAL(ringing( bool )), this, SLOT(slotRing( bool ) ) );
    connect(engine, SIGNAL(fullPhonebook()), this, SLOT(fullPhonebook()) );
    connect(p_smsPart, SIGNAL(getSMSList() ), engine, SLOT( slotFetchSMS() ) );
    connect(p_smsPart, SIGNAL(remove( SMS* ) ), engine, SLOT(slotDelSMS( SMS* ) ) );
    connect(engine->constEngineData()->smsList(), SIGNAL(updated()), this, SLOT(updateSMSList() ) );
    connect(engine, SIGNAL(jobFinished(KMobileTools::Job::JobType)), this, SLOT(jobDone(KMobileTools::Job::JobType)));
#ifdef HAVE_KCAL
    connect(engine, SIGNAL(calendarParsed() ), this, SLOT(slotCalendarFetched() ) );
#endif
    home->printInfoPage( 2, engine );
    engine->slotSearchPhone();
    if( DEVCFG(objectName() )->status_poll() && DEVCFG(objectName() )->status_pollTimeout() > 0 )
    {
        statusPollTimer=new QTimer(this);
        statusPollTimer->setSingleShot(false);
        connect(statusPollTimer, SIGNAL(timeout() ), engine, SLOT(slotPollStatus()) );
        statusPollTimer->start( (int) DEVCFG(objectName() )->status_pollTimeout() * 1000 );
    }
    if( DEVCFG(objectName() )->smsPoll() && DEVCFG(objectName() )->sms_pollTimeout() > 0 )
    {
        smsPollTimer=new QTimer(this);
        smsPollTimer->setSingleShot(false);
        connect(smsPollTimer, SIGNAL(timeout() ), engine, SLOT(slotFetchSMS()) );
        uint smsTimeout=(int) DEVCFG(objectName() )->sms_pollTimeout() * 1000;
        smsPollTimer->start( smsTimeout );
        if(smsTimeout > 30*1000)
            // if we've a long timer for sms just retrieve them earlier the first time.
            QTimer::singleShot(15*1000, engine, SLOT(slotFetchSMS()));
    }
    updateSMSCount();

    ErrorHandler::instance()->addError( new BaseError(ERROR_META_INFO) );
}


QString DeviceHome::friendlyName()
{
    return DEVCFG(objectName())->devicename();
}

DeviceHome::~DeviceHome()
{
    kDebug() << "DeviceHome::~DeviceHome()\n";
    KMobileTools::EnginesList::instance()->unlock(objectName() );
//     f_pidfile.remove();
//     delete m_widget;
//     delete m_statusbar;
}

void DeviceHome::setupStatusBar()
{
    if(!devNameLabel || !networkLabel || !voidLabel || !batteryLabel ||
        !batteryProgress || !signalLabel || !signalProgress || !statusBarBox )
    {
        QTimer::singleShot( 70, this, SLOT(setupStatusBar() ) );
        return;
    }
    statusBarExtension->addStatusBarItem( devNameLabel, 0, true );
    statusBarExtension->addStatusBarItem( networkLabel, 0, true );
    statusBarExtension->addStatusBarItem( voidLabel, 1, true );

    statusBarExtension->addStatusBarItem( batteryLabel, 0, true );

    statusBarExtension->addStatusBarItem( batteryProgress, 1, true);
    statusBarExtension->addStatusBarItem( signalLabel, 0, true );

    statusBarExtension->addStatusBarItem( signalProgress, 1, true);
    statusBarExtension->addStatusBarItem( statusBarBox, 0, true );
}

void DeviceHome::clearStatusBar()
{
    if(!statusBarExtension) return;
    if(devNameLabel) statusBarExtension->removeStatusBarItem(devNameLabel);
    if(networkLabel) statusBarExtension->removeStatusBarItem(networkLabel);
    if(voidLabel) statusBarExtension->removeStatusBarItem(voidLabel);
    if(batteryLabel) statusBarExtension->removeStatusBarItem(batteryLabel);
    if(batteryProgress) statusBarExtension->removeStatusBarItem(batteryProgress);
    if(signalLabel) statusBarExtension->removeStatusBarItem(signalLabel);
    if(signalProgress) statusBarExtension->removeStatusBarItem(signalProgress);
    if(statusBarBox) statusBarExtension->removeStatusBarItem(statusBarBox);
}

void DeviceHome::addSMSFolders()
{
    QStringList list( engine->smsFolders() );

    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) 
    {
        int fn = 1;
        K3ListViewItem* i = new K3ListViewItem(p_smsItem, *it );
        i->setPixmap(0,KIconLoader::global()->loadIcon("mail_generic", K3Icon::NoGroup, K3Icon::SizeSmall) );
        i->setText( 1, "SMSFolder" );
        i->setText( 2, QString::number( fn ) );
     }

}

/*!
    \fn DeviceHome::clicked ( QListViewItem * item )
 */
void DeviceHome::clicked ( Q3ListViewItem * item )
{
    if (! item) return;
    if ( item->text(0) == i18n("PhoneBook") )
    {
        ui.widgetStack->raiseWidget(PHONEBOOK_WIDGET_ID);
        return;
    }
    if ( item->text(0) == i18n("SMS") || item->text(1) == "SMSFolder" )
    {
        ui.widgetStack->raiseWidget(SMS_WIDGET_ID);
        return;
    }
    if (item->text(0) == i18n("Calendar") )
    {
        ui.widgetStack->raiseWidget(CAL_WIDGET_ID);
        return;
    }
    ui.widgetStack->raiseWidget(INDEX_WIDGET_ID);
}

void DeviceHome::updateAllContacts(KMobileTools::ContactsList *addressBook)
{
    KABC::Addressee::List::ConstIterator it_addressee=addressBook->begin();
    for(; it_addressee != addressBook->end(); ++it_addressee)
    {
        if(!(*it_addressee).phoneNumbers().count()) continue;
        new ContactListViewItem(ui.phonebookListViewFull, *it_addressee, true);
    }
}
void DeviceHome::updateAllContacts()
{
    ui.phonebookListViewFull->clear();
    ContactsList *abc=new ContactsList(KABC::StdAddressBook::self()->allAddressees() );
    if(!abc) return;
    updateAllContacts(abc);
    KMobileTools::Engine *p_engine;
    QList<KMobileTools::Engine*>::ConstIterator it=KMobileTools::EnginesList::instance()->begin(), itEnd=KMobileTools::EnginesList::instance()->end();
    for( ; it!=itEnd; ++it)
    {
        p_engine = *it;
        if( engine && QString(p_engine->objectName())==QString(engine->objectName())) continue;
        kDebug() << "DevicePart " << objectName() << ": adding contacts from engine " << p_engine->objectName() << endl;
        updateAllContacts( p_engine->constEngineData()->contactsList() );
    }
}

void DeviceHome::jobDone(KMobileTools::Job::JobType jobtype)
{
    if(jobtype==KMobileTools::Job::fetchAddressBook) emit phonebookUpdated();
    int newsmscnt=engine->constEngineData()->smsList()->count(SMS::Unread, SMS::SIM | SMS::Phone );

    if(newsmscnt && engine->ThreadWeaver()->isEmpty() && engine->ThreadWeaver()->isIdle() && newsmscnt!=smsnotifynum) {
        smsnotifynum=newsmscnt;
        QString eventString(i18n("<qt>%1 New Messages.<br>Mobile Phone: %2</qt>", newsmscnt, DEVCFG(name() )->devicename() ) );
        KNotification::event( QString("kmobiletools_sms"), eventString, QPixmap(),
            KMobileTools::KMobiletoolsHelper::instance()->systray()->contextMenu() );
    }
}

void DeviceHome::updatePB()
{
//     updating without partial updates should ALWAYS clear the listview...
    ui.phonebookListView->clear();
//     p_addressbook->clear();
    ContactsList *phoneBook =engine->constEngineData()->contactsList();
    //    if (!(phoneBook->count() ) ) return;
//     p_addressDetails->stopFetch();
//     ui.phonebookListView->clear();
    for (KABC::Addressee::List::ConstIterator it=phoneBook->begin(); it != phoneBook->end(); ++it)
    {
        new ContactListViewItem(ui.phonebookListView, *it);
//         p_addressbook->insertAddressee( *phoneBook->current() );
    }
    home->printInfoPage( home->currentInfoPage(), engine );
    enableWidgets(false);
    slotSaveAddressBook();
    emit phonebookUpdated();
}

/*
void DeviceHome::updatePB(int , const KMobileTools::ContactsList &p_phoneBook)
{
    //    if (!(phoneBook->count() ) ) return;
//     p_addressDetails->stopFetch();
//     ui.phonebookListView->clear();
    for (KABC::Addressee::List::ConstIterator it=p_phoneBook.begin(); it != p_phoneBook.end(); ++it)
    {
        new ContactListViewItem(ui.phonebookListView, *it);
//         p_addressbook->insertAddressee( *phoneBook->current() );
    }
    home->printInfoPage( home->currentInfoPage(), engine );
    enableWidgets(false);
//     slotSaveAddressBook();
//     emit phonebookUpdated();
}*/

QStringList DeviceHome::parseAddressee(const KABC::Addressee &addressee)
{
    QStringList retval;
    KABC::PhoneNumber::List::ConstIterator it;
    KABC::PhoneNumber::List p_list=addressee.phoneNumbers();
    for( it=p_list.begin(); it!=p_list.end() ;++it )
        retval+=(*it).number();
    retval+=addressee.emails();

    return retval;
}

/*!
    \fn DeviceHome::pb_clicked ( QListViewItem * item )
 */
void DeviceHome::pb_clicked ( Q3ListViewItem * item )
{
    if(!item)return;
    ContactListViewItem* c_item=((ContactListViewItem*) item);
    p_addressDetails->showAddressee( c_item->contact(), c_item->readOnly() );
}

void DeviceHome::devDisconnected()
{
    devIsConnected=false;
    emit disconnected();
    home->printInfoPage(0, this->engine);
}

void DeviceHome::devConnected()
{
//     kDebug() << "DeviceHome::devConnected()" << endl;
    devIsConnected=true;
    emit connected();
    DEVCFG(objectName() )->setLastpath(engine->currentDeviceName());
    home->printInfoPage(0, this->engine);
}

/*!
    \fn DeviceHome::enableWidgets()
 */
void DeviceHome::enableWidgets( bool fetch)
{
    if (!fetch) return;
    // It seems that having a retrieveAddressBook too early it's confusing.. so let's start it after some secs
    QTimer::singleShot( 5000, this, SLOT(slotFetchPhonebook() ) );
}


/*!
    \fn DeviceHome::disableWidgets()
 */
void DeviceHome::disableWidgets()
{
//     ui.pb_PutPhoneBook->setEnabled(false);
//     ui.pb_getPhoneBook->setEnabled(false);
//     ui.pb_add->setEnabled(false);
//     ui.pb_del->setEnabled(false);
}


/*!
    \fn DeviceHome::smsSelected(QListViewItem *smsItem)
 */
void DeviceHome::smsSelected(Q3ListViewItem *smsItem)
{
        if (! smsItem) return;
        SMSListViewItem *smsLVItem=(SMSListViewItem*) smsItem;
        p_smsPart->show( smsLVItem->sms() );
        smsLVItem->selected();
}


/*!
    \fn DeviceHome::slotAddContact()
 */
void DeviceHome::slotAddContact()
{
    editAddressee *new_contact=new editAddressee( engine->constEngineData()->manufacturerID(), engine->availPbSlots() );
    if (new_contact->exec() == QDialog::Accepted )
    {
        KABC::Addressee::List abclist;
        KABC::Addressee addressee=new_contact->getAddressee();
        abclist.append(addressee);
        engine->slotAddAddressee( abclist );
    }
    delete new_contact;
}

void DeviceHome::slotEditContact(const KABC::Addressee &p_addressee)
{
    editAddressee *new_contact=new editAddressee(p_addressee, engine->constEngineData()->manufacturerID(), engine->availPbSlots());
    if (new_contact->exec() == QDialog::Accepted )
        engine->slotEditAddressee(p_addressee, new_contact->getAddressee());
    delete new_contact;
}


/*!
    \fn DeviceHome::slotDeleteContact()
 */
void DeviceHome::slotDeleteContact()
{
    Q3PtrList<Q3ListViewItem> lst;
    QStringList(sl_toDelete);
    Q3ListViewItemIterator l_it( ui.phonebookListView, Q3ListViewItemIterator::Selected );
    if( ! l_it.current()  ) return;
    sl_toDelete+= l_it.current()->text(0); // Adding the first item to delete.
    bool multiple_selected= (bool) *( ++l_it );
    while ( l_it.current() )
    {
        sl_toDelete+= l_it.current()->text(0);
        ++l_it;
    }

    int retval;
    if( multiple_selected )
        retval=KMessageBox::warningYesNoList(NULL,
                                             i18n("<qt>This will permanently delete the following contacts.\nContinue?</qt>"),
                                             sl_toDelete,
                                             "KMobileTools");
    else
        retval=KMessageBox::warningYesNo(NULL,
                                         i18n("<qt>This will permanently delete the contact <b>%1</b> from the phone addressbook.\nContinue?</qt>",
                                                 sl_toDelete.first() ),
                                         "KMobileTools" );

    if(retval!=KMessageBox::Yes) return;
    KABC::Addressee::List toDelete;

    l_it=Q3ListViewItemIterator ( ui.phonebookListView, Q3ListViewItemIterator::Selected );

    while ( l_it.current() ) {
        toDelete.append(((ContactListViewItem*)(*l_it))->contact());
        ++l_it;
    }
    engine->slotDelAddressee( toDelete);
}

void DeviceHome::slotFetchPhonebook()
{
//     p_addressbook->clear();
    disableWidgets();
    ui.phonebookListView->clear();
    engine->slotFetchPhonebook();
    p_addressDetails->showHP();
//     p_addressDetails->startFetch();
}

void DeviceHome::slotSavePhonebook()
{
    exportPhonebookDlg *exportDlg=new exportPhonebookDlg(engine->constEngineData()->contactsList() );
    exportDlg->exec();
}



/*!
    \fn DeviceHome::slotUploadAddressBook()
 */
void DeviceHome::slotUploadAddressBook()
{
    importPhonebookDlg *abcDialog=new importPhonebookDlg( engine->availPbSlots() );
    if (abcDialog->exec() != QDialog::Accepted ) return;
    if(abcDialog->deletePhoneBook() )
    {
        KABC::Addressee::List toDeleteList;
        ContactsList *phoneBook =engine->constEngineData()->contactsList();
        for (KABC::Addressee::List::ConstIterator it=phoneBook->begin(); it != phoneBook->end(); ++it)
            toDeleteList.append(*it);
        engine->slotDelAddressee( toDeleteList);
    }
    engine->slotAddAddressee( abcDialog->addresseeList() );
}

void DeviceHome::stopDevice()
{
    if( ! suspends_count ) engine->slotStopDevice();
    suspends_count++;
}
void DeviceHome::resumeDevice()
{
    suspends_count--;
    if ( ! suspends_count ) engine->slotResumeDevice();
}



/*!
    \fn DeviceHome::addresseeListRightClick(QListViewItem *item, const QPoint &point, int column)
 */
void DeviceHome::addresseeListRightClick(Q3ListViewItem *item, const QPoint &point, int )
{
    if(!item) return;
    ContactListViewItem *c_item=(ContactListViewItem*)item;
    popupAddressee *popup=new popupAddressee( objectName(), c_item->contact(), ui.phonebookListView, c_item->readOnly() );
    connect(popup, SIGNAL(editClicked(KABC::Addressee) ), this, SLOT(slotEditContact(KABC::Addressee) ) );
    connect(popup, SIGNAL(delContact() ), this, SLOT(slotDeleteContact() ) );
    popup->exec(point);
}

void DeviceHome::errNotConnected()
{
    KMessageBox::error( m_widget, i18n("Please connect your mobile phone first."));
}

/*!
    \fn DeviceHome::smsListRightClick(QListViewItem *item, const QPoint &point, int column)
 */
void DeviceHome::smsListRightClick(Q3ListViewItem *item, const QPoint &point, int )
{
    if(!item) return;
    popupSMS *popup=new popupSMS( ((SMSListViewItem*)item)->sms(), ui.SMSListView);
//     connect(popup, SIGNAL(editClicked(KABC::Addressee* ) ), this, SLOT(slotEditContact(KABC::Addressee* ) ) );
//     connect(popup, SIGNAL(delContact() ), this, SLOT(slotDeleteContact() ) );
    connect(popup, SIGNAL(remove( SMS* )), engine, SLOT(slotDelSMS(SMS*)) );
    popup->exec(point);
}


/*!
    \fn DeviceHome::raiseDevice()
 */
void DeviceHome::raiseDevice()
{
/*  @TODO remove this?
    QWidget *widget=kapp->mainWidget();
    if(widget)
    {
        // Thanks to boren@ #kde-devel for these infos ;-)
//         KWM::setCurrentDesktop(KWM::windowInfo( widget->winId() ).desktop() );
        KWindowSystem::activateWindow( widget->winId() );
        widget->show();
        widget->raise();
        widget->setFocus();
        widget->activateWindow();
    }*/
#if 0 // port to D-Bus!
    (new MainIFace_stub("kmobiletools", "KMobileTools"))->switchPart(objectName() );
#endif
}


/*!
    \fn DeviceHome::smsModified(const QString & smsUID)
 */
void DeviceHome::smsModified(const QString & smsUID)
{
    kDebug( ) << "DeviceHome::smsModified(" << smsUID << ")\n";
}


/*!
    \fn DeviceHome::smsAdded(const QString & smsUID)
 */
void DeviceHome::smsAdded(const QString & smsUID)
{
    updateSMSCount();
    home->printInfoPage( home->currentInfoPage(), engine );
    kDebug( ) << "DeviceHome::smsAdded(" << smsUID << ")\n";
    const SMSList *smsList = engine->constEngineData()->smsList();
    int newSMSIndex=smsList->find( smsUID );
    if(newSMSIndex<0) return;
    SMS *newSMS=smsList->at(newSMSIndex);
    if( !(newSMS->slot() & memslotSelected) || ! (newSMS->type() & smsTypeSelected)  ) return;
    new SMSListViewItem( ui.SMSListView, newSMS, engine->constEngineData()->contactsList() );
}

void DeviceHome::raisePage(int page)
{
    if(!ui.widgetStack->widget(page)) return;
    ui.widgetStack->raiseWidget(page);
}

QString DeviceHome::currentDeviceName() const
{
    return engine->currentDeviceName();
}

/*!
    \fn DeviceHome::smsRemoved(const QString & smsUID)
 */
void DeviceHome::smsRemoved(const QString & smsUID)
{
//     kDebug( ) << "DeviceHome::smsRemoved(" << smsUID << ")\n";
//     updateSMSList();
//     return;
    updateSMSCount();
    home->printInfoPage( home->currentInfoPage(), engine );
    const SMSList *smsList = engine->constEngineData()->smsList();
    int oldSMSIndex=smsList->find( smsUID );
    if(oldSMSIndex<0) return;
    SMS *oldSMS=smsList->at(oldSMSIndex);
    if( !(oldSMS->slot() & memslotSelected) || ! (oldSMS->type() & smsTypeSelected)  ) return;

    Q3ListViewItemIterator it(ui.SMSListView);
    SMSListViewItem *tempItem;
    while( (tempItem=((SMSListViewItem*)it.current())) !=0 )
    {
        if( tempItem->sms()->uid() == smsUID )
        {
            delete tempItem;
//             return;
        }
        ++it;
    }
//     smsList->dump();
}


/*!
    \fn DeviceHome::smsFolderClicked( QListViewItem * item )
 */
void DeviceHome::smsFolderClicked( Q3ListViewItem * item )
{
    if(!item) return;
    SMSFolderListViewItem *sms_item=(SMSFolderListViewItem*) item;
    memslotSelected=sms_item->memSlot(); smsTypeSelected=sms_item->smsType();
    updateSMSList();
}

void DeviceHome::updateSMSList()
{
    ui.SMSListView->clear();

    QListIterator<SMS*> it( *(engine->constEngineData()->smsList()) );

    while( it.hasNext() )
    {
        if( !(it.next()->slot() & memslotSelected) || ! (it.next()->type() & smsTypeSelected)  ) continue;
        new SMSListViewItem( ui.SMSListView, it.next(), engine->constEngineData()->contactsList() );
    }
    updateSMSCount();

    if( (smsTypeSelected & SMS::Unread) || (smsTypeSelected & SMS::Read) )
    {
        ui.SMSListView->adjustColumn(0);
        ui.SMSListView->hideColumn(1);
    } else
    {
        ui.SMSListView->adjustColumn(1);
        ui.SMSListView->hideColumn(0);
    }
}

void DeviceHome::slotNewSMS(const QString &number)
{
    newSMSDlg *dialogNewSMS = new newSMSDlg(m_widget, objectName() );
    if(! number.isNull() ) dialogNewSMS->addNumber( number );
    if(! dialogNewSMS->exec()) return;
    if( dialogNewSMS->action() & newSMSDlg::Send )
        engine->slotSendSMS(dialogNewSMS->getSMSItem() );
    if( dialogNewSMS->action() & newSMSDlg::Store )
        engine->slotStoreSMS(dialogNewSMS->getSMSItem() );
}


void DeviceHome::sendSMS(const QString& number, const QString& text)
{
    engine->slotSendSMS( number, text );
}
void DeviceHome::storeSMS(const QString& number, const QString& text)
{
    engine->slotStoreSMS( number, text);
}


/*!
    \fn DeviceHome::openURL(const KUrl &url)
 */
void DeviceHome::openURL(const KUrl &url)
{
    kDebug() << "Parsing url " << url << endl;
    if(url.path() == "sms") ui.widgetStack->raiseWidget(SMS_WIDGET_ID);
    if(url.path() == "phonebook") ui.widgetStack->raiseWidget(PHONEBOOK_WIDGET_ID);
    if(url.path() == "tryconnect")
    {
        if(suspends_count) resumeDevice();
        else engine->slotSearchPhone();
        home->printInfoPage( 2, engine);
    }
    if(url.path() == "configure")
    {
        emit command( QString("configure:") + objectName() );
        kDebug() << "emitted command(" << QString("configure:") + objectName() << ")\n";
    }
}


/*!
    \fn DeviceHome::updateSMSCount()
 */
void DeviceHome::updateSMSCount()
{
    if(!engine) return;
    engine->constEngineData()->smsList()->calcSMSNumber();
    Q3ListViewItemIterator it( ui.SMSFolderView );
    SMSFolderListViewItem *tempItem;
    while ( it.current() ) {
        tempItem=(SMSFolderListViewItem*) it.current();
        tempItem->setText(1, QString::number(engine->constEngineData()->smsList()->count( ( tempItem->smsType() & (SMS::Unread | SMS::Unsent) ), tempItem->memSlot() ) ) );
        tempItem->setText(2,QString::number(engine->constEngineData()->smsList()->count( tempItem->smsType(), tempItem->memSlot() ) ) );
        ++it;
    }
}

// This method is partially derived from akregator - articlelistview.cpp, See copyright statement below
//    Copyright (C) 2007 Frank Osterfeld <frank.osterfeld at kdemail.net>


void SMSFolderListViewItem::paintCell( QPainter * p, const QColorGroup & cg,
                                 int column, int width, int align )

{
    if(column != 1 || text(column).toInt() <=0 )
    {
        K3ListViewItem::paintCell(p,cg,column,width,align);
        return;
    }
    QColorGroup cg2(cg);
    if( i_smsType & SMS::Unsent) cg2.setColor(QColorGroup::Text, Qt::red);
    else cg2.setColor(QColorGroup::Text, Qt::blue);
    K3ListViewItem::paintCell( p, cg2, column, width, align );
}



/*!
    \fn DeviceHome::fullPhonebook()
 */
void DeviceHome::fullPhonebook()
{
    KMessageBox::error( m_widget, i18n("Could not import all contacts. Phonebook is full.") );
}


/*!
    \fn DeviceHome::slotFetchCalendar()
 */
void DeviceHome::slotFetchCalendar()
{
    engine->slotFetchCalendar();
}


/*!
    \fn DeviceHome::slotCalendarFetched()
 */
void DeviceHome::slotCalendarFetched()
{
#ifdef HAVE_KCAL
    QString savefile=KGlobal::dirs()->saveLocation( "data", "kmobiletools", true).append( "%1.vcs" ).arg(objectName() );

    korgpart->closeUrl();
    kDebug() << "DeviceHome::slotCalendarFetched()\n";
    Calendar *engineCal=constEngineData->calendar();
    p_calendar->deleteAllEvents ();
    for(Calendar::Iterator it=engineCal->begin(); it!=engineCal->end(); ++it)
        p_calendar->addEvent(*it);
    p_calendar->save(savefile );
    if(engineCal->count() ==0 )
    {
        p_calendar->close();
        delete p_calendar;
        QFile::remove( savefile );
        p_calendar=new KCal::CalendarLocal("");
        p_calendar->save(savefile );
    } else
    {
        KRES::Manager<KCal::ResourceLocal> *manager=new KRES::Manager<KCal::ResourceLocal>("calendar");
        KConfig *config=new KConfig("kresources/calendar/stdrc");
        manager->readConfig( config );
        KRES::Manager<KCal::ResourceLocal>::Iterator it;
        bool isPresent=false;
        for(it=manager->begin(); it!=manager->end(); it++)
            if( (*it)->fileName() == savefile) isPresent=true;
        if(!isPresent)
        {
            KCal::ResourceLocal *res=new KCal::ResourceLocal(savefile);
            res->setResourceName( DEVCFG(objectName() )->devicename() );
//            res->setValue("Format", "vcal");
            res->setReadOnly(true);
            manager->add(res);
            manager->writeConfig(config);
        }
        delete config;
        delete manager;
    }
    korgpart->openUrl( savefile );
#endif
}


/*!
    \fn DeviceHome::slotExportSMSList()
 */
void DeviceHome::slotExportSMSList()
{
//     if ( KMobileTools::MainConfig::self()->maildir() )
    KMobileTools::KMobiletoolsHelper::createMailDir( objectName() );
    engine->constEngineData()->smsList()->saveToMailBox();
    kDebug() << "STARTING SMS EXPORT\n";
    KMessageBox::information( m_widget, i18n("<qt>SMS List for the mobile phone <b>%1</b> was exported to KMail default directory (%2).<br>To view exported messages, close and reopen KMail.</qt>", DEVCFG(objectName() )->devicename(), DEVCFG(objectName() )->maildir_path() ), i18n("SMS List Exported."), "smslistexported_infobox" );
}

/*!
    \fn DeviceHome::slotExportSMSListToCSV()
 */
void DeviceHome::slotExportSMSListToCSV()
{
    int result;
    
    kDebug() << "STARTING SMS EXPORT TO CSV\n";
    result = engine->constEngineData()->smsList()->saveToCSV();
    if (result >= 1) {
        KMessageBox::information( m_widget, i18n("<qt>SMS List for the mobile phone <b>%1</b> was exported to the selected Directory.</qt>", DEVCFG(objectName() )->devicename() ), i18n("SMS List Exported."), "smslistexportedtocsv_infobox" );
    }
}


/*!
    \fn DeviceHome::setupStatusBar()
 */
void DeviceHome::slotStatusBar()
{
    statusBarExtension=((kmobiletoolsMainPart *)parent())->statusBarExtension();

    signalProgress=new QProgressBar(statusBarExtension->statusBar());
    batteryProgress=new QProgressBar(statusBarExtension->statusBar());
    networkLabel=new QLabel(statusBarExtension->statusBar() );
    signalProgress->setMaximumSize(70,16);
    batteryProgress->setMaximumSize(70, 16);
//     statusBarExtension->statusBar()->clear();
    statusBarBox=new StatusBarProgressBox( statusBarExtension->statusBar(), widget() );
    devNameLabel=new QLabel(DEVCFG(objectName())->devicename(), statusBarExtension->statusBar());
    voidLabel=new QLabel(statusBarExtension->statusBar());
    batteryLabel=new QLabel(i18n("Battery"), statusBarExtension->statusBar());
    signalLabel=new QLabel(i18n("Signal"), statusBarExtension->statusBar());
    if(!engine) return;
    connect(engine, SIGNAL(charge(int ) ), batteryProgress, SLOT(setValue(int ) ) );
    connect(engine->constEngineData(), SIGNAL(signalStrengthChanged( int ) ), signalProgress, SLOT(setValue( int ) ) );
    connect(engine, SIGNAL(networkNameChanged( const QString &) ), networkLabel, SLOT(setText(const QString& ) ) );
//     connect(engine, SIGNAL( jobEnqueued(KMobileTools::Job *) ), statusBarBox, SLOT(slotJobEnqueued(KMobileTools::Job* ) ) );
}


/*!
    \fn DeviceHome::slotSaveAddressBook()
 */
void DeviceHome::slotSaveAddressBook()
{
    /** @TODO fix kabc saving
    if( engine->constEngineData()->contactsList()->isEmpty() ) return;
    KABC::AddressBook *p_addressbook;
//     KABCResFile *p_resourcefile;
    QString filename=KGlobal::dirs()->saveLocation( "data", "kmobiletools", true).append( "%1.vcf" ).arg(objectName() );
    p_resourcefile=0;
    KRES::Manager<KABC::Resource> *manager=new KRES::Manager<KABC::Resource>("contact");
    KConfig *config=new KConfig("kresources/contact/stdrc");
    manager->readConfig( config );
    KRES::Manager<KABC::Resource>::Iterator it;
    bool found=false;
    for(it=manager->begin(); it!=manager->end(); it++)
        if( (*it)->fileName() == filename )
    {
        p_resourcefile=(*it);
        found=true;
        break;
    }
    if(!found)
        p_resourcefile=new KABCResFile( filename );
    p_addressbook=new KABC::AddressBook();
    p_resourcefile->setResourceName( DEVCFG(objectName() )->devicename() );

    p_addressbook->addResource( p_resourcefile);
    p_addressbook->load();
    p_addressbook->clear();
    KABC::Addressee::List::ConstIterator pbit=engine->constEngineData()->contactsList()->begin();
    KABC::Addressee::List::ConstIterator pbend=engine->constEngineData()->contactsList()->end();
    for(; pbit != pbend; ++pbit)
    {
        p_addressbook->insertAddressee(*pbit);
    }
    p_resourcefile->setReadOnly(false);
    KABC::Ticket *ticket=p_addressbook->requestSaveTicket( p_resourcefile );
    if(!ticket)
    {
        kDebug() << "Error: Unable to save to KAddressBook (engine " << objectName() << ", " << DEVCFG(objectName() )->devicename() << "; filename: " << p_resourcefile->fileName() << ")\n";
        return;
    }
    p_addressbook->save( ticket );
    p_resourcefile->setReadOnly(true);

    if( !found && ! p_addressbook->allAddressees().isEmpty() )
    {
        manager->add(p_resourcefile);
        manager->writeConfig(config);
    }
    delete p_addressbook;
//     delete p_resourcefile;
    delete config;
//     delete manager; */
}

void DeviceHome::slotSendStoredSMS( SMS *sms)
{
    engine->slotSendStoredSMS( sms);
}

/// KNotify Methods

void DeviceHome::slotRing( bool ringing)
{
    if(!ringing) return;
    kDebug() << "KNotify for ring event\n";
    KNotification::event( QString("kmobiletools_ring"), i18n("Incoming Call"), QPixmap(),
        KMobileTools::KMobiletoolsHelper::instance()->systray()->contextMenu() );
}


/*!
    \fn DeviceHome::slotDial()
 */
void DeviceHome::slotDial()
{
    if( !ui.number_dial->currentText().length()) return;
    if(! engine->constEngineData()->phoneConnected())
    {
        errNotConnected();
        return;
    }
    QString num=ui.number_dial->currentText();
    QString name=KMobiletoolsHelper::translateNumber(num);
    if(name==num) name.clear() ;
    (new callDialogImpl(engine, m_widget))->call(num, name );
}

void DeviceHome::slotDialNumber(const QString &number)
{
    if(b_dialing) return;
    ui.number_dial->setCurrentText(number);
    slotDial();
}


/*!
    \fn DeviceHome::switch2filesystem()
 */
void DeviceHome::switch2filesystem()
{
    engine->slotSwitchToFSMode();
}


/*!
    \fn DeviceHome::contactsTabChanged()
 */
void DeviceHome::contactsTabChanged()
{
    kDebug() << "Contacts tab index: " <<ui.contactsTab->currentIndex() << endl;
    if(ui.contactsTab->currentIndex())
        slwidget->searchLine()->setListView(ui.phonebookListViewFull);
    else slwidget->searchLine()->setListView(ui.phonebookListView);
}
