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
  color: "#00000000" // Set a transparent color.
//   property int currentNoteRow : -1
  anchors.fill : parent

  property variant itemSelection

//   onCurrentNoteRowChanged : {
//     titleInput.text = itemSelection.noteTitle;
//     contentEdit.text = itemSelection.noteContent;
//   }

  function saveNote()
  {
//     application.saveNote(titleInput.text, contentEdit.text);
  }

  Rectangle {
    border { color: "grey"; width: 2; }
    color: "#00000000"
    id : titleWrapper
    anchors.top : parent.top
    anchors.left : parent.left
    anchors.right : parent.right
    height : 30
    radius : 8
    TextInput {
      id : titleInput
      text : itemSelection.noteTitle
      color : "#000001" // yes, not exactly black, since QML maps black to white on the N900...
      anchors.left : parent.left
      anchors.right: parent.right
      anchors.verticalCenter: parent.verticalCenter
      anchors.leftMargin : 10
      anchors.rightMargin : 10

      /*
      onClicked : {
        application.saveCurrentNoteContent(contentEdit.text);
      } */
    }

  }

  Rectangle {
    border { color: "grey"; width: 2; }
    color: "#00000000"
    anchors.top : titleWrapper.bottom
    anchors.left : parent.left
    anchors.right : parent.right
    anchors.bottom : parent.bottom
    anchors.topMargin: 8
    radius : 8
    Flickable {
      id: flick

      anchors.fill : parent
      anchors.topMargin : 8
      anchors.bottomMargin : 8
      anchors.leftMargin : 10
      anchors.rightMargin : 10

      clip: true

      function ensureVisible(r)
      {
          if (contentX >= r.x)
              contentX = r.x;
          else if (contentX+width <= r.x+r.width)
              contentX = r.x+r.width-width;
          if (contentY >= r.y)
              contentY = r.y;
          else if (contentY+height <= r.y+r.height)
              contentY = r.y+r.height-height;
      }

      TextEdit {
        id : contentEdit
        text : itemSelection.noteContent
        color : "#000001" // yes, not exactly black, since QML maps black to white on the N900...
        anchors.fill : parent

        wrapMode: TextEdit.Wrap
        onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)
      }
    }
  }
}
