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
    id: actionList_top
    width: parent.width

    //property alias listElementContent: _listElementContent.data
    default property alias listContent: _listContent.content
    property string category
    property string name
    property alias text: nameItem.buttonText
    property alias columnHeight: _listContent.height
    property alias totalWidth: _listContent.width
    property alias depth: _listContent.bottomMargin
    property alias actionItemSpacing: _listContent.actionItemSpacing
    property alias actionItemHeight: _listContent.actionItemHeight
    property alias showChildren: _listContent.visible
    property alias menuStyle: _listContent.menuStyle

    signal doCollapse()
    signal triggered(string triggeredName)

    function preselectAction() {
        actionList_top.triggered(actionList_top.name)
        showChildren = true
    }

    onVisibleChanged: {
        if (!visible) {
            height = -actionItemSpacing
        } else {
            height = actionItemHeight
        }
    }



    KPIM.Button {
        id: nameItem
        height: parent.height
        width: parent.width

        onClicked: {
            actionList_top.triggered(actionList_top.name)
            showChildren = true
        }
    }

    KPIM.ActionMenuContainer {
        id: _listContent

        anchors {
            left: nameItem.right
            bottom: nameItem.top
        }

        visible: false
        actionItemWidth: parent.totalWidth

        onTriggered: {
            actionList_top.triggered(triggeredName)
        }

        onDoCollapse: {
            actionList_top.doCollapse()
        }

    }
}
