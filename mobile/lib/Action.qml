/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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

Rectangle {
    id: _topContext

    property variant action
    property variant actionIconName
    property int hardcoded_height: 70
    property bool disableable: true
    property bool showIcon: true
    property bool hidable: true
    property bool hidden: hidable && !action.enabled
    property bool checkable: false
    property alias showText: buttonText.visible
    property alias image: image.source
    property alias imageWidth: image.width
    property alias imageHeight: image.height

    signal triggered()
    signal longPressed()

    Binding {
        target: image
        property: "source"
        value: "image://action_images/" + actionIconName
        when: showIcon && actionIconName != undefined
    }

    Binding {
        target: buttonText
        property: "text"

        value: {
            action.text.replace("&", "");
        }
    }

    Binding {
        target: _topContext
        property: "enabled"
        value: disableable ? action.enabled : true
    }

    height: (!hidable || action.enabled) ? hardcoded_height : 0
    visible: (!hidable || action.enabled)

    Connections {
        target: action

        onChanged: {
            border.width = action.checked ? 2 : 0
            if (!hidable)
              return;
            parent.height = action.enabled ? hardcoded_height : 0;
        }
    }

    radius: 12
    color: "#00000000" // Set a transparent color.

    Image {
        id: image
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 5
    }

    Text {
        id: buttonText
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        color: parent.enabled ? "black" : "gray"
    }

    MouseArea {
        anchors.fill: parent

        onPressed: {
            border.width = 2
        }

        onReleased: {
            border.width = 0
        }

        onCanceled: border.width = 0

        onClicked: {
            triggered();
            action.trigger();
        }

        onPressAndHold: {
            longPressed()
        }

    }

    border.color: "#4166F5"
    border.width: action.checked ? 2 : 0
}
