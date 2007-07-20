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
#include "at_engine.h"

#include <qregexp.h>
//Added by qt3to4:
#include <Q3ValueList>

#include <qtimer.h>
#include <klocale.h>
#include <QWizardPage>
#include <QLabel>
#include <ksharedconfig.h>
#include "wizard/at_connectionspage.h"
#include "wizard/at_devicesfoundpage.h"
#include "wizard/at_scanprogresspage.h"
#include "cfgwidgets/engineconfig.h"

#include <libkmobiletools/picksmscenter.h>
#include <libkmobiletools/smslist.h>
#include <libkmobiletools/weaver.h>
#include <libkmobiletools/enginedata.h>

// Error handling includes
#include <libkmobiletools/errorhandler.h>

// Jobs includes
#include "at_jobs.h"
#include "sms_jobs.h"
#include "phonebook_jobs.h"
#include "testphonedevice.h"

#include <config-kmobiletools.h>


using namespace KMobileTools;

AT_Engine::AT_Engine(QObject *parent, const QString &name)
 : KMobileTools::Engine(parent, name), p_lastJob(NULL)
{
    KGlobal::locale()->insertCatalog( "kmobiletools_at_engine" );
    KLocale::setMainCatalog("kmobiletools_at_engine");
    device=NULL;
//     p_smsList = new SMSList();
    queue_sms=false;
    connect( this->engineData(), SIGNAL(connected()), this, SLOT(slotFetchInfos()) );
}


AT_Engine::~AT_Engine()
{
    kDebug() << "AT_Engine::~AT_Engine()\n";
//      weaver->finish();
//     closeDevice();
//     delete weaver;

}
void AT_Engine::queryClose()
{
        closeDevice();
        KMobileTools::Engine::queryClose();
}

K_EXPORT_COMPONENT_FACTORY( libkmobiletools_at, AT_EngineFactory )


AT_EngineFactory::AT_EngineFactory()
{
}
AT_EngineFactory::~AT_EngineFactory()
{
}
AT_Engine *AT_EngineFactory::createObject(QObject *parent, const char */*classname*/, const QStringList &/*args*/ )
{
    return new AT_Engine(parent); /// @TODO we have to specify engine name, perhaps in args
}




/*!
    \fn AT_Engine::probePhone()
 */
void AT_Engine::slotSearchPhone()
{
    kDebug() << "********** engine instance name: " << objectName() << endl;
    searchPhones(static_cast<ATDevicesConfig::Connection>(config()->at_connections()), QStringList(), config()->at_userdevices() );
//     devicesList()->probeDevices( config()->atdevices(), engineLibName(), QStringList(config()->at_initString())+=config()->at_initString2(), false, 0, config()->mobileimei() );
    /// @TODO reimplement
}


void AT_Engine::slotInitPhone()
{
/** @TODO replace this
    setFoundDevice(devicesList()->findByIMEI(config()->mobileimei()) );
    kDebug() << "Was searching for imei=\"" << config()->mobileimei() << "\"; list of devices found::\n";
    devicesList()->dump();
    if(!foundDevice() )
    {
        kDebug() << "Device not found\n";
        emit disconnected();
        return;
    } else kDebug() << "Device found on " << foundDevice()->foundPath() << endl;
    */
    device=new KMobileTools::SerialManager(this, this->objectName(), engineData()->property("devicePath").toString(), initStrings() );
    connect(device, SIGNAL(disconnected()), this, SLOT(connectionStateChanged()));
    connect(device, SIGNAL(error()), SIGNAL(error() ) );
    p_lastJob=new initPhoneJob(device, this);
    enqueueJob( p_lastJob );
}

QStringList AT_Engine::initStrings() 
{
    QStringList retval( config()->at_initString() );
    retval+=config()->at_initString2();
    return retval;
}



void AT_Engine::processSlot(KMobileTools::Job* job)
{
    KMobileTools::Engine::processSlot(job);
//     kDebug() << "job Owner: " << p_job->jobOwner() << "; job class: " << p_job->className() << endl;
    if(job->property("owner") != objectName() ) return;
//     kDebug() << "KMobileTools::Engine::processSlot; jobType=" << job->type() << endl;
    p_lastJob=0;
//     kDebug() << "is KMobileToolsJob: " << p_job->inherits("KMobileTools::Job") << endl;
    KMobileTools::DevicesConfig *wconfig=KMobileTools::DevicesConfig::prefs(objectName() );
    switch( job->type() ){
        case KMobileTools::Job::initPhone:
            kDebug() << "Device is connected: " << device->isConnected() << endl;
            engineData()->setPhoneConnected( device->isConnected() );
            break;
        case KMobileTools::Job::pollStatus:
            engineData()->setSignalStrength( ((PollStatus*) job)->phoneSignal() );
            engineData()->setCharge( ((PollStatus*) job)->phoneCharge() );
            engineData()->setChargeType( ((PollStatus*) job)->phoneChargeType() );
            engineData()->setPhoneRinging( ((PollStatus*) job)->ringing() );
            break;
        case KMobileTools::Job::fetchPhoneInfos:
            engineData()->setManufacturer( ( (FetchPhoneInfos*) job )->rawManufacturer() );
            engineData()->setModel( ( (FetchPhoneInfos*) job )->model() );
            engineData()->setRevision (( (FetchPhoneInfos*) job )->revision() );
            engineData()->setIMEI( ( (FetchPhoneInfos*) job )->imei() );
            engineData()->setSMSCenter(( (FetchPhoneInfos*) job )->smsCenter() );
            if(! engineData()->smsCenter().isNull() )
                engineData()->setNetworkName( i18n("Network: %1",
                PickSMSCenter::smsCenterName (engineData()->smsCenter() ) ) );
            if ( engineData()->manufacturer().contains( "Siemens", Qt::CaseInsensitive ) ) engineData()->setManufacturerID( Siemens);
            if ( engineData()->manufacturer().contains( "Motorola", Qt::CaseInsensitive ) ) engineData()->setManufacturerID ( Motorola);
            if ( engineData()->manufacturer().contains( "Ericsson", Qt::CaseInsensitive ) ) engineData()->setManufacturerID ( SonyEricsson);
            wconfig->setRawdevicename( engineData()->model() );
            wconfig->setRawdevicevendor( engineData()->manufacturer() );
            wconfig->writeConfig();
            break;
        case KMobileTools::Job::fetchAddressBook:
            suspendStatusJobs(false);
            engineData()->setContactsList( (ContactsList*) &((FetchAddressee*) job )->fullAddresseeList() );
//                 kDebug() << "trying to call KMobileTools::EnginesList::instance()->emitPhonebookUpdated();\n";
//                 KMobileTools::EnginesList::instance()->emitPhonebookUpdated();
//                 if( ! ((FetchAddressee*) job )->partialUpdates() )
            //emit phoneBookChanged();
            break;
        case KMobileTools::Job::fetchSMS:
            if( ((FetchSMS*) job)->last()) {
                engineData()->setSMSList( ((FetchSMS*)job)->smsList );
                queue_sms=false;
            }
//          emit smsListUpdated();
            break;
        case KMobileTools::Job::testPhoneFeatures:
            wconfig->setFstype( ((TestPhoneFeatures *) job)->getAbilities().filesystem() );
        wconfig->writeConfig();
        break;
        case KMobileTools::Job::addAddressee:
            slotFetchPhonebook();
            if( ((EditAddressees*)job)->pbIsFull() ) {
                /// @TODO replace BaseError with a more appropriate type as soon as the type is defined ;-)
                ErrorHandler::instance()->addError( new BaseError(ERROR_META_INFO) );
            }
            suspendStatusJobs(false);
            break;
        case KMobileTools::Job::editAddressee:
            slotFetchPhonebook();
            if( ((EditAddressees*)job)->pbIsFull() ) {
                /// @TODO replace BaseError with a more appropriate type as soon as the type is defined ;-)
                ErrorHandler::instance()->addError( new BaseError(ERROR_META_INFO) );
            }
            suspendStatusJobs(false);
            break;
        case KMobileTools::Job::delAddressee:
            slotFetchPhonebook();
            suspendStatusJobs(false);
            break;
        case KMobileTools::Job::selectSMSSlot:
            if( ((SelectSMSSlot*) job)->done() )
                setSMSSlot( config()->at_smsslots().indexOf( ((SelectSMSSlot*) job)->getReadSlot() ) );
            break;
        case KMobileTools::Job::delSMS:
            if( ! ((DeleteSMS*)job)->succeeded() ) break;
            //emit smsDeleted( ((DeleteSMS*)job)->sms()->uid() );
            break;
        case KMobileTools::Job::storeSMS:
        case KMobileTools::Job::sendStoredSMS:
        case KMobileTools::Job::sendSMS:
            suspendStatusJobs(false);
            break;
        case KMobileTools::Job::fetchKCal:
#ifdef HAVE_KCAL
            suspendStatusJobs(false);
            engineData()->setCalendar( p_calendar );
            p_calendar->dump();
#endif
            break;
        case TestPhoneDevice:
            l_testphonejobs.removeAll((TestPhoneDeviceJob*)job);
            if( ((TestPhoneDeviceJob*)job)->found() &&
                ((TestPhoneDeviceJob*)job)->data()->imei() == config()->mobileimei() ) {
                kDebug() << "Probe finished: phone found in " << ((TestPhoneDeviceJob*)job)->path() << endl;
                while(!l_testphonejobs.isEmpty())
                    ThreadWeaver()->dequeue(l_testphonejobs.takeFirst());
                engineData()->setProperty("devicePath", ((TestPhoneDeviceJob*)job)->path());
                slotInitPhone();
            }
            kDebug() << "jobs remaining: " << l_testphonejobs.count() << endl;
            break;
        case FindDeviceData:
            emit foundDeviceData((FindDeviceDataJob*) job);
            break;
    }
}

#include "at_engine.moc"

void AT_Engine::slotPollStatus()
{
    if (statusJobsSuspended() ) return;
    if(!device) return;
    p_lastJob=( new PollStatus(p_lastJob, device, this ) );
    enqueueJob(p_lastJob);
}

void AT_Engine::slotFetchInfos()
{
    if(!device) return;
    enqueueJob ( new FetchPhoneInfos(device, this ) );
    enqueueJob ( new TestPhoneFeatures(device, this ) );
    if( config()->sync_clock() )
    {
      p_lastJob=new SyncDateTime(p_lastJob, device, this );
      enqueueJob ( p_lastJob );
    }
    p_lastJob=new SelectCharacterSet( p_lastJob, config()->at_encoding(), device, this );
    enqueueJob( p_lastJob );
    p_lastJob=new SelectSMSSlot( p_lastJob, "ME", device, this );
    enqueueJob( p_lastJob );
//    enqueueJob ( new FetchAddresseeSiemens(device, this, this->objectName() ) );
//    enqueueJob ( new FetchSMS( SMS::All, device, this, this->objectName() ) );
}

bool AT_Engine::pdu()
{
    return atAbilities.isPDU();
}


/*!
    \fn AT_Engine::retrieveAddressBook()
 */
void AT_Engine::slotFetchPhonebook()
{
    if(!device) return;
    KMobileTools::Job *job;
    if( atAbilities.canSiemensVCF() || atAbilities.canSDBR() )
        job=( new FetchAddresseeSiemens(p_lastJob, device, this ) );
    else job= ( new FetchAddressee(p_lastJob, availPbSlots(), device, this ) );
    //connect(job, SIGNAL(gotAddresseeList(int, const ContactsList&) ), this, SIGNAL(phoneBookChanged(int, const ContactsList& ) ) );
    p_lastJob=job;
    enqueueJob(job);
}

void AT_Engine::setATAbilities( ATAbilities atAbilities )
{
    this->atAbilities = atAbilities;
}

ATAbilities AT_Engine::getATAbilities() const
{
    return atAbilities;
}


void AT_Engine::slotFetchSMS()
{
    if(statusJobsSuspended() || ! device ) return;
//     p_smsList->clear();
    if(queue_sms) return;
    QStringList sl_slots=config()->at_smsslots();
    if( ! sl_slots.count() ) {
        kDebug() << "**** WARNING - this phone is NOT reporting having SMS slots. Perhaps it can't provide SMS. I'm trying anyway to fetch them.\n";
        p_lastJob=new FetchSMS(p_lastJob, SMS::All, device, true, this );
        enqueueJob(p_lastJob);
        queue_sms=true;
        return;
    }
    for (QStringList::iterator it=sl_slots.begin(); it!=sl_slots.end(); ++it)
    {
        p_lastJob=new SelectSMSSlot( p_lastJob, (*it), device, this );
        enqueueJob( p_lastJob );
    p_lastJob=new FetchSMS( p_lastJob, SMS::All, device, *it==sl_slots.last() ,this );
        enqueueJob( p_lastJob );
        queue_sms=true;
    }
}

/*!
    \fn AT_Engine::addAddressee(KABC::Addressee *addressee)
 */
void AT_Engine::slotAddAddressee(const KABC::Addressee::List& abclist)
{
    EditAddressees *tempjob=new EditAddressees(p_lastJob, abclist, device, false, this );
    p_lastJob=tempjob;
    if(device) enqueueJob( tempjob ) ;
}
/*!
    \fn AT_Engine::delAddressee(int index)
 */
void AT_Engine::slotDelAddressee(const KABC::Addressee::List& abclist)
{
    if(device) {
        p_lastJob=new EditAddressees(p_lastJob, abclist, device, true, this );
    enqueueJob(p_lastJob);
    }
}

int AT_Engine::availPbSlots()
{
    int retval=0;
    if ( atAbilities.getPBSlots().indexOf("ME") >=0 ) retval+= PB_Phone;
    if ( atAbilities.getPBSlots().indexOf("SM") >=0 ) retval+= PB_SIM;
    if ( atAbilities.getPBSlots().indexOf("TA") >=0 ) retval+= PB_DataCard;
    return retval;
}


/*!
    \fn AT_Engine::getPBMemSlotString(int memslot)
 */
QString AT_Engine::getPBMemSlotString(int memslot)
{
    switch( memslot ){
        case PB_SIM:
            return QString("\"SM\"");
        case PB_Phone:
            return QString("\"ME\"");
        case PB_DataCard:
            return QString("\"TA\"");
        default:
            return QString() ;
    }
}


void AT_Engine::slotEditAddressee(const KABC::Addressee& p_oldAddressee, const KABC::Addressee& p_newAddressee)
{
    EditAddressees *tempjob=new EditAddressees( p_lastJob, p_oldAddressee, p_newAddressee, device, this );
    p_lastJob=tempjob;
    if(device) enqueueJob( tempjob );
}


void AT_Engine::slotWeaverSuspended()
{
    KMobileTools::Engine::slotWeaverSuspended();
    /// @TODO Look if we can fix this, otherwise remove
//     device->stopDevice();
}


/*!
    \fn AT_Engine::resumeDevice()
 */
void AT_Engine::slotResumeDevice()
{
    KMobileTools::Engine::slotResumeDevice();
    /// @TODO Look if we can fix this, otherwise remove
//     device->resumeDevice();
    initPhoneJob* tempjob=new initPhoneJob(device, this);
    enqueueJob( tempjob );
    p_lastJob=tempjob;
    suspendStatusJobs(false);
}


/*!
    \fn AT_Engine::stopDevice()
 */
void AT_Engine::slotStopDevice()
{
    if(! statusJobsSuspended() ) suspendStatusJobs(true);
    if(ThreadWeaver()->queueLength())
    {
        QTimer::singleShot( 500, this, SLOT(stopDevice()));
        return;
    }
    KMobileTools::Engine::slotStopDevice();
    device->close();
}


/*!
    \fn AT_Engine::setDevice ( const QString &deviceName)
 */
void AT_Engine::setDevice ( const QString &deviceName)
{
//     delete device;
    device->setDevicePath(deviceName );
}

/*!
    \fn AT_Engine::probeDevice ( const QString &deviceName, const QString &param1, const QString &param2)
 */
/*
DeviceInfos *AT_Engine::slotProbePhoneCaps( KMobileTools::Job *job, bool fullprobe, const QString &deviceName, const QStringList &params ) const
{
    kDebug() << "AT_Engine::probePhoneCaps: device path: " << deviceName << endl;
    KMobileTools::SerialManager *device=new KMobileTools::SerialManager(0, "nodevice", deviceName, params );
    connect(device, SIGNAL(invalidLockFile( const QString& )), this, SIGNAL(invalidLockFile( const QString& )) );
//     kDebug() << "Probing device " << deviceName << endl;
    if(! device->open(job) ) return new DeviceInfos();
//     kDebug() << deviceName << " opened" << endl;
    QString buffer, temp;
    DeviceInfos *retval=new DeviceInfos;
    retval->setFoundPath( deviceName );
    const int probeTimeout=600;
    buffer=device->sendATCommand(job, "AT+CGSN\r", probeTimeout);
    if( !KMobileTools::SerialManager::ATError(buffer) ) // Phone imei
        retval->setImei( kmobiletoolsATJob::parseInfo( buffer ) );
    else  { closeDevice(device); delete device; return retval; }
    if(! fullprobe )
    {
        closeDevice(device);
        delete device;
        return retval;
    }

    buffer=device->sendATCommand(job, "AT+CGMR\r", probeTimeout);
    if(!KMobileTools::SerialManager::ATError(buffer)) // Phone revision
        retval->setRevision( kmobiletoolsATJob::parseInfo( buffer ) );

    buffer=device->sendATCommand(job, "AT+CGMI\r", probeTimeout);
    if( !KMobileTools::SerialManager::ATError(buffer) ) // Phone manufacturer
        retval->setManufacturer( kmobiletoolsATJob::parseInfo( buffer ) );

    buffer=device->sendATCommand(job, "AT+CPMS=?\r", probeTimeout);
    if( !KMobileTools::SerialManager::ATError(buffer) ) // SMS Slots
        temp = kmobiletoolsATJob::parseInfo( buffer );
    else temp.clear();
    retval->setSMSSlots( kmobiletoolsATJob::parseList( temp.replace("AT+CPMS=?", "") ) );

    buffer=device->sendATCommand(job, "AT+CPBS=?\r", probeTimeout);
    if( !KMobileTools::SerialManager::ATError(buffer) ) // PhoneBook Slots
        temp = kmobiletoolsATJob::parseInfo( buffer );
    else temp.clear();
    retval->setPbSlots( kmobiletoolsATJob::parseList( temp.replace("AT+CPBS=?", "") ) );

    buffer=device->sendATCommand(job, "AT+CGMM\r", probeTimeout);
    if(!KMobileTools::SerialManager::ATError(buffer)) // Phone model
        retval->setModel( kmobiletoolsATJob::parseInfo( buffer ) );

    buffer=device->sendATCommand(job, "AT+CSCS=?\r", probeTimeout);
    if( !KMobileTools::SerialManager::ATError(buffer) ) // Charset
        temp = kmobiletoolsATJob::parseInfo( buffer );
    else temp.clear();
    retval->setCharsets( kmobiletoolsATJob::parseList( temp ) );

    buffer=device->sendATCommand(job, "AT+CSCA?\r", probeTimeout); // SMS Center
    if(!KMobileTools::SerialManager::ATError(buffer))
    {
        temp=kmobiletoolsATJob::parseInfo( buffer);
        QRegExp tempRegexp;
        tempRegexp.setPattern( ".*\"(.*)\".*");
        if(tempRegexp.indexIn(temp)>=0) temp=tempRegexp.cap( 1 ); else temp.clear();
        retval->setSMSCenter( temp );
    }
    closeDevice(device);
    delete device;
    return retval;
}
*/

/*!
    \fn AT_Engine::slotDelSMS(SMS* sms);

 */
void AT_Engine::slotDelSMS(SMS* sms)
{
    if(!device) return;
    p_lastJob=new SelectSMSSlot( p_lastJob, sms->rawSlot(), device, this ) ;
    enqueueJob(p_lastJob);
    p_lastJob=new DeleteSMS( p_lastJob, sms, device, this) ;
}

void AT_Engine::slotStoreSMS( const QString &number, const QString &text)
{
    if(!device) return;
    p_lastJob=( new StoreSMS(p_lastJob, number, text, device, this ));
    enqueueJob(p_lastJob);
}


void AT_Engine::slotSendSMS( const QString &number, const QString &text)
{
    if(!device) return;
    p_lastJob=( new SendSMS(p_lastJob, number, text, device, this ));
    enqueueJob(p_lastJob);
}


/*!
    \fn AT_Engine::slotSendSMS(SMS*)
 */
void AT_Engine::slotSendSMS(SMS* sms)
{
    if(!device) return;
    p_lastJob=( new SendSMS(p_lastJob, sms, device, this ) );
    enqueueJob(p_lastJob);
}

void AT_Engine::slotSendStoredSMS(SMS* sms)
{
    if(!device) return;
    p_lastJob=( new SendStoredSMS(p_lastJob, sms, device, this ) );
    enqueueJob(p_lastJob);
}


/*!
    \fn AT_Engine::slotStoreSMS(SMS*)
 */
void AT_Engine::slotStoreSMS(SMS* sms)
{
    if(!device) return;
    p_lastJob=( new StoreSMS(p_lastJob, sms, device, this ) );
    enqueueJob(p_lastJob);
}


/*!
    \fn AT_Engine::fetchCalendar()
 */
void AT_Engine::slotFetchCalendar()
{
#ifdef HAVE_KCAL
    if(!device) return;
    p_lastJob=( new FetchCalendar(p_lastJob, device, this ) );
    enqueueJob(p_lastJob);
#endif
}


/*!
    \fn AT_Engine::dial(DialActions)
 */
void AT_Engine::slotDial(DialActions action, const QString &gnumber)
{
    uint dialsystem=config()->at_dialsystem();
    QString number=gnumber;
    switch( action ){
        case DIAL_DIAL:
            if(number.isNull()) return;
            switch( dialsystem ){
                case 0:
                    if( number.at(0)=='+' )
                        number=number.right( number.length() -1 ).prepend("AT+CKPD=\"0\",20;+CKPD=\"").append("s\"\r" );
                    else number=number.prepend( "AT+CKPD=\"" ).append( "s\"\r" );
                    break;
                case 1:
                    number=number.prepend( "ATD" ).append( ";\r" );
                    break;
            }
            device->sendATCommand(0, number, 1500);
            break;
        case DIAL_HANGUP:
            switch( dialsystem ){
                case 0:
                    device->sendATCommand(0, "AT+CKPD=\"e\"\r", 1500 );
                    break;
                case 1:
                    device->sendATCommand( 0, "ATH" );
                    device->sendATCommand( 0, "AT+CHUP" );
                    break;
            }
            break;
    }
}

QString AT_Engine::currentDeviceName() const
{
    return device->devicePath();
}


QString AT_Engine::engineLibName() const
{
    return QString("libkmobiletools_at");
}

/*!
    \fn AT_Engine::switchToFSMode()
 */
void AT_Engine::slotSwitchToFSMode()
{
    switch( config()->fstype() ){
        case 1:
            device->sendATCommand(0, "AT+MODE=8\r");
            slotStopDevice();
            break;
        default:
            KMobileTools::Engine::slotSwitchToFSMode();
            break;
    }
}

QList<QWizardPage*> AT_Engine::wizardPages(QWidget *parentWidget)
{
    QList<QWizardPage*> retval;
    retval+=new AT_ConnectionsPage(parentWidget);
    retval+=new AT_ScanProgressPage(parentWidget);
    retval+=new AT_DevicesFoundPage(parentWidget);
    return retval;
}

QList<QWidget*> AT_Engine::configWidgets(QWidget *parentWidget)
{
    QList<QWidget*> retval;
    QWidget *tempPage;
    tempPage=new EngineConfigWidget(parentWidget);
    tempPage->setProperty("itemName", i18nc("At Engine Configuration Dialog title", "AT Engine") );
    tempPage->setProperty("pixmapName", QString("kmobiletoolsAT") );
    tempPage->setProperty("header", i18nc("At Engine Configuration Dialog Header", "AT Engine Configuration Page") );
    retval+=tempPage;
    return retval;
}


QString AT_Engine::parseWizardSummary(const QString &strtemplate, const QString &deviceName) const
{
    QString retstr=strtemplate;
    ATDevicesConfig *cfg=(ATDevicesConfig*)DEVCFG(deviceName);
    int conn=cfg->at_connections();
    QStringList tmpstrlist;
    if(conn & ATDevicesConfig::ConnectionUSB)        tmpstrlist+=i18nc("USB Connection", "USB");
    if(conn & ATDevicesConfig::ConnectionSerial)     tmpstrlist+=i18nc("Serial Connection", "Serial Ports");
    if(conn & ATDevicesConfig::ConnectionBluetooth)  tmpstrlist+=i18nc("Bluetooth Connection", "Bluetooth");
    if(conn & ATDevicesConfig::ConnectionIrDA)       tmpstrlist+=i18nc("IrDA Connection", "Infrared");
    if(conn & ATDevicesConfig::ConnectionUser)       tmpstrlist+=i18nc("User-defined Connection", "User defined");
    kDebug() << "conn=" << conn << "; stringlist: " << tmpstrlist << endl;
    QString tempstr=i18ncp("AT Wizard summary - using <connection types>", "Using connection: %2", "Using connections: %2",
        tmpstrlist.count(), tmpstrlist.join(", ") );
    retstr=retstr.arg(tempstr);
    tempstr=i18nc("AT Wizard summary - Basic parameters", "Manufacturer: %1<br>Model: %2", cfg->rawdevicevendor(), cfg->rawdevicename() );
    retstr=retstr.arg(tempstr);
    tempstr=i18nc("AT Wizard summary - Advanced parameters", "IMEI: %1", cfg->mobileimei() );
    retstr=retstr.arg(tempstr);

    return retstr;
}

ATDevicesConfig *AT_Engine::config(bool forceNew, const QString &groupName) {
    QString gpname;
    if(groupName.isEmpty()) gpname=objectName(); else gpname=groupName;
    kDebug() << "ATEngine config : force new=" << forceNew << "; groupname=" << gpname << endl;
    if(forceNew)
        return new ATDevicesConfig( gpname, gpname );
    return (ATDevicesConfig*) ATDevicesConfig::prefs(gpname );
}

void AT_Engine::enqueueTPJob(TestPhoneDeviceJob* pjob) {
    l_testphonejobs+=pjob;
    enqueueJob(pjob);
}

void AT_Engine::searchPhones(ATDevicesConfig::Connection connections, const QStringList &bluetoothDevices, const QStringList &customDevices) {
    if(!connections) return;
    QStringList devices;
    TestPhoneDeviceJob *curjob;
    if(connections & ATDevicesConfig::ConnectionUSB)
        for(uchar i=0; i<10; i++) {
            enqueueTPJob( new TestPhoneDeviceJob(QString("/dev/ttyACM%1").arg(i), this) );
            enqueueTPJob(new TestPhoneDeviceJob(QString("/dev/ttyUSB%1").arg(i), this) );
        }
    if(connections & ATDevicesConfig::ConnectionIrDA)
        for(uchar i=0; i<10; i++) {
            enqueueTPJob(new TestPhoneDeviceJob(QString("/dev/ircomm%1").arg(i), this) );
        }
    if(connections & ATDevicesConfig::ConnectionSerial)
        for(uchar i=0; i<10; i++) {
            enqueueTPJob(new TestPhoneDeviceJob(QString("/dev/ttyS%1").arg(i), this) );
        }
    if(connections & ATDevicesConfig::ConnectionBluetooth)
        for(QStringList::ConstIterator it=bluetoothDevices.begin(); it!=bluetoothDevices.end(); ++it)
            enqueueTPJob(new TestPhoneDeviceJob(*it, this) );
    if(connections & ATDevicesConfig::ConnectionUser)
        for(QStringList::ConstIterator it=customDevices.begin(); it!=customDevices.end(); ++it)
            enqueueTPJob(new TestPhoneDeviceJob(*it, this) );
}

void AT_Engine::connectionStateChanged() {
    engineData()->setPhoneConnected( device->isConnected() );
}

