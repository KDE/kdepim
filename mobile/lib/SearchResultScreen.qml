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
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.akonadi 4.5 as Akonadi

QML.Rectangle {
  property alias itemView: itemView.children
  property alias resultText: collectionView.multipleSelectionText

  id: searchResultView
  visible: guiStateManager.inSearchResultScreenState
  color: "white"

  Akonadi.AkonadiBreadcrumbNavigationView {
    id: collectionView
    anchors.left: parent.left
    anchors.top: parent.top
    anchors.bottom: backButton.top
    width: 1/3 * parent.width

    breadcrumbComponentFactory: _breadcrumbNavigationFactory

    onHomeClicked: {
      searchManager.stopSearch()
      guiStateManager.popState()

      // the user clicked on home, so explicitly change the state to it
      guiStateManager.switchState( KPIM.GuiStateManager.HomeScreenState )
    }

    KPIM.AgentStatusIndicator {
      anchors { top: parent.top; right: parent.right; rightMargin: 10; topMargin: 10 }
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
    color: "#00000000"
  }
}
