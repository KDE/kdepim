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
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.akonadi 4.5

Item {
  id : _topContext
  property alias startText: startText.text
  property alias contextActions: contextContainer.children
  property alias favoritesModel : favsView.model
  property alias showAccountsList : accountsList.visible

  signal accountSelected( int row )
  signal favoriteSelected( string favName )

  Component {
    id : accountDelegate
    CollectionDelegate {
      fullClickArea : true
      height : accountsList.height / 6
      width : accountsList.width
      onIndexSelected : { _topContext.accountSelected( model.index ); }
    }
  }

  Component {
    id : favDelegate
    CollectionDelegate {
      fullClickArea : true
      height : _topContext.height / 6
      width : ListView.view.width
      onIndexSelected : {
        favoriteSelected(model.display);
      }
    }
  }

  Row {
    anchors.topMargin: 30
    anchors.left: parent.left
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.bottom: firstStepsLink.top
    spacing: 10

    Column {
      width: accountsList.visible ? (parent.width) : 0
      height: parent.height
      spacing: 5

      Text {
        width: parent.width
        id: startText
        font {
          pointSize: 14
        }
      }

      KPIM.DecoratedListView {
        id : accountsList
        clip: true
        width: parent.width
        height : parent.height
        model : accountsModel
        delegate : accountDelegate
      }
    }

    Column {
      width: parent.width
      height: parent.height
      spacing: 5

      Item {
        id : contextContainer
        height: childrenRect.height
        width: parent.width
      }

      KPIM.DecoratedListView {
        id : favsView
        width: parent.width
        height: parent.height - contextContainer.height - 2 - 4 * 5
        delegate : favDelegate
        clip: true
      }
    }
  }

  Text {
    id: firstStepsLink
    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    anchors.bottomMargin: visible ? 20 : 0
    text: KDE.i18n( "<a href=\"foobar\">First steps...</a>" )
    textFormat: Text.RichText
    verticalAlignment: Text.AlignTop
    MouseArea {
      anchors.fill: parent
      onClicked: { application.openManual() }
    }

    visible: favsView.count < 4
    height: visible ? 20 : 0
    y: visible ? parent.height - height : parent.height
  }
}
