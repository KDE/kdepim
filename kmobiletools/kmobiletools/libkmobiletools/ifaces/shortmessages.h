/***************************************************************************
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

#ifndef KMOBILETOOLSIFACESSHORTMESSAGES_H
#define KMOBILETOOLSIFACESSHORTMESSAGES_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QString>

#include <libkmobiletools/kmobiletools_export.h>
#include <libkmobiletools/shortmessages.h>

namespace KMobileTools {

namespace Ifaces {
class SMS;
class SMSList;

/**
    This interface provides methods to access the phone's short message services

    @author Matthias Lechner <matthias@lmme.de>
*/
class KMOBILETOOLS_EXPORT ShortMessages {
public:
//public Q_SLOTS:
    /**
     * Fetches all short messages from the phone
     */
    virtual void fetchSMS() = 0;

    /**
     * Fetches the set sms center (SMSC) from the phone
     */
    virtual void fetchSMSCenter() = 0;

    /**
     * Stores the given @p sms on the phone
     *
     * @param sms the sms to store
     */
    virtual void storeSMS( const SMS& sms ) = 0;

    /**
     * Sends the given @p sms
     *
     * @param sms the sms to send
     */
    virtual void sendSMS( const SMS& sms ) = 0;

    /**
     * Sends the given @p sms stored on the phone
     *
     * @param sms the sms to store
     */
    virtual void sendStoredSMS( const SMS& sms ) = 0;

    /**
     * Removes the given @p sms from the phone
     *
     * @param sms the sms to remove
     */
    virtual void removeSMS( const SMS& sms ) = 0;

public:
    /**
     * Returns an OR-combination of available memory slots
     */
    virtual KMobileTools::ShortMessages::MemorySlots availableMemorySlots() const = 0;

    /**
     * Returns the fetched list of sms
     *
     * @return the fetched sms list
     */
    virtual SMSList shortMessages() const = 0;

    /**
     * Returns the fetched sms center (SMSC)
     *
     * @return the fetched sms center as a <number,name> pair
     */
    virtual QMap<QString,QString> smsCenter() const = 0;

    /**
     * Sets the default sms center (SMSC) on the phone to the given parameters
     *
     * @param number the SMSC's number
     * @param name the SMSC's name
     */
    virtual void setSMSCenter( const QString& number, const QString& name = QString() ) = 0;

    virtual ~ShortMessages();

protected:
//Q_SIGNALS:
    /**
     * This signal is emitted when the short messages have been fetched from
     * the phone
     */
    virtual void shortMessagesFetched() = 0;

    /**
     * This signal is emitted when a sms has been stored on the phone
     *
     * @p sms the stored sms
     */
    virtual void shortMessageStored( const SMS& sms ) = 0;

    /**
     * This signal is emitted when a sms has been sent
     *
     * @p sms the stored sms
     */
    virtual void shortMessageSent( const SMS& sms ) = 0;

    /**
     * This signal is emitted when a sms has been removed
     *
     * @p sms the removed sms
     */
    virtual void shortMessageRemoved( const SMS& sms ) = 0;

    /**
     * This signal is emitted when the sms center (SMSC) has been
     * fetched from the phone
     */
    virtual void smsCenterFetched() = 0;

};

}
}

Q_DECLARE_INTERFACE(KMobileTools::Ifaces::ShortMessages, "org.kde.KMobileTools.Ifaces.ShortMessages/0.1")


#endif
