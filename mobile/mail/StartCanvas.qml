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

Rectangle {
  id : _topContext

  Rectangle {
    anchors.top : parent.top
    anchors.bottom : parent.bottom
    anchors.left : parent.left
    id : startColumn
    width : _topContext.width / 3
    Rectangle {
      id : startLabel
      anchors.top : parent.top
      anchors.left : parent.left
      anchors.right : parent.right
      height : 200
      width : _topContext.width / 3
      Text { text : "Komo Start" }
    }
    Component {
      id : accountDelegate
      Text { text : model.display; height : 20 }
    }
    ListView {
      id : accountsList
      anchors.top : startLabel.bottom
      anchors.left : parent.left
      anchors.right : parent.right
      height : 200
      model : accountsModel
      delegate : accountDelegate
    }
  }
  Rectangle {
    anchors.top : parent.top
    anchors.left : startColumn.right
    anchors.right : parent.right
    anchors.bottom : parent.bottom
    ListModel {
      id : favsModel
      ListElement { text : "Favorite 1" }
      ListElement { text : "Favorite 2" }
      ListElement { text : "Favorite 3" }
      ListElement { text : "Favorite 4" }
      ListElement { text : "Favorite 5" }
      ListElement { text : "Favorite 6" }
    }

    Component {
      id : favDelegate
      Text { text : model.text; height : 20; }
    }

    Rectangle {
      id : labelItem
      anchors.top : parent.top
      anchors.horizontalCenter : parent.horizontalCenter
      Row {
        spacing : 2
        height : parent.height
        Text { id: favText; text : "Favorites"; height : 20  }

        Button {
          id : configureFavsButton
          x: favText.width + 50
          width : 200
          height : 20
          buttonText : "Configure Favorites"
          onClicked : {
            console.log( "Configure clicked" );
          }
        }
      }
      height : 20
    }
    ListView {
      id : favsView
      anchors.top : labelItem.bottom
      anchors.horizontalCenter : parent.horizontalCenter
      height : 200
      model : favsModel
      delegate : favDelegate
    }
    Button {
      id : newEmailButton
      height : 20
      width : 200
      anchors.top : favsView.bottom
      anchors.horizontalCenter : parent.horizontalCenter
      buttonText : "Write new Email"
      onClicked : {
        console.log( "Write new clicked" );
      }

    }
    Button {
      id : newAccountButton
      anchors.top : newEmailButton.bottom
      anchors.horizontalCenter : parent.horizontalCenter
      height : 20
      width : 200
      buttonText : "Add Account"
      onClicked : {
        console.log( "Add Account clicked" );
        application.launchAccountWizard();
      }
    }
  }
}
