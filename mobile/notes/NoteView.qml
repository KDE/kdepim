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
  color: "#00000000" // Set a transparant color.
  property int noteId : -1
  anchors.fill : parent

  onNoteIdChanged : {
    titleInput.text = application.noteTitle( noteId );
    contentEdit.text = application.noteContent( noteId );
  }

  Rectangle {
    border.color : "blue"
    color: "#00000000"
    id : titleWrapper
    anchors.top : parent.top
    anchors.left : parent.left
    anchors.right : parent.right
    height : 30
    radius : 5
    TextEdit {
      id : titleInput
      anchors.fill : parent
      anchors.topMargin : 8
      anchors.leftMargin : 10
      anchors.rightMargin : 10

  //    text : note.title
    }

  }

  Rectangle {
    border.color : "blue"
    color: "#00000000"
    anchors.top : titleWrapper.bottom
    anchors.left : parent.left
    anchors.right : parent.right
    anchors.bottom : parent.bottom
    radius : 5
    TextEdit {
      id : contentEdit
      anchors.fill : parent
      anchors.topMargin : 8
      anchors.bottomMargin : 8
      anchors.leftMargin : 10
      anchors.rightMargin : 10

//       text : note.content
    }
  }
}
