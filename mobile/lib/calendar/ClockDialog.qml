/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Artur Duque de Souza <asouza@kde.org>
    Copyright (C) 2010 Anselmo Lacerda Silveira de Melo <anselmolsm@gmail.com>
    Copyright (c) 2010 Eduardo Madeira Fleury <efleury@gmail.com>

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

import QtQuick 1.1
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM

Dialog {
    id: clockWidget

    property alias okEnabled: clockWidgetOk.enabled
    property alias hours: hourSelector.value
    property alias minutes: minuteSelector.value
    property bool blockSignalEmission: false

    signal timeChanged( int hour, int minute )

    content: [
        Item {
            anchors.fill: parent

            MouseArea {
                anchors.fill: parent
                onClicked: {}
            }

            Item {
                id: digitalClock
                anchors.fill: parent

                Row {
                    anchors.centerIn: parent

                    Text {
                        id: hourSelector

                        property int value: 0

                        width: digitalClock.width/2.5
                        text: value < 10 ? "0" + value : value
                        color: "#004bb8"
                        font.bold: true
                        font.pixelSize: 100
                        horizontalAlignment: Text.AlignRight
                        verticalAlignment: Text.AlignVCenter
                        style: Text.Sunken

                        MouseArea {
                            anchors.fill: parent

                            onClicked: {
                                hoursTable.currentHour = hourSelector.value;
                                hoursTable.visible = true
                            }
                        }
                    }

                    Text {
                        text: ":"
                        width: digitalClock.width - hourSelector.width - minuteSelector.width
                        color: "#004bb8"
                        font.bold: true
                        font.pixelSize: 100
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        style: Text.Sunken
                    }

                    Text {
                        id: minuteSelector

                        width: digitalClock.width/2.5
                        text: value < 10 ? "0" + value : value
                        color: "#004bb8"
                        font.bold: true
                        font.pixelSize: 100
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        style: Text.Sunken

                        property int value: 0

                        MouseArea {
                            anchors.fill: parent

                            onClicked: {
                                minutesTable.currentMinute = minuteSelector.value;
                                minutesTable.visible = true
                            }
                        }
                    }
                }
            }

            Row {
                spacing: 5

                anchors{
                    bottom: parent.bottom
                    right: parent.right
                }

                KPIM.Button2 {
                    id: clockWidgetCancel
                    buttonText: KDE.i18n( "Cancel" );
                    width: 130

                    onClicked: {
                        clockWidget.collapse()
                    }
                }

                KPIM.Button2 {
                    id: clockWidgetOk
                    buttonText: KDE.i18n( "OK" );
                    width: 130

                    onClicked: {
                        clockWidget.collapse()
                        timeChanged(clockWidget.hours, clockWidget.minutes);
                    }
                }
            }
        }
    ]

    Rectangle {
        id: hoursTable

        visible: false

        property int currentHour

        signal selectionFinished( int value )

        Grid {
            anchors.fill: parent
            spacing: 3
            columns: 6
            rows: 4

            Repeater {
                model: 24

                Image {
                    width: (hoursTable.width-(3*5))/6
                    height: (hoursTable.height-(3*3))/4
                    source: "images/normaldate.png"

                    Image {
                        anchors.fill: parent
                        source: {
                            if ( hourText.text == hoursTable.currentHour ) {
                                return "images/activedate.png";
                            } else {
                                return "";
                            }
                        }
                    }

                    Text {
                        id: hourText
                        anchors.centerIn: parent
                        color: "#5ba0d4"
                        font.bold: true
                        font.pixelSize: 26
                        style: Text.Sunken
                        text: index
                    }

                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            hoursTable.currentHour = index
                            hoursTable.selectionFinished( index )
                            hoursTable.visible = false
                        }
                    }
                }
            }
        }

        Behavior on visible {

            ParallelAnimation {

                PropertyAnimation {
                    target: hoursTable
                    property: "x"
                    from: mapFromItem(hourSelector, hourSelector.width/2, 0).x
                    to: 0
                    duration: 200
                }
                PropertyAnimation {
                    target: hoursTable
                    property: "y"
                    from: mapFromItem(hourSelector, 0, hourSelector.height/2).y
                    to: 0
                    duration: 200
                }
                PropertyAnimation {
                    target: hoursTable
                    property: "width"
                    from: 0
                    to: clockWidget.width
                    duration: 200
                }
                PropertyAnimation {
                    target: hoursTable
                    property: "height"
                    from: 0
                    to: clockWidget.height
                    duration: 200
                }
            }
        }
    }

    Connections {
        target: hoursTable
        onSelectionFinished: {
            hourSelector.value = value
            clockWidgetOk.enabled = true
        }
    }

    Rectangle {
        id: minutesTable
        visible: false

        property int currentMinute
        property variant minutesOffset: [0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55];

        signal selectionFinished( int value )

        onCurrentMinuteChanged: {
            // round input values to a multiple of 5
            currentMinute = (Math.round(currentMinute/5)*5 == 60) ? 0 : Math.round(currentMinute/5)*5
        }

        Grid {
            anchors.fill: parent
            spacing: 3
            columns: 4
            rows: 3

            Repeater {
                model: 12

                Image {
                    width: (minutesTable.width-(3*3))/4
                    height: (minutesTable.height-(3*2))/3
                    source: "images/normaldate.png"

                    Image {
                        anchors.fill: parent

                        source: {
                            if ( minuteText.text == minutesTable.currentMinute ) {
                                return "images/activedate.png";
                            } else {
                                return "";
                            }
                        }
                    }

                    Text {
                        id: minuteText
                        anchors.centerIn: parent
                        color: "#5ba0d4"
                        font.bold: true
                        font.pixelSize: 26
                        style: Text.Sunken
                        text: minutesTable.minutesOffset[ index ]
                    }

                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            minutesTable.currentMinute = minutesTable.minutesOffset[ index ]
                            minutesTable.selectionFinished( minutesTable.minutesOffset[ index ] )
                            minutesTable.visible = false
                        }
                    }
                }
            }
        }

        Behavior on visible {

            ParallelAnimation {

                PropertyAnimation {
                    target: minutesTable
                    property: "x"
                    from: mapFromItem(minuteSelector, minuteSelector.width/2, 0).x
                    to: 0
                    duration: 200
                }
                PropertyAnimation {
                    target: minutesTable;
                    property: "y"
                    from: mapFromItem(minuteSelector, 0, minuteSelector.height/2).y
                    to: 0
                    duration: 200
                }
                PropertyAnimation {
                    target: minutesTable
                    property: "width"
                    from: 0
                    to: clockWidget.width
                    duration: 200
                }
                PropertyAnimation {
                    target: minutesTable
                    property: "height"
                    from: 0
                    to: clockWidget.height
                    duration: 200
                }
            }
        }
    }

    Connections {
        target: minutesTable
        onSelectionFinished: {
            minuteSelector.value = value
            clockWidgetOk.enabled = true
        }
    }
}
