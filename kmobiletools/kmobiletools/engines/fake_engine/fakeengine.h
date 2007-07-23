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

#ifndef KMOBILETOOLSFAKEENGINE_H
#define KMOBILETOOLSFAKEENGINE_H

#include <QtCore/QObject>

#include <libkmobiletools/enginexp.h>

#include <libkmobiletools/ifaces/status.h>
#include <libkmobiletools/ifaces/information.h>

/**
    @author Matthias Lechner <matthias@lmme.de>
*/
class FakeEngine : public KMobileTools::EngineXP, // base class
                   public KMobileTools::Ifaces::Status,         // interfaces
                   public KMobileTools::Ifaces::Information
{
    Q_OBJECT
    Q_INTERFACES(KMobileTools::Ifaces::Status KMobileTools::Ifaces::Information)

public:
    FakeEngine( QObject *parent );
    virtual ~FakeEngine();

    void initialize() {};

    //
    // Status interface implementation
    //
    void fetchStatusInformation() {};
    int signalStrength() const {};
    int charge() const {};
    KMobileTools::Status::PowerSupplyType powerSupplyType() const {};
    bool ringing() const {};

    //
    // Information interface implementation
    //
    void fetchInformation() {};
    QString networkName() const {};
    QString manufacturer() const {};
    KMobileTools::Information::Manufacturer manufacturerID() const {};
    QString model() const {};
    QString imei() const {};
    QString revision() const {};

Q_SIGNALS:

    //
    // Status interface implementation
    //
    void signalStrengthChanged( int );
    void chargeChanged( int );
    void chargeTypeChanged( KMobileTools::Status::PowerSupplyType );
    void phoneRinging( bool );

    //
    // Information interface implementation
    //
    void networkNameChanged( const QString& );

    void initialized( bool successful );

};

#endif
