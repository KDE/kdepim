/*
    Copyright (C) 2010 Artur Duque de Souza <asouza@kde.org>

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

Item {
    id: calendar
    width: 460
    height: 360

    Component {
        id: day
        Item {
            id: wrapper
            property alias text: myText.text
            property alias active: active.visible
            property alias valid: inactive.visible

            width: 54
            height: 54
            Image {
                id: normal
                source: "images/normaldate.png"
                anchors.fill: parent
            }
            Image {
                id: inactive
                source: "images/inactivedate.png"
                anchors.fill: parent
            }
            Image {
                id: active
                visible: false
                source: "images/activedate.png"
                anchors.fill: parent
            }
            Text {
                id: myText
                color: "#004bb8"
                font.bold: true
                font.pixelSize: 28
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    Row {
        id: title
        spacing: 8
        anchors.left: parent.left
        anchors.leftMargin: spacer.width + 5

        Text {
            id: month
            text: "April"
            color: "#004bb8"
            font.pixelSize: 24
        }

        Text {
            id: year
            text: "2010"
            color: "#004bb8"
            font.pixelSize: 24
        }
    }

    Row {
        spacing: 6
        anchors.top: title.bottom

        Column {
            id: weeks
            spacing: 6

            Item {
                id: spacer
                width: 20
                height: 20
            }

            Text {
                id: week1
                width: 20
                height: 54
                text: "1"
                color: "#828282"
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignTop
            }

            Text {
                id: week2
                width: 20
                height: 54
                text: "2"
                color: "#828282"
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignTop
            }

            Text {
                id: week3
                width: 20
                height: 54
                text: "3"
                color: "#828282"
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignTop
            }

            Text {
                id: week4
                width: 20
                height: 54
                text: "4"
                color: "#828282"
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignTop
            }

            Text {
                id: week5
                width: 20
                height: 54
                text: "5"
                color: "#828282"
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignTop
            }
        }

        Column {
            id: sundays
            spacing: 6

            Text {
                id: sundayLabel
                width: 54
                height: 20
                text: "Sun"
                color: "#5c5c5c"
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

        }

        Column {
            id: mondays
            spacing: 6

            Text {
                id: mondayLabel
                width: 54
                height: 20
                text: "Mon"
                color: "#5c5c5c"
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }
        }

        Column {
            id: tuesdays
            spacing: 6

            Text {
                id: tuesdayLabel
                width: 54
                height: 20
                text: "Tue"
                color: "#5c5c5c"
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }
        }

        Column {
            id: wednesdays
            spacing: 6

            Text {
                id: wednesdayLabel
                width: 54
                height: 20
                text: "Wed"
                color: "#5c5c5c"
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }
        }

        Column {
            id: thursdays
            spacing: 6

            Text {
                id: thursdayLabel
                width: 54
                height: 20
                text: "Thu"
                color: "#5c5c5c"
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }
        }

        Column {
            id: fridays
            spacing: 6

            Text {
                id: fridayLabel
                width: 54
                height: 20
                text: "Fri"
                color: "#5c5c5c"
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }
        }

        Column {
            id: saturdays
            spacing: 6

            Text {
                id: saturdayLabel
                width: 54
                height: 20
                text: "Sat"
                color: "#5c5c5c"
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }

            Loader {
                sourceComponent: day
            }
        }
    }


    // ### TODO: i18n calls
    // Row {
    //     width: 420
    //     anchors.left: parent.left
    //     anchors.leftMargin: 12
    //     spacing: 40

    // }

}