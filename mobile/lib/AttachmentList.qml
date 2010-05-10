/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM

/**
 * Shows a list view of the specified attachment model of a MessageView component.
 * @param model An attachment model
 */
Item {
  id: _attachmentList
  property alias model: attachmentListView.model
  property int rowHeight: 48
  property int attachmentListWidth: 300
  property int actionListWidth: 240
  property int requestedWidth: attachmentListWidth

  /** Emittted when an attachment has been selected. */
  signal attachmentSelected

  Component {
    id: attachmentDelegate

    Item {
      id: wrapper
      width: attachmentListWidth
      height: rowHeight
      clip: true

      Rectangle {
        id: background
        anchors.fill: parent
        opacity: (wrapper.ListView.isCurrentItem ? 0.25 : 0)
      }
      Text {
        anchors.fill: parent;
        text: model.display;
        horizontalAlignment: "AlignHCenter";
        verticalAlignment: "AlignVCenter";
      }

      MouseArea {
        anchors.fill: parent
        onClicked: {
          console.log( "TODO - attachment selected: " + model.display );
          var nonCurrentClicked = false
          if ( wrapper.ListView.view.currentIndex != model.index ) { nonCurrentClicked = true }
          wrapper.ListView.view.currentIndex = model.index
          wrapper.ListView.view.currentMimeType = model.mimeType;
          if ( model.mimeType.indexOf( "image" ) == 0 ) {
            wrapper.ListView.view.currentAttachmentUrl = model.attachmentUrl;
          }
          if ( nonCurrentClicked ) { attachmentSelected(); }
        }
      }
    }
  }

  ListView {
    property string currentMimeType
    property string currentAttachmentUrl

    id: attachmentListView
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: actionView.left
    height: Math.min( model.attachmentCount * rowHeight, parent.height )
    interactive: model.attachmentCount * rowHeight > parent.height
    delegate: attachmentDelegate
    clip: true

    Connections {
      target: model
      onModelReset: {
        attachmentListView.currentIndex = -1
        attachmentListView.currentMimeType = "";
        attachmentListView.currentAttachmentUrl = "";
      }
    }
  }

  Item {
    id: actionView
    visible: false
    width: 0
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.bottom: parent.bottom

    KPIM.Button {
      id: openButton
      anchors.top: parent.top
      anchors.horizontalCenter: parent.horizontalCenter;
      width: parent.width - 10
      height: parent.height / 6
      buttonText: KDE.i18n( "Open" )
    }
    KPIM.Button {
      id: saveButton
      anchors.top: openButton.bottom;
      anchors.horizontalCenter: parent.horizontalCenter;
      width: parent.width - 10
      height: parent.height / 6
      buttonText: KDE.i18n( "Save" )
    }

  }

  Item {
    id: previewView
    visible: false
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    width: _attachmentList.width - attachmentListWidth - 6

    Flickable {
      id: previewScrollArea
      anchors.fill: parent
      contentWidth: previewImage.width
      contentHeight: previewImage.height
      clip: true
      Image {
        id: previewImage
        source: attachmentListView.currentAttachmentUrl
      }
    }

    KPIM.Button {
      id: previewSaveButton
      anchors.bottom: parent.bottom
      anchors.right: parent.right
      anchors.margins: 12
      width: 48
      height: 48
      icon: KDE.iconPath( "document-save", width );
      states: [
        State {
          name: "movingState"
          when: previewScrollArea.moving
          PropertyChanges { target: previewSaveButton; opacity: 0.25 }
        }
      ]
      transitions: [
        Transition {
          NumberAnimation { properties: "opacity"; duration: 200 }
        }
      ]
    }
  }

  states: [
    State {
      name: "actionState"
      when: (attachmentListView.currentIndex >= 0 && attachmentListView.currentIndex < model.attachmentCount) && attachmentListView.currentMimeType.indexOf( "image" ) != 0
      PropertyChanges { target: actionView; width: actionListWidth; visible: true }
      PropertyChanges { target: _attachmentList; requestedWidth: attachmentListWidth + actionListWidth }
    },

    State {
      name: "previewState"
      when: (attachmentListView.currentIndex >= 0 && attachmentListView.currentIndex < model.attachmentCount) && attachmentListView.currentMimeType.indexOf( "image" ) == 0
      PropertyChanges { target: previewView; visible: true }
      PropertyChanges { target: _attachmentList; requestedWidth: 1000 } // limited by SlideoutPanel to maximum width anyway
    }
  ]

}
