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

#ifndef KMOBILETOOLSGAMMU_ENGINE_H
#define KMOBILETOOLSGAMMU_ENGINE_H

#include "kmobiletoolsengine.h"

extern "C" {
    #include <gammu/gammu.h>
}

#include "device.h"

#include <qptrlist.h>
#include <kabc/addressee.h>
#include <qptrvector.h>

using namespace ThreadWeaver;

/**
 * This is the gammu engine. It uses gammu to access a mobile phone.
 */
class kmobiletoolsGammu_engine : public kmobiletoolsEngine
{
    Q_OBJECT

public:
    explicit kmobiletoolsGammu_engine(QObject *parent = 0, const char *name = 0);
    ~kmobiletoolsGammu_engine();

public:
    /**
     * Returns the engine's name defined in the corresponding
     * desktop file
     *
     * @return the engine's name
     */
    QString engineDesktopName() const;

    /**
     * Returns the engine's library name
     *
     * @return the engine's library name
     */
    QString engineLibName() const;

    /**
     * Retrieves the numbers of phonebook memory slots
     *
     * @return the numbers of phonebook memory slots
     */
    int availPbSlots();

    /**
     * Shows if mobile phone can encode text in PDU mode
     *
     * @return true if mobile phone is PDU able
     */
    bool pdu();

    /**
     * This method is called when kmobiletools receives a
     * close event. It terminates the phone connection and
     * does some cleaning.
     */
    void queryClose();

public slots:
    /**
     * This method is called whenever a the specified job is finished
     * to inform about the results
     *
     * @p job the job that is finished
     */
    void processSlot( Job* job );

    /**
     * This method is the first method that's called after creating
     * the engine's instance. It looks up how the phone is connected
     * to the computer (bluetooth, irda..)
     *
     * @see probeDevice()
     */
    void probePhone();

    /**
     * This method is called when probePhone() has finished probing the devices.
     * Here we do the actual initialization of the device.
     */
    void initPhone();

    /**
     * This is called by DeviceInfoList's probeDevice() method.
     * Here we actually probe the device with the specified @p deviceName
     *
     * @p job the job that can be used to probe the device
     * @p fullprobe if true, not only IMEI but other information is fetched too
     * @p deviceName the device's address
     * @p params additional parameters to talk to the device
     */
    DeviceInfos probeDevice( ThreadWeaver::Job *job, bool fullprobe,
                            const QString &deviceName,
                            const QStringList &params ) const;

    // TODO: check if methods can be removed from public api since
    // resumeDevice() and stopDevice() exist
    void setDevice ( const QString& ) {}
    void closeDevice() {}

    /**
     * This method causes to phone to dial the specified @p number
     * or hangup, depending on the @p dialActions set
     * 
     * @p dialActions specifies whether to call or hangup
     * @p number the phone number to call
     */
    void dial( DialActions dialActions, const QString &number = QString() );

    /**
     * Returns the phone's encoding. Since we use gammu, this returns utf8.
     */
    QStringList encodings();

    /******************************************************
     * Phone information related methods                  *
     ******************************************************/
    /**
     * This triggers a job which retreives status information
     * such as charge and signal information
     */
    void pollPhoneStatus();

    /**
     * This triggers a job which retreives basic phone information
     * such as model, IMEI and manufacturer.
     */
    void getPhoneInfos();


    /******************************************************
     * SMS related methods                                *
     ******************************************************/

    /**
     * When a current SMS list is required, this method will create
     * a job for this task
     */
    void retrieveSMSList();

    /**
     * Deletes the specified sms from the phone
     * 
     * @p sms the sms to be deleted
     */
    void slotDelSMS( SMS* sms );

    /**
     * Stores the specified sms from the phone
     *
     * @p sms the sms to be stored on the phone
     */
    void slotStoreSMS( SMS* sms );

    /**
     * Stores the specified sms from the phone
     *
     * @p number the recipient's phone number
     * @p text the text to send
     */
    void slotStoreSMS( const QString &number, const QString &text );

    /**
     * Calling this method causes the phone to send the
     * specified sms
     *
     * @p sms the sms you want to send
     */
    void slotSendSMS( SMS* sms );

    /**
     * Calling this method causes the phone to send the
     * specified sms
     *
     * @p number the recipient's phone number
     * @p text the text to send
     */
    void slotSendSMS( const QString &number, const QString &text );

    /**
     * This will send an sms that is already
     * on the phone.
     *
     * @p sms the sms to be sent
     */
    void slotSendStoredSMS( SMS* sms );


    /******************************************************
     * Phone book related methods                         *
     ******************************************************/

    /**
     * This triggers a job which retreives the device's phone
     * book
     */
    void retrieveAddressBook();

    /**
     * Adds contacts to the phone's address book
     *
     * @p addressees the list that should be added
     */
    void slotAddAddressee( QValueList<KABC::Addressee>* addressees );

    /**
     * Deletes contacts from the phone's address book
     *
     * @p addressees the contacts to be removed
     */
    void slotDelAddressee( QValueList<KABC::Addressee>* addressees );

    /**
     * Edits the specified oldAddressee contact and sets
     * the information supplied by newAddressee
     *
     * @p oldAddressee the contact to be edited
     * @p newAddressee the new contact information
     */
    void slotEditAddressee( KABC::Addressee* oldAddressee,
                            KABC::Addressee* newAddressee );



    /******************************************************
     * Calendar related methods                           *
     ******************************************************/
    /**
     * This triggers a job which retreives the device's calendar
     */
    virtual void fetchCalendar();

private:
    Device* m_device;

    ContactPtrList m_addresseeList;
};


/**
 * The factory which creates a gammu engine instance.
 */
class kmobiletoolsGammu_engineFactory : public KLibFactory
{
   Q_OBJECT

public:
    kmobiletoolsGammu_engineFactory();
    ~kmobiletoolsGammu_engineFactory();

    kmobiletoolsGammu_engine* createObject( QObject *parent, const char *name,
                                            const char *classname, const QStringList &args );
};



#endif
