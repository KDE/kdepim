/***************************************************************************
   Copyright (C) 2005 by Marco Gulino <marco.gulino@gmail.com>
   Copyright (C) 2006 by Stefan Bogner <bochi@kmobiletools.org>
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

#include "device.h"
#include "kmobiletools_devices.h"

#include <kdebug.h>
#include <qstring.h>


/// @todo write nice encoding and decoding methods for communication with gammu
///       to replace the ugly statements we have so far
/// @todo design and implement real error handling

Device::Device( const char* name )
 : QObject(NULL, name) {
    kDebug() <<"Gammu engine: [!!!!!] device object created.";

    m_isConnected = false;

    // define human-readable error codes
    // Thanks to Igor Palkin (ip821) for the code.
    m_errorCodes.insert( ERR_NONE,                        "No error." );
    m_errorCodes.insert( ERR_DEVICEOPENERROR,             "Error opening device. Unknown/busy or no permissions." );
    m_errorCodes.insert( ERR_DEVICELOCKED,                "Error opening device. Device locked." );
    m_errorCodes.insert( ERR_DEVICENOTEXIST,              "Error opening device. Not exist." );
    m_errorCodes.insert( ERR_DEVICEBUSY,                  "Error opening device. Already opened by "
                                                          "other application." );
    m_errorCodes.insert( ERR_DEVICENOPERMISSION,          "Error opening device. No permissions." );
    m_errorCodes.insert( ERR_DEVICENODRIVER,              "Error opening device. No required driver in "
                                                          "operating system." );
    m_errorCodes.insert( ERR_DEVICENOTWORK,               "Error opening device. Some hardware not connected"
                                                          "/wrong configured." );
    m_errorCodes.insert( ERR_DEVICEDTRRTSERROR,           "Error setting device DTR or RTS." );
    m_errorCodes.insert( ERR_DEVICECHANGESPEEDERROR,      "Error setting device speed. Maybe speed not supported." );
    m_errorCodes.insert( ERR_DEVICEWRITEERROR,            "Error writing device." );
    m_errorCodes.insert( ERR_DEVICEREADERROR,             "Error during reading device" );
    m_errorCodes.insert( ERR_DEVICEPARITYERROR,           "Can't set parity on device" );
    m_errorCodes.insert( ERR_TIMEOUT,                     "No response in specified timeout. Probably "
                                                          "phone not connected." );
    /* Some missed */
    m_errorCodes.insert( ERR_UNKNOWNRESPONSE,             "Unknown response from phone. See readme.txt,"
                                                          " how to report it." );
    /* Some missed */
    m_errorCodes.insert( ERR_UNKNOWNCONNECTIONTYPESTRING, "Unknown connection type string. Check config file." );
    m_errorCodes.insert( ERR_UNKNOWNMODELSTRING,          "Unknown model type string. Check config file." );
    m_errorCodes.insert( ERR_SOURCENOTAVAILABLE,          "Some required functions not compiled for your OS. "
                                                          "Please contact." );
    m_errorCodes.insert( ERR_NOTSUPPORTED,                "Function not supported by phone." );
    m_errorCodes.insert( ERR_EMPTY,                       "Entry is empty" );
    m_errorCodes.insert( ERR_SECURITYERROR,               "Security error. Maybe no PIN ?" );
    m_errorCodes.insert( ERR_INVALIDLOCATION,             "Invalid location. Maybe too high ?" );
    m_errorCodes.insert( ERR_NOTIMPLEMENTED,              "Function not implemented. Help required." );
    m_errorCodes.insert( ERR_FULL,                        "Memory full." );
    m_errorCodes.insert( ERR_UNKNOWN,                     "Unknown error." );
    /* Some missed */
    m_errorCodes.insert( ERR_CANTOPENFILE,                "Can't open specified file. Read only ?" );
    m_errorCodes.insert( ERR_MOREMEMORY,                  "More memory required..." );
    m_errorCodes.insert( ERR_PERMISSION,                  "Permission to file/device required..." );
    m_errorCodes.insert( ERR_EMPTYSMSC,                   "Empty SMSC number. Set in phone or use -smscnumber" );
    m_errorCodes.insert( ERR_INSIDEPHONEMENU,             "You're inside phone menu (during editing ?)."
                                                          "Leave it and try again." );
    m_errorCodes.insert( ERR_WORKINPROGRESS,              "Function is during writing. If want help, please contact "
                                                          "the authors." );
    m_errorCodes.insert( ERR_PHONEOFF,                    "Phone is disabled and connected to charger" );
    m_errorCodes.insert( ERR_FILENOTSUPPORTED,            "File format not supported by Gammu" );
    m_errorCodes.insert( ERR_BUG,                         "Nobody is perfect, some bug appeared in protocol"
                                                          " implementation. Please contact authors." );
    m_errorCodes.insert( ERR_CANCELED,                    "Transfer was canceled by phone (you pressed "
                                                          "cancel on phone?)." );
    /* Some missed */
    m_errorCodes.insert( ERR_OTHERCONNECTIONREQUIRED,     "Current connection type doesn't support called function." );
    m_errorCodes.insert( ERR_WRONGCRC,                    "CRC error." );
    m_errorCodes.insert( ERR_INVALIDDATETIME,             "Invalid date or time specified." );
    m_errorCodes.insert( ERR_MEMORY,                      "Phone memory error, maybe it is read only" );
    m_errorCodes.insert( ERR_INVALIDDATA,                 "Invalid data" );
    m_errorCodes.insert( ERR_FILEALREADYEXIST,            "File with specified name already exist" );
}

Device::~Device() {
}

#include "device.moc"

void Device::initPhone() {
    if( m_isConnected )
        return;

    kDebug() <<"Gammu engine: initialising phone";

    // initialise gammu's state machine
    m_stateMachine.opened = false;

    // find config file (TODO: change to kmobiletools own config dialogs)
    INI_Section *config;
    m_error = GSM_FindGammuRC( &config );
    if (config == NULL) {
        kDebug() <<"Gammu engine: could not find gammu config file";
        return;
    }

    // read config file 
    if ( !GSM_ReadConfig( config, &m_stateMachine.Config[0], 0 ) ) {
        kDebug() <<"Gammu engine: could not open gammu config file";
        return;
    }

    // gammu allows different phone configurations, activate the first one
    // TODO: look for a sensible configuration or provide our own config dialog
    m_stateMachine.ConfigNum = 1;

    // di (Debug_Info) is a global from gammu
    di.dl = DL_TEXTALL;
    di.was_lf = true;

    GSM_SetDebugLevel( m_stateMachine.Config[0].DebugLevel, &di );
    m_error = GSM_SetDebugFile( m_stateMachine.Config[0].DebugFile, &di );


    // connect to the phone
    m_error = GSM_InitConnection( &m_stateMachine, 3 );
    if( m_error != ERR_NONE ) {
        printErrorMessage( m_error, "initPhone" );
        return;
    }

    kDebug() <<"Gammu engine: phone connected!";
    m_isConnected = true;

    // this provides easy access to phone functions
    m_phoneFunctions = m_stateMachine.Phone.Functions;

    emit connected();
}

void Device::terminatePhone()  {
    if( m_isConnected ) {
        m_error = GSM_TerminateConnection( &m_stateMachine );
        printErrorMessage( m_error, "terminatePhone" );

        emit disconnected();
    }
}

void Device::printErrorMessage( int errorCode, const QString& methodName ) {
    kDebug() <<"Gammu engine: [" << methodName <<"()]"
             <<  m_errorCodes[errorCode] << endl;
}

bool Device::phoneConnected() {
    return m_isConnected;
}

QString Device::model() {
    if( !m_isConnected )
        return QString();

    // output cached value if available
    if( !m_model.isEmpty() )
        return m_model;

    m_mutex.lock();
    m_error = m_phoneFunctions->GetModel( &m_stateMachine );
    if( m_error != ERR_NONE ) {
        printErrorMessage( m_error, "model" );
        m_mutex.unlock();
        return QString();
    }
    m_model = m_stateMachine.Phone.Data.ModelInfo->model;
    m_mutex.unlock();

    return m_model;
}

QString Device::manufacturer() {
    if( !m_isConnected )
        return QString();

    // output cached value if available
    if( !m_manufacturer.isEmpty() )
        return m_manufacturer;

    m_mutex.lock();
    m_error = m_phoneFunctions->GetManufacturer( &m_stateMachine );
    if( m_error != ERR_NONE ) {
        printErrorMessage( m_error, "manufacturer" );
        m_mutex.unlock();
        return QString();
    }
    m_manufacturer = m_stateMachine.Phone.Data.Manufacturer;
    m_mutex.unlock();

    return m_manufacturer;
}

QString Device::version() {
    if( !m_isConnected )
        return QString();

    // output cached value if available
    if( !m_version.isEmpty() )
        return m_version;

    m_mutex.lock();
    m_error = m_phoneFunctions->GetFirmware( &m_stateMachine );
    if( m_error != ERR_NONE ) {
        printErrorMessage( m_error, "version" );
        m_mutex.unlock();
        return QString();
    }
    m_version = m_stateMachine.Phone.Data.Version;
    m_mutex.unlock();

    return m_version;
}

QString Device::imei() {
    if( !m_isConnected )
        return QString();

    // output cached value if available
    if( !m_imei.isEmpty() )
        return m_imei;

    m_mutex.lock();
    m_error = m_phoneFunctions->GetIMEI( &m_stateMachine );
    if( m_error != ERR_NONE ) {
        printErrorMessage( m_error, "imei" );
        m_mutex.unlock();
        return QString();
    }
    m_imei = m_stateMachine.Phone.Data.IMEI;
    m_mutex.unlock();

    return m_imei;
}

QString Device::smsc() {
    if( !m_isConnected )
        return QString();

    GSM_SMSC smsc;
    QString smscNumber;

    m_mutex.lock();
    m_error = m_phoneFunctions->GetSMSC( &m_stateMachine, &smsc );
    if( m_error != ERR_NONE ) {
        printErrorMessage( m_error, "smsc" );
        m_mutex.unlock();
        return QString();
    }
    const char* c = (const char*) GetMsg( m_stateMachine.msg,
                                          DecodeUnicodeString( smsc.Number ) );
    m_mutex.unlock();

    smscNumber = QString::fromUtf8( c );

    return smscNumber;
}

int Device::battery() {
    if( !m_isConnected )
        return 0;

    GSM_BatteryCharge batteryCharge;
    int batteryStatus = 0;

    m_mutex.lock();
    m_error = m_phoneFunctions->GetBatteryCharge( &m_stateMachine, &batteryCharge );
    if( m_error != ERR_NONE ) {
        printErrorMessage( m_error, "battery" );
        m_mutex.unlock();
        return 0;
    }
    batteryStatus = batteryCharge.BatteryPercent;
    m_mutex.unlock();

    return batteryStatus;
}

QString Device::networkName() {
    if( !m_isConnected )
        return QString();

    GSM_NetworkInfo networkInfo;
    QString networkName;

    m_mutex.lock();
    m_error = m_phoneFunctions->GetNetworkInfo( &m_stateMachine, &networkInfo );
    if( m_error != ERR_NONE ) {
        printErrorMessage( m_error, "networkName" );
        m_mutex.unlock();
        return QString();
    }
    char* buffer = GSM_GetNetworkName( networkInfo.NetworkCode );
    networkName = QString::fromUtf8( (const char*)(DecodeUnicodeString( (unsigned char*) buffer )) );
    m_mutex.unlock();

    return networkName;
}

int Device::signalQuality() {
    if( !m_isConnected )
        return 0;

    GSM_SignalQuality signalQuality;
    int signal = 0;

    m_mutex.lock();
    m_error = m_phoneFunctions->GetSignalQuality( &m_stateMachine, &signalQuality );
    if( m_error != ERR_NONE ) {
        printErrorMessage( m_error, "signalQuality" );
        m_mutex.unlock();
        return 0;
    }
    signal = signalQuality.SignalPercent;
    m_mutex.unlock();

    return signal;
}

int Device::unreadSMS() {
    if( !m_isConnected )
        return 0;

    GSM_SMSMemoryStatus smsMemoryStatus;
    int unreadSMS = 0;

    m_mutex.lock();
    m_error = m_phoneFunctions->GetSMSStatus( &m_stateMachine, &smsMemoryStatus );
    if( m_error != ERR_NONE ) {
        printErrorMessage( m_error, "unreadSMS" );
        m_mutex.unlock();
        return 0;
    }

    if( smsMemoryStatus.SIMSize > 0 )
        unreadSMS += smsMemoryStatus.SIMUnRead;

    if( smsMemoryStatus.PhoneSize > 0 )
        unreadSMS += smsMemoryStatus.PhoneUnRead;
    m_mutex.unlock();

    return unreadSMS;
}

int Device::totalSMS() {
    if( !m_isConnected )
        return 0;

    GSM_SMSMemoryStatus smsMemoryStatus;
    int totalSMS = 0;

    m_mutex.lock();
    m_error = m_phoneFunctions->GetSMSStatus( &m_stateMachine, &smsMemoryStatus );
    if( m_error != ERR_NONE ) {
        printErrorMessage( m_error, "totalSMS" );
        m_mutex.unlock();
        return 0;
    }
    if( smsMemoryStatus.SIMSize > 0 )
        totalSMS += smsMemoryStatus.SIMUsed;

    if( smsMemoryStatus.PhoneSize > 0 )
        totalSMS += smsMemoryStatus.PhoneUsed;

    m_mutex.unlock();

    return totalSMS;
}


bool Device::ringing() {
    /* GetDisplayStatus is the only way I found to figure out whether or not the phone
       is ringing. Unfortunately it is unsupported by my phone, so I can't test it... 

       matlec: can't test it either.. TODO: check if it's better to use
       SetIncomingCall() and monitor actively for incoming connections.
       Don't know if this is supported by more phones or if it's just the same internally.
    */
    if( !m_isConnected )
        return false;


    GSM_DisplayFeatures displayFeatures;
    bool ringing = false;

    m_mutex.lock();
    m_error = m_phoneFunctions->GetDisplayStatus( &m_stateMachine, &displayFeatures );
    if( m_error == ERR_NONE ) {
        // loop through the phone's features and look for active call
        for ( int i=0; i<displayFeatures.Number; i++ ) {
            if( displayFeatures.Feature[i] == GSM_CallActive )
                ringing = true;
        }
    } else {
        printErrorMessage( m_error, "ringing" );
    }
    m_mutex.unlock();

    return ringing;
}

QValueList<QString> Device::phonebookSlots() {
    if( !m_isConnected )
        return QValueList<QString>();

    if( !m_phonebookSlots.isEmpty() )
        return m_phonebookSlots;

    // the idea is to read the first available phonebook entry of both ME and SM slot
    // If this is successful, let's assume the slot is supported
    GSM_MemoryEntry memoryEntry;

    m_mutex.lock();

    // test slot ME
    memoryEntry.MemoryType = MEM_ME;
    m_error = m_phoneFunctions->GetNextMemory( &m_stateMachine, &memoryEntry, true );
    if( m_error == ERR_NONE )
        m_phonebookSlots << "ME";

    // test slot SM
    memoryEntry.MemoryType = MEM_SM;
    m_error = m_phoneFunctions->GetNextMemory( &m_stateMachine, &memoryEntry, true );
    if( m_error == ERR_NONE )
        m_phonebookSlots << "SM";

    m_mutex.unlock();

    return m_phonebookSlots;
}

KABC::Addressee::List Device::phonebook() {
    if( !m_isConnected )
        return KABC::Addressee::List();

    GSM_MemoryEntry memoryEntry;
    QValueList<KABC::Addressee> phonebook;

    m_mutex.lock();

    // let's check out the phone internal phonebook first (slot ME)
    memoryEntry.MemoryType = MEM_ME;

    m_error = ERR_NONE;

    kDebug() <<"Gammu engine: start reading ME slot";

    bool start = true;
    while( m_error == ERR_NONE ) {
        m_error = m_phoneFunctions->GetNextMemory( &m_stateMachine, &memoryEntry, start );
        if( m_error == ERR_NONE ) {
            phonebook.push_back( toKAbc( memoryEntry ) );
            KABC::Addressee& currentAddressee = phonebook.last();
            kDebug() <<"memory index:" << memoryEntry.Location;
            currentAddressee.insertCustom( "KMobileTools", "index", 
                                           QString::number( memoryEntry.Location ) );
            currentAddressee.insertCustom( "KMobileTools", "memslot",
                                          QString::number( memoryEntry.MemoryType ) );
        }

        start = false;
    }

    kDebug() <<"Gammu engine: finished reading ME slot";
    kDebug() <<"Gammu engine: start reading SM slot";
    // now check out the sim phonebook (slot SM)
    memoryEntry.MemoryType = MEM_SM;

    m_error = ERR_NONE;
    start = true;
    while( m_error == ERR_NONE ) {
        m_error = m_phoneFunctions->GetNextMemory( &m_stateMachine, &memoryEntry, start );
        if( m_error == ERR_NONE ) {
            phonebook.push_back( toKAbc( memoryEntry ) );
            KABC::Addressee& currentAddressee = phonebook.last();
            kDebug() <<"memory index:" << memoryEntry.Location;
            currentAddressee.insertCustom( "KMobileTools", "index", 
                                           QString::number( memoryEntry.Location ) );
            currentAddressee.insertCustom( "KMobileTools", "memslot",
                                          QString::number( memoryEntry.MemoryType ) );
        }

        start = false;
    }

    kDebug() <<"Gammu engine: finished reading SM slot";

    m_mutex.unlock();

    return phonebook;
}


void Device::addAddressee( KABC::Addressee::List *addresseeList ) {
    if( !m_isConnected )
        return;

    GSM_MemoryEntry memoryEntry;

    QValueListIterator<KABC::Addressee> it = addresseeList->begin();
    while( it != addresseeList->end() ) {
        m_mutex.lock();
        memoryEntry = toMemoryEntry( &(*it) );
        /// @todo add fail-safe implementation if addMemory is not implemented in gammu's phone protocol
        memoryEntry.MemoryType = (GSM_MemoryType) (*it).custom( "KMobileTools", "memslot" ).toInt();

        m_error = m_phoneFunctions->AddMemory( &m_stateMachine, &memoryEntry );

        m_mutex.unlock();

        it++;

        if( m_error != ERR_NONE )
            printErrorMessage( m_error, "addAddressee" );
    }

    return;

}

void Device::editAddressee( KABC::Addressee* oldAddressee,
                            KABC::Addressee* newAddressee ) {
    if( !m_isConnected )
        return;

    GSM_MemoryEntry newMemoryEntry;

    m_mutex.lock();

    newMemoryEntry = toMemoryEntry( newAddressee );
    newMemoryEntry.Location = oldAddressee->custom( "KMobileTools", "index" ).toInt();
    newMemoryEntry.MemoryType = (GSM_MemoryType) oldAddressee->custom( "KMobileTools", "memslot" ).toInt();

    m_error = m_phoneFunctions->SetMemory( &m_stateMachine, &newMemoryEntry );

    if( m_error != ERR_NONE )
        printErrorMessage( m_error, "editAddressee" );

    m_mutex.unlock();

    return;
}

void Device::removeAddressee( KABC::Addressee::List *addresseeList ) {
    if( !m_isConnected )
        return;

    GSM_MemoryEntry memoryEntry;

    QValueListIterator<KABC::Addressee> it = addresseeList->begin();
    while( it != addresseeList->end() ) {
        m_mutex.lock();
        memoryEntry = toMemoryEntry( &(*it) );
        memoryEntry.Location = (*it).custom( "KMobileTools", "index" ).toInt();
        memoryEntry.MemoryType = (GSM_MemoryType) (*it).custom( "KMobileTools", "memslot" ).toInt();

        m_error = m_phoneFunctions->DeleteMemory( &m_stateMachine, &memoryEntry );

        m_mutex.unlock();

        it++;

        if( m_error != ERR_NONE )
            printErrorMessage( m_error, "removeAddressee" );
    }

    return;
}

bool Device::dial( const QString& number ) {
    if( !m_isConnected )
        return false;

    GSM_CallShowNumber showNumber = GSM_CALL_DefaultNumberPresence;

    m_mutex.lock();
    m_error = m_phoneFunctions->DialVoice( &m_stateMachine, (char*) number.data(),
                                           showNumber );
    if( m_error != ERR_NONE ) {
        printErrorMessage( m_error, "dial" );
        m_mutex.unlock();
        return false;
    }
    m_mutex.unlock();

    return true;
}

bool Device::hangup() {
    if( !m_isConnected )
        return false;

    m_mutex.lock();
    m_error = m_phoneFunctions->CancelCall( &m_stateMachine, 0, true );
    if( m_error != ERR_NONE ) {
        printErrorMessage( m_error, "hangup" );
        m_mutex.unlock();
        return false;
    }
    m_mutex.unlock();

    return true;
}


KABC::Addressee Device::toKAbc( const GSM_MemoryEntry& memoryEntry ) {
    KABC::Addressee addressee;
    KABC::Address address( KABC::Address::Postal );

    const GSM_SubMemoryEntry *entry;

    // iterate over the entry's fields
    for( int i=0; i<memoryEntry.EntriesNum; i++ ) {
        // for convenience we use a subentry here to avoid ugly constructs
        entry = &(memoryEntry.Entries[i]);
	switch ( entry->EntryType ) {
	    case PBK_Text_Name:
                addressee.setFormattedName( QString::fromUtf8( (char*)DecodeUnicodeString( entry->Text ) ) );
                addressee.setFamilyName( QString::fromUtf8((char*)DecodeUnicodeString( entry->Text ) ) );
                break;

            case PBK_Number_General:
                addressee.insertPhoneNumber( KABC::PhoneNumber( QString::fromUtf8( (char*)DecodeUnicodeString( entry->Text ) ),
                                             KABC::PhoneNumber::Pref ) );
                addressee.insertPhoneNumber( KABC::PhoneNumber( QString::fromUtf8( (char*)DecodeUnicodeString( entry->Text ) ) ,
                                             KABC::PhoneNumber::Home  ) );
                break;

            case PBK_Category:
                addressee.insertCategory( QString::fromUtf8((char*)DecodeUnicodeString( entry->Text )) );
                break;

            case PBK_Private:
                addressee.insertCustom( "KMobileTools", "private", QString::fromUtf8((char*)DecodeUnicodeString(entry->Text)) );
                break;

            case PBK_Number_Mobile:
                addressee.insertPhoneNumber( KABC::PhoneNumber( QString::fromUtf8((char*)DecodeUnicodeString( entry->Text )),
                                             KABC::PhoneNumber::Cell ) );
                break;

            case PBK_Number_Work:
                addressee.insertPhoneNumber( KABC::PhoneNumber( QString::fromUtf8((char*)DecodeUnicodeString( entry->Text )), KABC::PhoneNumber::Work ) );
                break;

            case PBK_Number_Fax:
                addressee.insertPhoneNumber( KABC::PhoneNumber( QString::fromUtf8((char*)DecodeUnicodeString( entry->Text )), KABC::PhoneNumber::Fax ) );
                break;

            case PBK_Number_Home:
                addressee.insertPhoneNumber( KABC::PhoneNumber( QString::fromUtf8((char*)DecodeUnicodeString( entry->Text )), KABC::PhoneNumber::Home ) );
                break;

            case PBK_Number_Pager:
                addressee.insertPhoneNumber( KABC::PhoneNumber( QString::fromUtf8((char*)DecodeUnicodeString( entry->Text )), KABC::PhoneNumber::Pager ) );
                break;

            // using PhoneNumber::Voice for PBK_Number_Other
            case PBK_Number_Other:
                addressee.insertPhoneNumber( KABC::PhoneNumber( QString::fromUtf8((char*)DecodeUnicodeString( entry->Text )), KABC::PhoneNumber::Voice ) );
                break;

            case PBK_Text_Note:
                addressee.setNote( QString::fromUtf8((char*)DecodeUnicodeString( entry->Text )) );
                break;

            case PBK_Text_Email:
                addressee.insertEmail( QString::fromUtf8((char*)DecodeUnicodeString(entry->Text)) );
                break;

            case PBK_Text_Email2:
                addressee.insertEmail( QString::fromUtf8((char*)DecodeUnicodeString(entry->Text)) );
                break;

            case PBK_Text_URL:
                addressee.setUrl( KURL( QString::fromUtf8((char*)DecodeUnicodeString(entry->Text) )) );
                break;

            case PBK_Text_LastName:
                addressee.setFamilyName( QString::fromUtf8((char*)DecodeUnicodeString( entry->Text )) );
                break;

            case PBK_Text_FirstName:
                addressee.setGivenName( QString::fromUtf8((char*)DecodeUnicodeString( entry->Text )) );
                break;

            case PBK_Text_Company:
                addressee.setOrganization( QString::fromUtf8((char*)DecodeUnicodeString( entry->Text )) );
                break;

            case PBK_Text_JobTitle:
                addressee.setTitle( QString::fromUtf8((char*)DecodeUnicodeString( entry->Text )) );
                break;

            case PBK_Text_StreetAddress:
                address.setStreet( QString::fromUtf8((char*)DecodeUnicodeString( entry->Text )) );
                break;

            case PBK_Text_City:
                address.setLocality( QString::fromUtf8((char*)DecodeUnicodeString( entry->Text )) );
                break;

            case PBK_Text_State:
                address.setRegion( QString::fromUtf8((char*)DecodeUnicodeString( entry->Text )) );
                break;

            case PBK_Text_Zip:
                address.setPostalCode( QString::fromUtf8((char*)DecodeUnicodeString( entry->Text )) );
                break;

            case PBK_Text_Country:
                address.setCountry( QString::fromUtf8((char*)DecodeUnicodeString( entry->Text )) );
                break;

            case PBK_Text_UserID:
                addressee.setUid( QString::fromUtf8((char*)DecodeUnicodeString( entry->Text )) );
                break;

            case PBK_Text_Custom1:
                addressee.insertCustom("KMobileTools", "custom_1", QString::fromUtf8(((char*)DecodeUnicodeString(entry->Text)) ) );
                break;

            case PBK_Text_Custom2:
                addressee.insertCustom("KMobileTools", "custom_2", QString::fromUtf8((char*)DecodeUnicodeString(entry->Text)) );
                break;

            case PBK_Text_Custom3:
                addressee.insertCustom("KMobileTools", "custom_3", QString::fromUtf8((char*)DecodeUnicodeString(entry->Text)) );
                break;

            case PBK_Text_Custom4:
                addressee.insertCustom("KMobileTools", "custom_4", QString::fromUtf8((char*)DecodeUnicodeString(entry->Text)) );
                break;

            case PBK_Caller_Group:
                addressee.insertCustom("KMobileTools", "caller_grp", QString::fromUtf8((char*)DecodeUnicodeString(entry->Text)) );
                break;

            case PBK_RingtoneID:
                addressee.insertCustom("KMobileTools", "ringtone_id", QString::fromUtf8((char*)DecodeUnicodeString(entry->Text)) );
                break;

            case PBK_RingtoneFileSystemID:
                addressee.insertCustom("KMobileTools", "ringtone_fs_id", QString::fromUtf8((char*)DecodeUnicodeString(entry->Text)) );
                break;

            case PBK_SMSListID:
                addressee.insertCustom("KMobileTools", "sms_list_id", QString::fromUtf8((char*)DecodeUnicodeString(entry->Text)) );
                break;

            case PBK_PictureID:
                addressee.insertCustom("KMobileTools", "picture_id", QString::fromUtf8((char*)DecodeUnicodeString(entry->Text)) );
                break;

            case PBK_Text_Postal:
                addressee.insertCustom("KMobileTools", "text_postal", QString::fromUtf8((char*)DecodeUnicodeString(entry->Text)) );
                break;

            case PBK_Date:
                break;

            case PBK_CallLength:
                break;

            case PBK_Text_LUID:
                break;
	}
    }

    if( !address.isEmpty() )
        addressee.insertAddress( address );

    return addressee;
}

GSM_MemoryEntry Device::toMemoryEntry( KABC::Addressee* addressee ) {
    GSM_MemoryEntry memoryEntry;
    memoryEntry.EntriesNum = 1;

    if( !addressee->formattedName().isEmpty() )
    {
        kDebug() <<"[ !!! ] !addressee->formattedName().isEmpty()";
        EncodeUnicode( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, addressee->formattedName().utf8(), addressee->formattedName().utf8().length() );
        memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Text_Name;
        memoryEntry.EntriesNum++;
    }

    if( !addressee->familyName().isEmpty() )
    {
        kDebug() <<"[ !!! ] !addressee->familyName().isEmpty()";
        memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Text_LastName;
        EncodeUnicode( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, addressee->familyName().utf8(), addressee->familyName().utf8().length() );
        memoryEntry.EntriesNum++;
    }

    if( !addressee->givenName().isEmpty() )
    {
        kDebug() <<"[ !!! ] !addressee->givenName().isEmpty()";
        memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Text_FirstName;
        EncodeUnicode( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, addressee->givenName().utf8(), addressee->givenName().utf8().length() );
        memoryEntry.EntriesNum++;
    }

    if( !addressee->categories().isEmpty() ) 
    {
        kDebug() <<"[ !!! ] !addressee->categories().isEmpty()";
        QStringList list( addressee->categories() );

        for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
            QString tmp = *it;
            memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Category;
            memcpy( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, tmp.utf8(), tmp.utf8().length() );
            memoryEntry.EntriesNum++;
        }
    }

    if( !addressee->phoneNumbers().isEmpty() )
    {
        kDebug() <<"[ !!! ] !addressee->phoneNumbers().isEmpty()";
        QValueList<KABC::PhoneNumber> phonenumbers=addressee->phoneNumbers();
        QValueList<KABC::PhoneNumber>::iterator it;

        for ( it = phonenumbers.begin(); it != phonenumbers.end(); ++it )
        {
            switch( (*it).type() ) {

                case KABC::PhoneNumber::Work:
                    kDebug() <<"[ !!! ] Adding KABC::PhoneNumber::Work";
                    memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Number_Work;
                    EncodeUnicode( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, (*it).number().utf8(), (*it).number().utf8().length() );
                    memoryEntry.EntriesNum++;
                break;

                case KABC::PhoneNumber::Home:
                    kDebug() <<"[ !!! ] Adding KABC::PhoneNumber::Home";
                    memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Number_Home;
                    EncodeUnicode( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, (*it).number().utf8(), (*it).number().utf8().length() );
                    memoryEntry.EntriesNum++;
                break;

                case KABC::PhoneNumber::Pref:
                    kDebug() <<"[ !!! ] Adding KABC::PhoneNumber::Pref";
                    memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Number_Work;
                    EncodeUnicode( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, (*it).number().utf8(), (*it).number().utf8().length() );
                    memoryEntry.EntriesNum++;
                break;

                case KABC::PhoneNumber::Cell:
                    kDebug() <<"[ !!! ] Adding KABC::PhoneNumber::Mobile";
                    memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Number_Mobile;
                    EncodeUnicode( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, (*it).number().utf8(), (*it).number().utf8().length() );
                    memoryEntry.EntriesNum++;
                break;

                case KABC::PhoneNumber::Fax:
                    kDebug() <<"[ !!! ] Adding KABC::PhoneNumber::Fax";
                    memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Number_Fax;
                    EncodeUnicode( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, (*it).number().utf8(), (*it).number().utf8().length() );
                    memoryEntry.EntriesNum++;
                break;

                case KABC::PhoneNumber::Pager:
                    kDebug() <<"[ !!! ] Adding KABC::PhoneNumber::Pager";
                    memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Number_Pager;
                    EncodeUnicode( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, (*it).number().utf8(), (*it).number().utf8().length() );
                    memoryEntry.EntriesNum++;
                break;

            }

        }
    }

    if( !addressee->note().isEmpty() ) 
    {
        kDebug() <<"[ !!! ] !addressee->note().isEmpty()";
        memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Text_Note;
        memcpy( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, addressee->note().utf8(), addressee->note().utf8().length() );
        memoryEntry.EntriesNum++;
    }

    if( !addressee->url().isEmpty() ) 
    {
        kDebug() <<"[ !!! ] !addressee->url().isEmpty()";
        memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Text_URL;
        memcpy( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, addressee->url().url().utf8(), addressee->url().url().utf8().length() );
        memoryEntry.EntriesNum++;
    }

    if( !addressee->organization().isEmpty() ) 
    {
        kDebug() <<"[ !!! ] !addressee->organization().isEmpty()";
        memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Text_Company;
        memcpy( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, addressee->organization().utf8(), addressee->organization().utf8().length() );
        memoryEntry.EntriesNum++;
    }

    if( !addressee->title().isEmpty() ) 
    {
        kDebug() <<"[ !!! ] !addressee->title().isEmpty()";
        memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Text_JobTitle;
        memcpy( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, addressee->title().utf8(), addressee->title().utf8().length() );
        memoryEntry.EntriesNum++;
    }

    //hmm uid never seems to be empty...
    //if( !addressee->uid().isEmpty() ) 
    //{
    //    kDebug() <<"[ !!! ] !addressee->uid().isEmpty()";
    //    memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Text_UserID;
    //    EncodeUnicode( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, (const unsigned char*)addressee->uid().utf8(), //addressee->uid().utf8().length() );
    //    memoryEntry.EntriesNum++;
    //}

    if( !addressee->custom("KMobileTools", "custom_1" ).isEmpty() ) 
    {
        kDebug() <<"[ !!! ] !custom_1.isEmpty()";
        memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Text_Custom1;
        memcpy( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, addressee->custom("KMobileTools", "custom_1" ).utf8(), addressee->custom("KMobileTools", "custom_1" ).utf8().length() );
        memoryEntry.EntriesNum++;
    }

    if( !addressee->custom("KMobileTools", "custom_2" ).isEmpty() ) 
    {
        kDebug() <<"[ !!! ] !custom_2.isEmpty()";
        memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Text_Custom2;
        memcpy( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, addressee->custom("KMobileTools", "custom_2" ).utf8(), addressee->custom("KMobileTools", "custom_2" ).utf8().length() );
        memoryEntry.EntriesNum++;
    }

    if( !addressee->custom("KMobileTools", "custom_3" ).isEmpty() ) 
    {
        kDebug() <<"[ !!! ] !custom_3.isEmpty()";
        memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Text_Custom3;
        memcpy( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, addressee->custom("KMobileTools", "custom_3" ).utf8(), addressee->custom("KMobileTools", "custom_3" ).utf8().length() );
        memoryEntry.EntriesNum++;
    }

    if( !addressee->custom("KMobileTools", "custom_4" ).isEmpty() ) 
    {
        kDebug() <<"[ !!! ] !custom_4.isEmpty()";
        memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Text_Custom4;
        memcpy( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, addressee->custom("KMobileTools", "custom_4" ).utf8(), addressee->custom("KMobileTools", "custom_4" ).utf8().length() );
        memoryEntry.EntriesNum++;
    }

    if( !addressee->custom("KMobileTools", "caller_grp" ).isEmpty() ) 
    {
        kDebug() <<"[ !!! ] !caller_grp.isEmpty()";
        memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Caller_Group;
        memcpy( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, addressee->custom("KMobileTools", "caller_grp" ).utf8(), addressee->custom("KMobileTools", "caller_grp" ).utf8().length() );
        memoryEntry.EntriesNum++;
    }

    if( !addressee->custom("KMobileTools", "ringtone_id" ).isEmpty() ) 
    {
        kDebug() <<"[ !!! ] !ringtone_id.isEmpty()";
        memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_RingtoneID;
        memcpy( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, addressee->custom("KMobileTools", "ringtone_id" ).utf8(), addressee->custom("KMobileTools", "ringtone_id" ).utf8().length() );
        memoryEntry.EntriesNum++;
    }

    if( !addressee->custom("KMobileTools", "ringtone_fs_id" ).isEmpty() ) 
    {
        kDebug() <<"[ !!! ] !ringtone_fs_id.isEmpty()";
        memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_RingtoneFileSystemID;
        memcpy( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, addressee->custom("KMobileTools", "ringtone_fs_id" ).utf8(), addressee->custom("KMobileTools", "ringtone_fs_id" ).utf8().length() );
        memoryEntry.EntriesNum++;
    }

    if( !addressee->custom("KMobileTools", "sms_list_id" ).isEmpty() ) 
    {
        kDebug() <<"[ !!! ] !sms_list_id.isEmpty()";
        memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_SMSListID;
        memcpy( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, addressee->custom("KMobileTools", "sms_list_id" ).utf8(), addressee->custom("KMobileTools", "sms_list_id" ).utf8().length() );
        memoryEntry.EntriesNum++;
    }

    if( !addressee->custom("KMobileTools", "picture_id" ).isEmpty() ) 
    {
        kDebug() <<"[ !!! ] !picture_id.isEmpty()";
        memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_PictureID;
        memcpy( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, addressee->custom("KMobileTools", "picture_id" ).utf8(), addressee->custom("KMobileTools", "picture_id" ).utf8().length() );
        memoryEntry.EntriesNum++;
    }

    if( !addressee->custom("KMobileTools", "text_postal" ).isEmpty() ) 
    {
        kDebug() <<"[ !!! ] !text_postal.isEmpty()";
        memoryEntry.Entries[ memoryEntry.EntriesNum ].EntryType = PBK_Text_Postal;
        memcpy( memoryEntry.Entries[ memoryEntry.EntriesNum ].Text, addressee->custom("KMobileTools", "text_postal" ).utf8(), addressee->custom("KMobileTools", "text_postal" ).utf8().length() );
        memoryEntry.EntriesNum++;
    }

    return memoryEntry;
}

SMSList* Device::smsList() {
    if( !m_isConnected )
        return 0;

    GSM_SMSFolders folders;
    GSM_MultiSMSMessage sms;
    SMSList *smsList = new SMSList();

    m_mutex.lock();
    m_error = m_phoneFunctions->GetSMSFolders( &m_stateMachine, &folders );
    if( m_error != ERR_NONE ) {
        printErrorMessage( m_error, "smsList" );
        m_mutex.unlock();
        delete smsList;
        return 0;
    }

    bool start = true;
    while ( m_error == ERR_NONE ) {
        sms.SMS[0].Folder=0x00;
        m_error= m_phoneFunctions->GetNextSMS( &m_stateMachine, &sms, start );
        switch ( m_error ) {
            case ERR_EMPTY:
                break;

            default:
                GammuSMS *kmobileSMS = new GammuSMS;

                // set slot
                switch( sms.SMS[0].Memory ) {
                    case MEM_ME:
                        kmobileSMS->setSlot( SMS::Phone );
                        break;
                    case MEM_SM:
                        kmobileSMS->setSlot( SMS::SIM );
                        break;
                    default:
                        kDebug() <<"Gammu engine: sms stored using unknown memory slot";
                        kmobileSMS->setSlot( SMS::Phone );
                        break;
                }

                // set type
                switch ( sms.SMS[0].State ) {
                    case SMS_Sent:
                        kmobileSMS->setType( SMS::Sent );
                        break;
                    case SMS_Read:
                        kmobileSMS->setType( SMS::Read );
                        break;
                    case SMS_UnRead:
                        kmobileSMS->setType( SMS::Unread );
                    case SMS_UnSent:
                        kmobileSMS->setType( SMS::Unsent );
                }

                // set date and time
                QDate date( sms.SMS[0].DateTime.Year,
                            sms.SMS[0].DateTime.Month,
                            sms.SMS[0].DateTime.Day );

                QTime time( sms.SMS[0].DateTime.Hour,
                            sms.SMS[0].DateTime.Minute,
                            sms.SMS[0].DateTime.Second );

                kmobileSMS->setDateTime( KDateTime( date, time ) );

                // join multi-part sms and set text
                QString text;
                for ( int j=0; j<sms.Number; j++) {
                    text += QString::fromUtf8( (const char*) DecodeUnicodeString( sms.SMS[j].Text ) );
                }

                kmobileSMS->setText( text );

                // set phone number
                QStringList number;
                number << QString::fromUtf8( (const char*) 
                          DecodeUnicodeString( (const unsigned char*) sms.SMS[0].Number ) );
                kmobileSMS->setNumbers( number );

                // set folder and location
                kmobileSMS->setFolder( sms.SMS[0].Folder );
                kmobileSMS->setLocation( sms.SMS[0].Location );

                kDebug() <<"=========================";
                kDebug() <<"Number:" << number[0];
                kDebug() <<"Date:" << KDateTime( date, time );
                kDebug() <<"Slot:" << kmobileSMS->slot();
                kDebug() <<"Folder:" << kmobileSMS->folder();
                kDebug() <<"Location:" << kmobileSMS->location();
                kDebug() << text;
                kDebug() <<"=========================";

                if ( smsList->find( kmobileSMS->uid() ) >= 0 )
                    break;

                smsList->append( kmobileSMS );
                break;
        }

        start=false;
    }

    m_mutex.unlock();

    return smsList;
}

void Device::deleteSMS( GammuSMS *sms ) {
    if( !m_isConnected )
        return;

    GSM_SMSMessage _sms;

    m_mutex.lock();
    _sms.Folder = sms->folder();
    _sms.Location = sms->location();

    m_error = m_phoneFunctions->DeleteSMS( &m_stateMachine, &_sms );
    if( m_error != ERR_NONE ) {
        printErrorMessage( m_error, "deleteSMS" );
        m_mutex.unlock();
        return;
    }

    m_mutex.unlock();

    return;
}

void Device::storeSMS( SMS *sms ) {
    if( !m_isConnected )
        return;

    GSM_MultiSMSMessage _sms = composeSMS( sms );
    if( m_error != ERR_NONE )
        return;

    m_mutex.lock();
    for( int i=0; i<_sms.Number; i++ ) {
        m_error = m_phoneFunctions->AddSMS( &m_stateMachine, &_sms.SMS[i]);
        printErrorMessage( m_error, "AddSMS" );
    }
    m_mutex.unlock();

    return;
}

void Device::sendStoredSMS( GammuSMS *sms ) {
    if( !m_isConnected )
        return;

    m_mutex.lock();

    m_error = m_phoneFunctions->SendSavedSMS( &m_stateMachine, sms->folder(), sms->location() );
    if( m_error != ERR_NONE ) {
        printErrorMessage( m_error, "SendSavedSMS" );
        m_mutex.unlock();
        return;
    }

    m_mutex.unlock();

    return;
}

void Device::sendSMS( SMS *sms ) {
    if( !m_isConnected )
        return;

    GSM_MultiSMSMessage _sms = composeSMS( sms );
    if( m_error != ERR_NONE )
        return;

    m_mutex.lock();
    for( int i=0; i<_sms.Number; i++ ) {
        m_error = m_phoneFunctions->SendSMS( &m_stateMachine, &_sms.SMS[i]);
        printErrorMessage( m_error, "SendSMS" );
    }
    m_mutex.unlock();

    return;
}

GSM_MultiSMSMessage Device::composeSMS( SMS *sms ) {
    GSM_MultiSMSMessage _sms;
    GSM_SMSFolders folders;
    GSM_MultiPartSMSInfo smsInfo;

    m_mutex.lock();

    m_error = m_phoneFunctions->GetSMSFolders( &m_stateMachine, &folders );
    if( m_error != ERR_NONE ) {
        printErrorMessage( m_error, "GetSMSFolders" );
        m_mutex.unlock();
        return _sms;
    }

    smsInfo.EntriesNum = 1;

    // check if we need multi-part sms (len >160 chars)
    if( sms->getText().length() > 160 )
        smsInfo.Entries[0].ID = SMS_ConcatenatedTextLong;
    else
        smsInfo.Entries[0].ID = SMS_Text;

    unsigned char buffer[1000];
    EncodeUnicode( buffer, sms->getText().utf8(), sms->getText().utf8().length() );
    smsInfo.Entries[0].Buffer = &buffer[0];

    smsInfo.UnicodeCoding = true;

    m_error = GSM_EncodeMultiPartSMS( &smsInfo, &_sms );
    if( m_error != ERR_NONE ) {
        printErrorMessage( m_error, "GSM_EncodeMultiPartSMS" );
        m_mutex.unlock();
        return _sms;
    }

    GSM_SMSC smsc;
    smsc.Location = 0x01;

    m_error = m_phoneFunctions->GetSMSC( &m_stateMachine, &smsc );
    if( m_error != ERR_NONE ) {
        printErrorMessage( m_error, "GetSMSC" );
        m_mutex.unlock();
        return _sms;
    }

    for( int i=0; i<_sms.Number; i++ ) {
        /// @todo how to determine which folder number "outgoing" is?!?!
        /// in worst case we need to let the user decide upon sms folder list
        _sms.SMS[i].Folder = 13;
        _sms.SMS[i].PDU = SMS_Submit;
        _sms.SMS[i].SMSC = smsc;
        EncodeUnicode( _sms.SMS[i].Number, sms->getTo()[0].utf8(), sms->getTo()[0].utf8().length() );

        // set further recipient numbers if available
        int numberRecipients = sms->getTo().count();
        if( numberRecipients > 1 ) {
            _sms.SMS[i].OtherNumbersNum = numberRecipients - 1;
            for( int contactNumber=1; contactNumber < numberRecipients; contactNumber++ ) {
                EncodeUnicode( _sms.SMS[i].Number, sms->getTo()[contactNumber].utf8(),
                               sms->getTo()[contactNumber].utf8().length() );
            }
        }
    }

    m_mutex.unlock();

    return _sms;
}
