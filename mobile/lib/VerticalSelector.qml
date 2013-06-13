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

import QtQuick 1.1

Item {
    id: verticalselector
    width: 160
    height: 90
    state: "unselected"

    property alias model: list.model
    property int value: -1
    signal selected()

    property int beginWith: 0
    property variant displayTexts

    onValueChanged: {
        list.positionViewAtIndex( value - beginWith, ListView.Center );
    }

    Image {
        id: inputLeft
        source: "images/scrollinput-left" + (verticalselector.focus ? "-active" : "") + ".png"

        anchors.top: parent.top
        anchors.topMargin: 15
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 15
        anchors.left: parent.left
        anchors.leftMargin: 5
    }

    BorderImage {
        id: inputCenter
        source: "images/scrollinput-center" + (verticalselector.focus ? "-active" : "") + ".png"

        anchors.top: parent.top
        anchors.topMargin: 15
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 15
        anchors.left: inputLeft.right
        anchors.right: inputRight.left
    }

    Image {
        id: inputRight
        source: "images/scrollinput-right" + (verticalselector.focus ? "-active" : "") + ".png"

        anchors.top: parent.top
        anchors.topMargin: 15
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 15
        anchors.right: parent.right
        anchors.rightMargin: 5
    }

    // ### TODO: for some reason, onValueChanged is not properly
    // setting the positionViewAtIndex when the component is just
    // loaded. maybe a bug with the list?
    function setValue(newValue) {
        value = newValue;
        list.positionViewAtIndex( value - beginWith, ListView.Center );
    }

    function setRange(range) {
        list.model = range
        list.positionViewAtIndex( value - beginWith, ListView.Center )
    }

    function toggleState(newValue)
    {
        state = (state == "selected") ? "unselected" : "selected";
        if (state == "unselected")
            verticalselector.value = newValue
        else
            verticalselector.selected();
    }

    Component {
        id: fadeDelegate
        Item {
            id: fadewrapper
            width: verticalselector.width
            height: verticalselector.height
            Text {
                text: displayTexts ? displayTexts[list.currentIndex + beginWith] : list.currentIndex + beginWith
                anchors.fill: parent
                color: "#004bb8"
                font.bold: true
                font.pixelSize: 28
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            MouseArea {
                hoverEnabled: true
                anchors.fill: parent
                onClicked: {
                    toggleState(list.currentIndex + beginWith)
                }
            }
        }
    }

    Component {
        id: highlight
        Rectangle {
            color: "red"
            opacity: 0
        }
    }

    ListView {
        id: list
        clip: true
        model: 10
        interactive: verticalselector.focus

        delegate: fadeDelegate
        highlight: highlight
        highlightRangeMode: ListView.StrictlyEnforceRange
        highlightFollowsCurrentItem: true
        preferredHighlightBegin: 0
        preferredHighlightEnd: verticalselector.height

        anchors.fill: parent
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        orientation: Qt.Vertical
        snapMode: ListView.SnapToItem
    }

    states: [
        State {
            name: "selected"
            PropertyChanges {
                target: verticalselector
                focus: true
                height: 135
                width: 240
            }
        },
        State {
            name: "unselected"
            PropertyChanges {
                target: verticalselector
                focus: false
                height: 90
                width: 160
            }
        }
    ]
    transitions: Transition {
        PropertyAnimation { target: verticalselector; properties: "height, width"; duration: 500 }
    }
}
