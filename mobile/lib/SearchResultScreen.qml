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

import Qt 4.7 as QML
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.akonadi 4.5 as Akonadi

QML.Rectangle {
  property alias itemView: itemView.children

  id: searchResultView
  visible: guiStateManager.inSearchResultScreenState
  color : "white"

  QML.Item {
    id: collectionView
    anchors.left : parent.left
    anchors.top : parent.top
    width: 1/3 * parent.width
    height : 65
    QML.ListView {
      id : selectedItem
      anchors.fill : parent
      model : _breadcrumbNavigationFactory.qmlSelectedItemModel();
      delegate : Akonadi.CollectionDelegate {
        height : 70
        indentation : 80
      }
    }
    QML.Image {
      id : topLine
      source : "images/list-line-top.png"
      anchors.right : selectedItem.right
      anchors.top : selectedItem.top
    }
    QML.Image {
      id : topLineFiller
      source : "images/dividing-line-horizontal.png"
      anchors.right : topLine.left
      anchors.bottom : topLine.bottom
      fillMode : QML.Image.TileHorizontally
      width : parent.width - topLine.width
    }
    QML.Image {
      id : bottomLine
      source : "images/dividing-line-horizontal.png"
      anchors.right : selectedItem.right
      anchors.bottom : selectedItem.bottom
      fillMode : QML.Image.TileHorizontally
      width : parent.width
    }
    QML.Image {
      source : "images/dividing-line.png"
      anchors.top : selectedItem.bottom
      anchors.right : parent.right
      height : searchResultView.height - selectedItem.height
      fillMode : QML.Image.TileVertically
    }
    QML.Image {
      source : "images/bulk-forward-overlay.png"
      anchors.right : parent.right
      anchors.verticalCenter : parent.verticalCenter
      QML.MouseArea {
        anchors.fill : parent
        onClicked: guiStateManager.pushState( KPIM.GuiStateManager.BulkActionScreenState );
      }
    }
  }

  KPIM.Button2 {
    id: backButton
    anchors.left: collectionView.left
    anchors.right: collectionView.right
    anchors.bottom: parent.bottom
    buttonText: KDE.i18n( "Back to Search" )
    onClicked: {
      searchManager.stopSearch()
    }
  }

  QML.Rectangle {
    id: itemView
    anchors.left: collectionView.right
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    anchors.right: parent.right
    color : "#00000000"
  }
}
