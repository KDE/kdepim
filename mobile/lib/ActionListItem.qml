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

import QtQuick 1.1 as QML
import org.kde.pim.mobileui 4.5 as KPIM


QML.Item {
    id: _top
    height: parent.height
    width: (parent ? parent.width : 0)

    property string name
    property string argument: ""
    property string title: ""
    property bool reactsOnLongPressed: false
    property string category

    signal triggered(string triggeredName)
    signal doCollapse()
    signal pressAndHold()

    onVisibleChanged: {

        if (!visible) {
            height = -actionItemSpacing
        } else {
            height = actionItemHeight
        }
    }

    KPIM.Action {
        height: parent.height
        width: parent.width
        showIcon: false
        hidable: false

        action: {
            application.setActionTitle(name, title);
            application.getAction(name, argument);
        }

        actionIconName: {
            application.getActionIconName(name);
        }

        onLongPressed: {
            pressAndHold();
        }

        onTriggered: {
            parent.doCollapse()
            parent.triggered(name)
        }

        QML.Image {
            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
            }
            source: KDE.locate( "data", "mobileui/long-press-indicator.png" )
            visible: _top.reactsOnLongPressed
        }
    }
}
