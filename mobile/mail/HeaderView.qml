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

import QtQuick 1.1 as QML
import org.kde.pim.mobileui 4.5 as KPIM

/** Akonadi Message Header List View
 */
KPIM.ItemListView {
  id : _top
  property bool showDeleteButton : false
  property bool showCheckBox
  property variant checkModel

  delegate: [
    KPIM.ItemListViewDelegate {
      id : _delegate
      showCheckBox : _top.showCheckBox
      checkModel : _top.checkModel
      navigationModel : _top.navigationModel
      height: _top.itemHeight
      clip: true
      summaryContent : [
        QML.Text {
          id: fromLabel
          anchors.top : parent.top
          anchors.topMargin : 1
          anchors.left : parent.left
          anchors.leftMargin : 10
          text : model.from
          color : "#0C55BB"
          font.pixelSize: 16
          elide: "ElideRight"
          width: parent.width - dateLabel.width - anchors.leftMargin - dateLabel.anchors.rightMargin
        },
        QML.Row {
          anchors.top: parent.top
          anchors.right: dateLabel.left
          anchors.rightMargin: 5
          QML.Image {
            width: 22
            height: 22
            source: KDE.locate( "data", "libmessageviewer/pics/mobile_status_important.png" )
            visible: model.is_important
          }
          QML.Image {
            width: 22
            height: 22
            source: KDE.locate( "data", "libmessageviewer/pics/mobile_status_actionitem.png" )
            visible: model.is_action_item
          }
          QML.Image {
            width: 22
            height: 22
            source: KDE.locate( "data", "libmessageviewer/pics/mobile_status_signed.png" )
            visible: model.is_signed
          }
          QML.Image {
            width: 22
            height: 22
            source: KDE.locate( "data", "libmessageviewer/pics/mobile_status_encrypted.png" )
            visible: model.is_encrypted
          }
          QML.Image {
            width: 22
            height: 22
            source: KDE.locate( "data", "libmessageviewer/pics/mobile_status_attachment.png" )
            visible: model.has_attachment
          }
          QML.Image {
            width: 22
            height: 22
            source: KDE.locate( "data", "libmessageviewer/pics/mobile_status_replied.png" )
            visible: model.is_replied
          }
          QML.Image {
            width: 22
            height: 22
            source: KDE.locate( "data", "libmessageviewer/pics/mobile_status_forwarded.png" )
            visible: model.is_forwarded
          }
        },
        QML.Text {
          id: dateLabel
          anchors { top: parent.top; topMargin: 1; right: parent.right; rightMargin: deleteAction.width }
          text: model.date
          color: "#0C55BB"
          font.pixelSize: 16
          horizontalAlignment: "AlignRight"
        },
        QML.Text {
          id: sizeLabel
          visible:  model.threadSize == undefined || model.threadSize <= 1
          anchors { bottom: parent.bottom; bottomMargin: 1; right: parent.right; rightMargin: deleteAction.width }
          text: model.size
          color: "#0C55BB"
          font.pixelSize: 16
          horizontalAlignment: "AlignRight"
        },
        QML.Text {
          id: subjectLabel
          anchors.top : fromLabel.bottom
          anchors.topMargin : 1
          anchors.left : parent.left
          anchors.leftMargin : 10
          height : 30;
          width: parent.width - (threadInfoLabel.visible ? threadInfoLabel.width : 0) - anchors.leftMargin - threadInfoLabel.anchors.rightMargin
          text : model.subject
          font.pixelSize: 18
          color : model.is_unread ? "#E10909" : "#3B3B3B"
          elide: "ElideRight"
        },
        QML.Text {
          id : threadInfoLabel
          visible : model.threadSize != undefined && model.threadSize > 1
          anchors.top : fromLabel.bottom
          anchors.topMargin : 1
          anchors.right : parent.right
          anchors.rightMargin: deleteAction.width
          height : (model.threadSize != undefined) ? 30 : 0
          font.pixelSize: 18
          text : model.threadUnreadCount > 0 ? KDE.i18ncp("This text is only visible if messages > 1", "%2 messages, %1 unread", "%2 messages, %1 unread",
                                                           model.threadUnreadCount, model.threadSize)
                                             : KDE.i18np( "One message", "%1 messages", model.threadSize );
        },
        KPIM.Action{
          id : deleteAction
          anchors.verticalCenter: parent.verticalCenter;
          anchors.right : parent.right;
          width: (showDeleteButton || showCheckBox) ? imageWidth : 0
          height : imageHeight
          action : application.getAction("akonadi_move_to_trash", "")
          hidable : false
          showText : false
          disableable : false
          opacity : 0.6
          visible : showDeleteButton
          onTriggered : {
            _itemActionModel.select(model.index, 3);
          }
          image : KDE.locate( "data", "mobileui/delete-button.png" );
        }
      ]

      states : [
        QML.State {
          name : "deleteFaded"
          when : itemListView.flicking
          QML.PropertyChanges {
            target : deleteAction;
            opacity : 0
          }
          QML.PropertyChanges {
            target : deleteAction.anchors;
            rightMargin : -deleteAction.width
          }
        }
      ]
      transitions : [
        QML.Transition {
          from : ""
          to   : "deleteFaded"
          QML.PropertyAnimation {
            target : deleteAction
            properties : "opacity"
            duration: 500
            easing.type: "OutQuad"
          }
        },
        QML.Transition {
          from : "deleteFaded"
          to   : ""
          QML.SequentialAnimation {
            QML.PauseAnimation {
              // delay a bit
              duration: {
                // TODO: figure out how to do this.
                0
              }
            }
            QML.PropertyAnimation {
              target : deleteAction.anchors
              properties : "rightMargin"
              duration: 500
              easing.type: "InQuad"
            }
          }
        }
      ]
    }
  ]

}
