/*
    Copyright (C) 2010 Anselmo Lacerda Silveira de Melo <anselmolsm@gmail.com>

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

import Qt 4.7

Image {
    id: clock
    source: "images/clock-background.png"
    smooth: true

    Image {
        id: seconds
        source: "images/clock-seconds.png"
        smooth: true

        x: clock.width/2 - width/2
        y: clock.height/2 - seconds.height*0.95
        height: clock.height/(372/173)
        width: clock.width/(370/3)

        transform: Rotation {
            origin.x: seconds.width/2
            origin.y: seconds.height*0.95
        }
    }

    Image {
        id: minutes
        source: "images/clock-minutes.png"
        smooth: true

        x: clock.width/2 - width/2
        y: clock.height/2 - minutes.height*0.95
        height: clock.height/(372/148)
        width: clock.width/(370/12)

        transform: Rotation {
            origin.x: minutes.width/2
            origin.y: minutes.height*0.95
        }
    }

    Image {
        id: hours
        source: "images/clock-hours.png"
        smooth: true

        x: clock.width/2 - hours.width/2
        y: clock.height/2 - hours.height*0.95
        height: clock.height/(372/113)
        width: clock.width/(370/12)

        transform: Rotation {
            origin.x: hours.width/2
            origin.y: hours.height*0.95
        }
    }

    Image {
        id: center
        source: "images/clock-center.png"

        anchors.centerIn: clock
        height: clock.height/(372/38)
        width: clock.width/(370/38)
    }
}
