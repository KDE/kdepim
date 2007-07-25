/***************************************************************************
   Copyright (C) 2005 by Marco Gulino <marco.gulino@gmail.com>
   Copyright (C) 2005 by Stefan Bogner <bochi@kmobiletools.org>
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

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

#include "kmobiletoolsgammu_engine.h"

// job includes
#include "initphonejob.h"
#include "phoneinfosjob.h"
#include "pollstatusjob.h"
#include "fetchaddressbookjob.h"
#include "editaddressbookjob.h"
#include "fetchsmsjob.h"
#include "deletesmsjob.h"
#include "storesmsjob.h"
#include "sendsmsjob.h"

#include "gammusms.h"

#include <kdebug.h>

kmobiletoolsGammu_engine::kmobiletoolsGammu_engine(QObject *parent, const char *name)
 : kmobiletoolsEngine(parent, name)
{
    kDebug() << "Initialized Gammu engine" << endl;

    m_device = new Device( this->name() );
}

kmobiletoolsGammu_engine::~kmobiletoolsGammu_engine()
{
}


#include "kmobiletoolsgammu_engine.moc"

QString kmobiletoolsGammu_engine::engineDesktopName() const {
    return "Gammu Engine";
}


QString kmobiletoolsGammu_engine::engineLibName() const {
    return "libkmobiletools_gammu";
}


bool kmobiletoolsGammu_engine::pdu() {
    return true;
}


int kmobiletoolsGammu_engine::availPbSlots()
{
    /// @todo make a job out of this
    QStringList phonebookSlots = m_device->phonebookSlots();

    int retval = 0;
    if ( phonebookSlots.contains("ME") )
        retval+= PB_Phone;

    if ( phonebookSlots.contains("SM") )
        retval+= PB_SIM;

    if ( phonebookSlots.contains("TA") )
        retval+= PB_DataCard;

    return retval;
}


void kmobiletoolsGammu_engine::queryClose() {
    m_device->terminatePhone();
}


void kmobiletoolsGammu_engine::processSlot( Job* p_job ) {
    kmobiletoolsEngine::processSlot(p_job);

    kmobiletoolsJob *job=(kmobiletoolsJob *) p_job;

    int unreadSMS = 0;
    int totalSMS = 0;

    switch( job->type() ){
        //
        // Misc phone jobs
        //
        case kmobiletoolsJob::initPhone:
            b_connected = m_device->phoneConnected();
            if( b_connected )
                emit connected();
            else
                emit disconnected();
            break;

        case kmobiletoolsJob::fetchPhoneInfos:
            s_manufacturer=( (PhoneInfosJob*) job )->manufacturer();
            if( s_manufacturer.contains( "Nokia", false ) )
                i_manufacturer = Nokia;
            s_model=( (PhoneInfosJob*) job )->model();
            s_revision=( (PhoneInfosJob*) job )->revision();
            s_imei=( (PhoneInfosJob*) job )->imei();
            break;

        case kmobiletoolsJob::pollStatus:
            emit charge( ( (PollStatusJob*) job )->phoneCharge() );
            emit signal( ( (PollStatusJob*) job )->phoneSignal() );
            emit networkName( ( (PollStatusJob*) job )->networkName() );
            emit isRinging(  ( (PollStatusJob*) job )->ringing() );

            unreadSMS = ( (PollStatusJob*) job )->unreadSMS();
            totalSMS = ( (PollStatusJob*) job )->totalSMS();

            if( unreadSMS != s_newSMS || totalSMS != s_totalSMS ) {
                s_newSMS = unreadSMS;
                s_totalSMS = totalSMS;
                emit updateInfoPage(0);
            }
            break;

        case kmobiletoolsJob::testPhoneFeatures:
            break;

        case kmobiletoolsJob::syncDateTimeJob:
            break;

        case kmobiletoolsJob::selectCharacterSet:
            break;

        //
        // Address book jobs
        //
        case kmobiletoolsJob::fetchAddressBook:
            m_addresseeList = ((FetchAddressBookJob*) job)->addresseeList();
            p_addresseeList= (ContactPtrList*) &m_addresseeList;
            emit phoneBookUpdated();
            break;

        case kmobiletoolsJob::addAddressee:
            // nothing to do here
            break;

        case kmobiletoolsJob::delAddressee:
            // nothing to do here
            break;

        case kmobiletoolsJob::editAddressee:
            // nothing to do here
            break;

        //
        // SMS jobs
        //
        case kmobiletoolsJob::fetchSMS:
            if( ((FetchSMSJob*) job)->smsList() == 0 )
                break;

            p_smsList->sync( ((FetchSMSJob*) job)->smsList() );
            emit smsListUpdated();
            break;

        case kmobiletoolsJob::smsFolders:
            break;

        case kmobiletoolsJob::sendSMS:
            // nothing to do here
            break;

        case kmobiletoolsJob::storeSMS:
            // nothing to do here
            break;

        case kmobiletoolsJob::sendStoredSMS:
            // nothing to do here
            break;

        case kmobiletoolsJob::delSMS:
            // nothing to do here
            break;

        case kmobiletoolsJob::selectSMSSlot:
            break;

        //
        // Calendar jobs
        //
        case kmobiletoolsJob::fetchKCal:
            break;
    }
}


void kmobiletoolsGammu_engine::probePhone() {
    kDebug() << "Gammu engine: probePhone() called" << endl;
    b_connected = false;

    // since we don't use DeviceList, we need to call initPhone() ourselves
    initPhone();

    /// @todo implement me
}


void kmobiletoolsGammu_engine::initPhone()
{
    kDebug() << "Gammu engine: initPhone() called" << endl;
    enqueueJob( new InitPhoneJob( m_device, this, this->name() ) );
}

DeviceInfos kmobiletoolsGammu_engine::probeDevice( ThreadWeaver::Job *job, bool fullprobe,
                            const QString &deviceName,
                            const QStringList &params ) const {
    Q_UNUSED(job)
    Q_UNUSED(fullprobe)
    Q_UNUSED(params)

    bool bluetooth = false;
    if( deviceName.contains( "bluetooth" ) )
        bluetooth = true;



    DeviceInfos deviceInfos;
    if( !m_device->phoneConnected() ) {
        // try to initialize phone
        m_device->initPhone();
        if( !m_device->phoneConnected() )
            return deviceInfos;
    }

    kDebug() << "Gammu engine: device name:" << deviceName << endl;


    deviceInfos.setFoundPath( deviceName );

    deviceInfos.setModel( m_device->model() );
    deviceInfos.setRevision( m_device->version() );
    deviceInfos.setImei( m_device->imei() );
    deviceInfos.setManufacturer( m_device->manufacturer() );
    deviceInfos.setPbSlots( m_device->phonebookSlots() );
    deviceInfos.setSMSCenter( m_device->smsc() );

    QValueList<QString> utf8;
    utf8 << "utf8";
    deviceInfos.setCharsets( utf8 );

    return deviceInfos;
}


void kmobiletoolsGammu_engine::dial( DialActions dialActions, const QString &number ) {
    switch( dialActions ) {
        case DIAL_DIAL:
            m_device->dial( number );
            break;

        case DIAL_HANGUP:
            m_device->hangup();
            break;

        default:
            kDebug() << "Gammu engine: dialActions == " << dialActions << endl;
            break;
    }
}


QStringList kmobiletoolsGammu_engine::encodings() {
    QStringList encodings;
    encodings << "utf8";
    return encodings;
}


    /******************************************************
     * Phone information related methods                  *
     ******************************************************/

void kmobiletoolsGammu_engine::pollPhoneStatus() {
    if( m_device->phoneConnected() )
        enqueueJob( new PollStatusJob( m_device, this, this->name() ) );
}


void kmobiletoolsGammu_engine::getPhoneInfos()
{
    if( m_device->phoneConnected() )
        enqueueJob( new PhoneInfosJob( m_device, this, this->name() ) );
}


    /******************************************************
     * SMS related methods                                *
     ******************************************************/

void kmobiletoolsGammu_engine::retrieveSMSList()
{
    enqueueJob( new FetchSMSJob( m_device, this, this->name() ) );
}


void kmobiletoolsGammu_engine::slotDelSMS( SMS* sms ) {
    enqueueJob( new DeleteSMSJob( (GammuSMS*) sms, m_device, this, this->name() ) );
}


void kmobiletoolsGammu_engine::slotStoreSMS( SMS* sms ) {
    enqueueJob( new StoreSMSJob( sms, m_device, this, this->name() ) );
}


void kmobiletoolsGammu_engine::slotStoreSMS( const QString &number,
                                             const QString &text ) {
    enqueueJob( new StoreSMSJob( number, text, m_device, this, this->name() ) );
}


void kmobiletoolsGammu_engine::slotSendSMS( const QString &number,
                                            const QString &text ) {
    enqueueJob( new SendSMSJob( kmobiletoolsJob::sendSMS, number, text,
                                m_device, this, this->name() ) );
}


void kmobiletoolsGammu_engine::slotSendSMS( SMS* sms ) {
    enqueueJob( new SendSMSJob( kmobiletoolsJob::sendSMS, (GammuSMS*) sms,
                                m_device, this, this->name() ) );
}


void kmobiletoolsGammu_engine::slotSendStoredSMS( SMS* sms ) {
    enqueueJob( new SendSMSJob( kmobiletoolsJob::sendStoredSMS, (GammuSMS*) sms,
                                m_device, this, this->name() ) );
}

    /******************************************************
     * Phone book related methods                         *
     ******************************************************/

void kmobiletoolsGammu_engine::retrieveAddressBook() {
    enqueueJob( new FetchAddressBookJob( m_device, this, this->name() ) );
}


void kmobiletoolsGammu_engine::slotAddAddressee( QValueList<KABC::Addressee>* list ) {
    enqueueJob( new EditAddressBookJob
                (  kmobiletoolsJob::addAddressee, list, m_device, this, this->name() )
              );

    enqueueJob( new FetchAddressBookJob( m_device, this, this->name() ) );
}


void kmobiletoolsGammu_engine::slotDelAddressee( QValueList<KABC::Addressee>* list ) {
    enqueueJob( new EditAddressBookJob( kmobiletoolsJob::delAddressee,
                                        list,
                                        m_device,
                                        this,
                                        this->name() ) );

    enqueueJob( new FetchAddressBookJob( m_device, this, this->name() ) );

}


void kmobiletoolsGammu_engine::slotEditAddressee(KABC::Addressee* oldAddressee, KABC::Addressee* newAddressee) {
    enqueueJob( new EditAddressBookJob( oldAddressee,
                                        newAddressee,
                                        m_device,
                                        this,
                                        this->name() ) );

    enqueueJob( new FetchAddressBookJob( m_device, this, this->name() ) );
}


    /******************************************************
     * Calendar related methods                           *
     ******************************************************/

void kmobiletoolsGammu_engine::fetchCalendar() {
    /// @todo implement me
}


/**************************************************************************/


extern "C"
{
    void* init_libkmobiletools_gammu()
    {
      return new kmobiletoolsGammu_engineFactory;
    }
};

kmobiletoolsGammu_engineFactory::kmobiletoolsGammu_engineFactory()
{
}

kmobiletoolsGammu_engineFactory::~kmobiletoolsGammu_engineFactory()
{
}

kmobiletoolsGammu_engine *kmobiletoolsGammu_engineFactory::createObject(QObject *parent, const char *name,
                                            const char *classname, const QStringList &args )
{
    Q_UNUSED(classname)
    Q_UNUSED(args)
    return new kmobiletoolsGammu_engine( parent, name );
}
