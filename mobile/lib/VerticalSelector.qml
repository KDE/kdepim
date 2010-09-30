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
    id: fadeselector
    width: 160
    height: 90
    state: "unselected"

    property alias model: list.model
    property alias currentIndex: list.currentIndex
    property int value
    signal selected()

    onValueChanged: {
        list.currentIndex = fadeselector.value;
    }

    Image {
        id: inputLeft
        source: "images/scrollinput-left" + (fadeselector.focus ? "-active" : "") + ".png"

        anchors.top: parent.top
        anchors.topMargin: 15
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 15
        anchors.left: parent.left
        anchors.leftMargin: 5
    }

    BorderImage {
        id: inputCenter
        source: "images/scrollinput-center" + (fadeselector.focus ? "-active" : "") + ".png"

        anchors.top: parent.top
        anchors.topMargin: 15
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 15
        anchors.left: inputLeft.right
        anchors.right: inputRight.left
    }

    Image {
        id: inputRight
        source: "images/scrollinput-right" + (fadeselector.focus ? "-active" : "") + ".png"

        anchors.top: parent.top
        anchors.topMargin: 15
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 15
        anchors.right: parent.right
        anchors.rightMargin: 5
    }

    function toggleState(newValue)
    {
        state = (state == "selected") ? "unselected" : "selected";
        if (state == "unselected")
            fadeselector.value = newValue;
        else
            fadeselector.selected();
    }

    Component {
        id: fadeDelegate
        Item {
            id: fadewrapper
            width: fadeselector.width
            height: fadeselector.height
            Text {
                text: (value == "") ? index : value
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
                    var newValue = (value == "") ? index : value;
                    toggleState(newValue)
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
        interactive: fadeselector.focus

        delegate: fadeDelegate
        highlight: highlight
        highlightRangeMode: ListView.StrictlyEnforceRange
        highlightFollowsCurrentItem: true
        preferredHighlightBegin: 0
        preferredHighlightEnd: fadeselector.height

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
                target: fadeselector
                focus: true
                height: 135
                width: 240
            }
        },
        State {
            name: "unselected"
            PropertyChanges {
                target: fadeselector
                focus: false
                height: 90
                width: 160
            }
        }
    ]
    transitions: Transition {
        PropertyAnimation { target: fadeselector; properties: "height, width"; duration: 500 }
    }
}
