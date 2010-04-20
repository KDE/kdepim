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
import org.kde.pim.mobileui 4.5

Rectangle {
  id : _topContext
  color: "#00000000" // Set a transparant color.

  signal accountSelected( int row )

  Rectangle {
    color: "#00000000" // Set a transparant color.
    anchors.top : parent.top
    anchors.bottom : parent.bottom
    anchors.left : parent.left
    id : startColumn
    width : _topContext.width / 3
    Rectangle {
      id : startLabel
      color: "#00000000" // Set a transparant color.
      anchors.top : parent.top
      anchors.left : parent.left
      anchors.right : parent.right
      height : 200
      width : _topContext.width / 3
      Text { text : "Komo Start" }
    }
    Component {
      id : accountDelegate
      Item {
        id : _wrapper1
        height : 20
        Text { text : model.display; height : 20 }
        width : 100
        MouseArea {
          anchors.fill : parent

          onClicked : { console.log( "Account clicked: " + model.display ); _topContext.accountSelected( model.index ); }
        }
      }
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
    color: "#00000000" // Set a transparant color.
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
      Item {
        id : _wrapper2
        height : 20
        width : 100
        Text { text : model.text; height : 20; }
        MouseArea {
          anchors.fill : parent
          onClicked : { console.log("favorite clicked: " + model.index ); }
        }
      }
    }

    Text {
      id: favText;
      anchors.top : parent.top
      text : "Favorites"
      height : 20
      width : 100
    }
    Button {
      id : configureFavsButton
      anchors.top : parent.top
      anchors.left : favText.right
      width : 100
      height : 20
      buttonText : "Configure Favorites"
      onClicked : {
        console.log( "Configure clicked" );
      }
    }

    ListView {
      id : favsView
      anchors.top : favText.bottom
      anchors.left : favText.left
      height : 200
      width : 100
      model : favsModel
      delegate : favDelegate
    }
    Button {
      id : newEmailButton
      height : 20
      width : 200
      anchors.top : favsView.bottom
      buttonText : "Write new Email"
      onClicked : {
        console.log( "Write new clicked" );
      }

    }
    Button {
      id : newAccountButton
      anchors.top : newEmailButton.bottom
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
