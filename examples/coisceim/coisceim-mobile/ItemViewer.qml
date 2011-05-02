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
import org.kde.kcal 4.5 as KCal
import org.kde.messageviewer 4.5 as MessageViewer

Item {

  KCal.IncidenceView {
    id: eventView
    anchors.fill: parent

    visible : triplist.currentItem.trip ? triplist.currentItem.trip.todoSelection ? triplist.currentItem.trip.todoSelection.hasSelection : false : false

    itemId: triplist.currentItem.trip ? triplist.currentItem.trip.itemSelection.id : -1
  }

  NoteView {
    id: noteView
    objectName : "noteView"
    visible : triplist.currentItem.trip ? triplist.currentItem.trip.notesSelection ? triplist.currentItem.trip.notesSelection.hasSelection : false : false
    anchors.fill: parent

    itemSelection : triplist.currentItem.trip ? triplist.currentItem.trip.itemSelection : null

    Rectangle {
      anchors.top : noteView.top
      anchors.bottom : noteView.bottom
      anchors.right : noteView.left
      width : noteView.anchors.leftMargin
      color : "#FAFAFA"
    }
  }

  MessageViewer.MessageView {
    id: messageView
    visible : triplist.currentItem.trip ? triplist.currentItem.trip.mailSelection ? triplist.currentItem.trip.mailSelection.hasSelection : false : false
    anchors.fill: parent
    itemId: triplist.currentItem.trip ? triplist.currentItem.trip.itemSelection.id : -1
  }

  Rectangle {
    id : backToMessageListButton
    visible: triplist.currentItem.trip.itemSelection.id != -1
    anchors.right : parent.right
    anchors.rightMargin : 70
    anchors.bottom : parent.bottom
    anchors.bottomMargin : 100
    Image {
      source : KDE.locate( "data", "mobileui/back-to-list-button.png" );
      MouseArea {
        anchors.fill : parent;
        onClicked : {
          console.log(triplist.currentItem.trip, triplist.currentItem.trip.itemSelection)
          triplist.currentItem.trip.itemSelection.clear()
        }
      }
    }
  }


}
