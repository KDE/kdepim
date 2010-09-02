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

    property alias model: list.model
    property alias currentIndex: list.currentIndex


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

    function relativeopacity(y1, y2, height) {
        var opacity = (y2 + y1) / height;
        if (opacity > 1)
            return 2.4 - opacity;
        else if (opacity == 1)
            return opacity
        return opacity + 0.4
    }

    function indexopacity(index) {
        if (index == list.currentIndex)
            return 1;
        return 0;
    }

    Component {
        id: fadeDelegate
        Item {
            id: fadewrapper
            width: fadeselector.width
            height: 30
            opacity: fadeselector.focus ? relativeopacity(parent.y, y, height) : indexopacity(index);
            Text {
                text: index
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
                    fadeselector.focus = true;
                }
                onEntered: {
                    fadeselector.focus = true;
                }
                onExited: {
                    fadeselector.focus = false;
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
        delegate: fadeDelegate
        highlight: highlight
        highlightRangeMode: ListView.StrictlyEnforceRange
        highlightFollowsCurrentItem: true
        preferredHighlightBegin: 30
        preferredHighlightEnd: 60

        anchors.fill: parent
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        orientation: Qt.Vertical
        snapMode: ListView.SnapToItem
    }

}
