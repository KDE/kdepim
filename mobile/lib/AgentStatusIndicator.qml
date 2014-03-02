/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

import QtQuick 1.1 as QML
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM

QML.Item {
    id: root

    width: icon.width
    height: icon.height

    function iconFromStatus( status ) {
        if ( !(status & KPIM.AgentStatusMonitor.Online) ) {
            return "images/status/offline.png";
        } else {
            if ( (status & KPIM.AgentStatusMonitor.Sending) && (status & KPIM.AgentStatusMonitor.Receiving) ) {
                return "images/status/sending-receiving.png";
            } else if ( status & KPIM.AgentStatusMonitor.Receiving ) {
                return "images/status/receiving.png";
            } else if ( status & KPIM.AgentStatusMonitor.Sending ) {
                return "images/status/sending.png";
            } else {
                return "images/status/online.png"
            }
        }
    }

    QML.Image {
        id: icon
        source: iconFromStatus( agentStatusMonitor.status )
    }
}
