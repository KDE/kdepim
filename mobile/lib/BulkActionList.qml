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
import org.kde.akonadi 4.5
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.plasma.extras 0.1 as PlasmaExtras

QML.Column {
    id: _top

    property alias selectedItemModel: selectedItem.model
    property alias multipleText: _multipleText.text
    property alias actionModel: actionListView.model
    property int itemHeight: 65

    signal backClicked()
    signal triggered(string name)

    onChildrenChanged: {
        var newChild = children[ children.length - 1 ];
        newChild.anchors.left = _top.left;
        newChild.anchors.right = _top.right;
        newChild.height = itemHeight;
    }

    QML.Item {
        id: firstItem
        height: itemHeight
        anchors {
            left: parent.left
            right: parent.right
        }


        PlasmaExtras.ScrollArea {
            id: selectedItemContainer
            anchors.fill: parent

            flickableItem: QML.ListView {
                id: selectedItem

                delegate: CollectionDelegate {
                    height: 70
                    indentation: 80
                }
                visible: count == 1
            }
        }

        QML.Text {
            id: _multipleText
            anchors.horizontalCenter : parent.horizontalCenter
            y: height / 2
            visible: selectedItem.count != 1
            height: 70
        }

        QML.Image {
            id: topLine
            source: "images/list-line-top.png"
            anchors {
                right: selectedItemContainer.right
                top: selectedItemContainer.top
            }
        }

        QML.Image {
            id: topLineFiller
            source: "images/dividing-line-horizontal.png"
            anchors {
                right: topLine.left
                bottom: topLine.bottom
            }
            fillMode: QML.Image.TileHorizontally
            width: parent.width - topLine.width
        }

        QML.Image {
            id: bottomLine
            source: "images/dividing-line-horizontal.png"
            anchors {
                right: selectedItemContainer.right
                bottom: selectedItemContainer.bottom
            }
            fillMode: QML.Image.TileHorizontally
            width: parent.width
        }

        QML.Image {
            source: "images/dividing-line.png"
            anchors {
                top: selectedItemContainer.bottom
                right: parent.right
            }
            height: _top.height - selectedItemContainer.height
            fillMode: QML.Image.TileVertically
        }

        QML.Image {
            id: backIcon
            source: "images/bulk-back-overlay.png"
            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
            }
        }

        QML.MouseArea {
            id: back_ma
            anchors.fill: parent
            onClicked: _top.backClicked()
        }

    }

    QML.ListModel {
        id: actionModel;

        QML.ListElement {
            action: "akonadi_item_delete"
        }

        QML.ListElement {
            action: "akonadi_item_move_to_dialog"
        }

        QML.ListElement {
            action: "akonadi_item_copy_to_dialog"
        }
    }

    QML.Rectangle {
        anchors {
            left: _top.left
            right: _top.right
            top: _top.top
            topMargin: itemHeight
        }
        height: _top.height - itemHeight
        clip: true

        QML.Text {
            anchors.fill: parent
            horizontalAlignment: QML.Text.AlignHCenter
            verticalAlignment: QML.Text.AlignVCenter
            font.pixelSize: 22
            text:  KDE.i18n( "Please select one\nor more items\non the right." )
            visible: !_itemActionModel.hasSelection
        }

        PlasmaExtras.ScrollArea {
            anchors.fill:parent

            flickableItem: QML.ListView {
                id: actionListView
                model: actionModel

                delegate: KPIM.Action {
                    height: itemHeight
                    width: parent.width
                    action: application.getAction( model.action, "" )
                }
            }
        }
    }
}
