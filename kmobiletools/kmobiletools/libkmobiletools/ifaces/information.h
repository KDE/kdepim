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

#ifndef KMOBILETOOLSIFACESINFORMATION_H
#define KMOBILETOOLSIFACESINFORMATION_H

#include <QtCore/QObject>

#include <libkmobiletools/information.h>
#include <libkmobiletools/kmobiletools_export.h>


namespace KMobileTools {

namespace Ifaces {

/**
    This interface provides methods to gather basic information about the phone

    @author Matthias Lechner <matthias@lmme.de>
*/
class KMOBILETOOLS_EXPORT Information {
public:
//public Q_SLOTS:
    /**
     * Fetches information about the phone
     */
    virtual void fetchInformation() = 0;

public:
    /**
     * Returns the network the phone is currently logged in
     *
     * @return the network name
     */
    virtual QString networkName() const = 0;

    /**
     * Returns the phone manufacturer
     *
     * @return a QString containing the phone manufacturer.
     */
    virtual QString manufacturer() const = 0;

    /**
     * Returns the phone manufacturer ID
     *
     * @return the phone manufacturer ID
     */
    virtual KMobileTools::Information::Manufacturer manufacturerID() const = 0;

    /**
     * Returns the phone model
     *
     * @return the phone model
     */
    virtual QString model() const = 0;

    /**
      * Returns the phone's raw IMEI.
      * The IMEI is a number unique to every GSM and UMTS mobile phone.
      *
      * @return the phone raw IMEI
      */
    virtual QString imei() const = 0;

    /**
      * Returns the phone firmware revision.
      *
      * @return the phone firmware revision
      */
    virtual QString revision() const = 0;

    virtual ~Information();

protected:
//Q_SIGNALS:
    /**
     * This signal is emitted when the phone information have been fetched from
     * the phone
     */
    virtual void informationFetched() = 0;

    /**
     * This signal is emitted whenever the network name changes
     */
    virtual void networkNameChanged( const QString& networkName ) = 0;
};

}
}

Q_DECLARE_INTERFACE(KMobileTools::Ifaces::Information, "org.kde.KMobileTools.Ifaces.Information/0.1")


#endif
