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

#ifndef DEVICE_H
#define DEVICE_H

#include <qobject.h>
#include <qmutex.h>
#include <qmap.h>

#include <kdebug.h>
#include <kabc/addressee.h>
#include "smslist.h"
#include "gammusms.h"

extern "C" {
    #include <gammu/gammu.h>
}

/**
 * This class abstracts the Gammu device functions and provides an easy
 * to use interface for the engine.
 *
 * @author Matthias Lechner
 */

class Device : public QObject
{
    Q_OBJECT
public:
    Device(const char* name);
    ~Device();

    /**
     * Returns whether the phone is initialized
     *
     * @return true if the phone is initialized
     */
    bool phoneConnected();


    /**
     * Returns the phone model information
     *
     * @return the device model
     */
    QString model();

    /**
     * Returns the phone manufacturer information
     *
     * @return the device manufacturer
     */
    QString manufacturer();

    /**
     * Returns the phone's firmware version
     * 
     * @return the device's firmware version
     */
    QString version();

    /**
     * Returns the device's International Mobile Equipment Identity (IMEI)
     *
     * @return the device's imei
     */
    QString imei();

    /**
     * Returns the SMS Center phone number
     *
     * @return the phone's smsc
     */
    QString smsc();

    /**
     * Returns the phone's battery status in percent
     *
     * @return the battery charge status
     */
    int battery();

    /**
     * Returns the network's name the phone is currently logged in
     *
     * @return the current network name
     */
    QString networkName();

    /**
     * Returns the signal quality in percent
     *
     * @return the signal quality in percent
     */
    int signalQuality();

    /**
     * Returns whether the phone is currently ringing
     *
     * @return true if the phone is ringing
     */
    bool ringing();

    /**
     * Returns the list of supported phonebook slots
     *
     * @return the list of supported phonebook slots
     */
    QValueList<QString> phonebookSlots();

    /**
     * Returns the device's phonebook. This will collect
     * contacts from the internal phonebook and the sim card
     * as well.
     * 
     * @return the device's phonebook
     */
    KABC::Addressee::List phonebook();

    /**
     * Adds the supplied addressee list to the phone's contact
     * database
     *
     * @p addresseeList the new contact
     */
    void addAddressee( KABC::Addressee::List *addresseeList );

    /**
     * Changes the supplied contact according to the
     * new one
     *
     * @p oldAddressee the contact to change
     * @p newAddressee the new contact information
     */
    void editAddressee( KABC::Addressee* oldAddressee, KABC::Addressee* newAddressee );

    /**
     * Removes the supplied addressee list from the phone's contact
     * database
     *
     * @p addresseeList the contact list to remove
     */
    void removeAddressee( KABC::Addressee::List *addresseeList );

    /**
     * This causes the phone to dial the supplied @p number
     *
     * @p number the phone number to dial
     * @return whether dialing was successful
     */
    bool dial( const QString& number );

    /**
     * This cancels an persisting phone call
     *
     * @return whether canceling was successful
     */
    bool hangup();

    /**
     * Returns the amount of unread SMS
     *
     * @return the amount of unread SMS
     */
    int unreadSMS();

    /**
     * Returns the total amount of SMS
     *
     * @return the total amount of SMS
     */
    int totalSMS();

    /**
     * Loads the phone's sms folder list and fetches the sms
     * 
     * @return the sms
     */
    SMSList* smsList();

    /**
     * Deletes the specified sms from the phone
     *
     * @p sms the sms to delete
     */
    void deleteSMS( GammuSMS *sms );

    /**
     * Stores the specified sms on the phone
     *
     * @p sms the sms to store
     */
    void storeSMS( SMS *sms );

    /**
     * Sends the specified previous stored sms
     *
     * @p sms the sms to send
     */
    void sendStoredSMS( GammuSMS *sms );

    /**
     * Sends the specified sms
     *
     * @p sms the sms to send
     */
    void sendSMS( SMS *sms );

public slots:
    /**
     * This method sets up the connection to the phone
     */
    void initPhone();

    /**
     * This terminates the connection if established
     */
    void terminatePhone();

private:
    bool m_isConnected;
    QMutex m_mutex;

    GSM_Phone_Functions *m_phoneFunctions;
    GSM_StateMachine m_stateMachine;
    GSM_Error m_error;

    QMap<int,QString> m_errorCodes;

    // cached values that don't change throughout the session
    QString m_model;
    QString m_manufacturer;
    QString m_version;
    QString m_imei;
    QValueList<QString> m_phonebookSlots;

    /**
     * This method converts a gammu phonebook entry to a
     * KABC entry
     * 
     * @p memoryEntry the gammu phonebook entry
     * @return the KABC entry
     */
    KABC::Addressee toKAbc( const GSM_MemoryEntry& memoryEntry );

    /**
     * This method converts a KABC entry to a gammu phonebook entry
     *
     * @p addressee the KABC entry
     * @return the gammu phonebook entry
     */
    GSM_MemoryEntry toMemoryEntry( KABC::Addressee* addressee );

    /**
     * This method composes a GSM_MultiSMSMessage out of a GammuSMS
     *
     * @p sms the sms to convert
     * @return the converted sms
     */
    GSM_MultiSMSMessage composeSMS( SMS *sms );

    /**
     * Prints a human-readable debugging message.
     * To simplify debugging you can specify the method where the error occurred.
     *
     * @p errorCode the gammu engine error code
     * @p methodName the name of the method where the error occurred
     * @return the translated error string
     */
    void printErrorMessage( int errorCode, const QString& methodName );

signals:
    void disconnected();
    void connected();

};

#endif
