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

import Qt 4.7
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.akonadi 4.5

Item {
  id : _topContext
  property alias startText: startText.text
  property alias contextActions: contextContainer.children
  property alias favoritesModel : favsView.model

  signal accountSelected( int row )

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
        onClicked : { console.log("favorite clicked: " + model.index ); }
      }
    }
  }

  Row {
    anchors.topMargin: 30
    anchors.fill: parent
    spacing: 10

    Column {
      width: parent.width / 3
      height: parent.height
      spacing: 5

      Text {
        width: parent.width
        id: startText
        font {
          bold: true
          pointSize: 14
        }
      }

      ListView {
        id : accountsList
        clip: true
        width: parent.width
        height : parent.height
        model : accountsModel
        delegate : accountDelegate
      }
    }

    Column {
      width: parent.width / 3 * 2
      height: parent.height
      spacing: 5

      Item {
        id : contextContainer
        height: childrenRect.height
        width: parent.width
      }

      Rectangle {
        color: "gray"
        width: parent.width
        height: 2
      }

      Text {
        id: favoritesText
        width: parent.width
        height: 14
        text: "Favorites:"
        font.bold: true
      }

      ListView {
        id : favsView
        width: parent.width
        height: parent.height - contextContainer.height - favoritesText.height - 2 - 4 * 5
        delegate : favDelegate
        clip: true
      }
    }
  }
}
