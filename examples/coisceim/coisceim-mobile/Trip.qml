/*
    This file is part of Akonadi.

    Copyright (c) 2011 Stephen Kelly <steveire@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

import Qt 4.7
import org.kde.pim.mobileui 4.5 as KPIM

Item {

  property variant trip

  ItemViewer {
    anchors.fill : parent
  }

  Item {
    anchors.fill : parent

    visible: triplist.currentItem.trip.itemSelection.id == -1

    Item {
      anchors.top : parent.top
      anchors.left : parent.left
      anchors.right : parent.right
      anchors.bottom : selector.top

      NotesListView {
        id: mailView
        anchors.left : parent.left
        anchors.top : parent.top
        anchors.bottom : parent.bottom
        width : parent.width / (navList.currentIndex == 0 ? 2 : 4)
        showDeleteButton : false
        navigationModel : trip ? trip.mailSelection : null
        model: trip ? trip.mailModel : null

        Behavior on width {
          NumberAnimation {}
        }
      }

      Rectangle {
        anchors.left : mailView.right
        anchors.top : parent.top
        anchors.bottom : parent.bottom
        width : 1
        color : "grey"
      }

      TaskListView {
        id: todoView
        anchors.left : mailView.right
        anchors.top : parent.top
        anchors.bottom : parent.bottom
        width : parent.width / (navList.currentIndex == 1 ? 2 : 4 )
        navigationModel : trip ? trip.todoSelection : null
        model: trip ? trip.todoModel : null

        Behavior on width {
          NumberAnimation {}
        }
      }

      Rectangle {
        anchors.left : todoView.right
        anchors.top : parent.top
        anchors.bottom : parent.bottom
        width : 1
        color : "grey"
      }

      NotesListView {
        id: noteList
        anchors.left : todoView.right
        anchors.right : parent.right
        anchors.top : parent.top
        anchors.bottom : parent.bottom
        showDeleteButton : false
        navigationModel : trip ? trip.notesSelection : null
        model: trip ? trip.notesModel : null
      }
    }
    Item {
      id : selector
      anchors.left : parent.left
      anchors.right : parent.right
      anchors.bottom : parent.bottom
      height : 35

      ListView {
        id : navList
        orientation : ListView.Horizontal
        anchors.centerIn : parent
        width : 200
        height : 30
        model : 3
        delegate :
          Item {
            y : 20
            height : 10
            width : ListView.view.width / ListView.view.count
            property bool current : ListView.isCurrentItem

            KPIM.Button2 {
              height : 30
              width : 30
              anchors.bottom : parent.bottom
              _state : current ? "-active" : ""
              onClicked : {
                ListView.view.currentIndex = model.index
              }
            }
          }
      }
    }
  }
}
