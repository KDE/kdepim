/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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
import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.pim.mobileui 4.5 as KPIM

QML.Rectangle {

    id: _topLevel
    anchors.bottomMargin: bottomMargin
    color: "#00000000"

    property int actionItemHeight: 70
    property int actionItemWidth: 200
    property int actionItemSpacing: 0
    property int bottomMargin
    property alias model: myList.model
    property alias customActions: actionColumn.content

    signal triggered(string triggeredName)

    PlasmaExtras.ScrollArea {

        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
        }

        width: parent.width - actionColumn.width

        flickableItem: QML.ListView {
            id: myList
            focus: true
            clip: true

            delegate: AgentInstanceListDelegate {
                height: _topLevel.actionItemHeight
                width: myList.width
            }

            onCurrentIndexChanged: {
                application.setAgentInstanceListSelectedRow( currentIndex )
            }
        }
    }

    ActionMenuContainer {
        id: actionColumn
        width: _topLevel.actionItemWidth
        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }
        actionItemWidth: width
        actionItemHeight: _topLevel.actionItemHeight

        content: [
            ActionListItem { name: "akonadi_agentinstance_configure" },
            ActionListItem { name: "akonadi_agentinstance_delete" },
            ActionListItem { name: "akonadi_agentinstance_create" }
        ]
    }

}
