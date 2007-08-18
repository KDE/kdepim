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

#ifndef KMOBILETOOLSIFACESSTATUS_H
#define KMOBILETOOLSIFACESSTATUS_H

#include <QtCore/QObject>

#include <libkmobiletools/status.h>
#include <libkmobiletools/kmobiletools_export.h>


namespace KMobileTools {

namespace Ifaces {

/**
    This interface provides methods to trigger dialing on the phone

    @author Matthias Lechner <matthias@lmme.de>
*/
class KMOBILETOOLS_EXPORT Status {
public:
//public Q_SLOTS:
    /**
     * Fetches status information from the phone
     */
    virtual void fetchStatusInformation() = 0;

public:
    /**
     * Returns the phone's signal strength in percent
     *
     * @return the signal strength in percent
     */
    virtual int signalStrength() const = 0;

    /**
     * Returns the phone's charge in percent
     *
     * @return the charge in percent
     */
    virtual int charge() const = 0;

    /**
     * Returns the phone's current power supply type
     *
     * @return the power supply type
     */
    virtual KMobileTools::Status::PowerSupplyType powerSupplyType() const = 0;

    /**
     * Returns whether the phone is ringing
     *
     * @return true if the phone is ringing
     */
    virtual bool ringing() const = 0;

    virtual ~Status();

protected:
//Q_SIGNALS:
    /**
     * This signal is emitted when the status information have been fetched from
     * the phone
     */
    virtual void statusInformationFetched() = 0;

    /**
      * This signal is emitted whenever the signal strength has changed.
      *
      * @param signalStrength the signal level in percent
      */
    virtual void signalStrengthChanged( int signalStrength ) = 0;

    /**
     * This signal is whenever the phone charge changes
     *
     * @param charge the charge level in percent
     */
    virtual void chargeChanged( int charge ) = 0;

    /**
     * This signal is emitted whenever the phone's power supply source changes
     *
     * @param powerSupplyType the power supply type
     */
    virtual void chargeTypeChanged( KMobileTools::Status::PowerSupplyType powerSupplyType ) = 0;

    /**
     * This signal whenever the phone is ringing
     *
     * @param ringing true if phone is ringing
     */
    virtual void phoneRinging( bool ringing ) = 0;
};

}
}

Q_DECLARE_INTERFACE(KMobileTools::Ifaces::Status, "org.kde.KMobileTools.Ifaces.Status/0.1")


#endif
