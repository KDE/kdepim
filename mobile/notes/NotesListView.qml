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

import QtQuick 1.0
import org.kde.pim.mobileui 4.5 as KPIM

/** Akonadi Note List View
 */
KPIM.ItemListView {
  id: _top
  property bool showDeleteButton : true
  property bool showCheckBox
  property variant checkModel

  delegate: [
    KPIM.ItemListViewDelegate {
      showCheckBox : _top.showCheckBox
      checkModel : _top.checkModel
      navigationModel : _top.navigationModel
      height : _top.itemHeight
      summaryContent : [
        Text {
          id: titleLabel
          anchors.top : parent.top
          anchors.topMargin : 1
          anchors.left : parent.left
          anchors.leftMargin : 10
          anchors.right: parent.right
          anchors.rightMargin: deleteAction.width
          text : model.title
          color : "#0C55BB"
          font.pixelSize: 16
          elide: "ElideRight"
        },
        Text {
          id: subjectLabel
          anchors.top : titleLabel.bottom
          anchors.topMargin : 1
          anchors.left : parent.left
          anchors.leftMargin : 10
          anchors.right: parent.right
          anchors.rightMargin: deleteAction.width
          height : 30;
          text : model.shortContent
          font.pixelSize: 18
          elide: "ElideRight"
          clip: true
        }, /*
        Image {
          id : importantFlagImage
          anchors.verticalCenter : parent.verticalCenter;
          anchors.left : parent.left
          anchors.leftMargin : 15
          source : "important-email.png"
          opacity : model.is_important ? 0.25 : 0
        },
        Image {
          id : actionFlagImage
          anchors.verticalCenter : parent.verticalCenter;
          anchors.left : importantFlagImage.right
          source : "action-item-email.png"
          opacity : model.is_action_item ? 0.25 : 0
        }, */
        KPIM.Action{
          id : deleteAction
          anchors.verticalCenter: parent.verticalCenter;
          anchors.right : parent.right;
          width: imageWidth
          height : imageHeight
          action : application.getAction("akonadi_item_delete", "")
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
        State {
          name : "deleteFaded"
          when : itemListView.flicking
          PropertyChanges {
            target : deleteAction;
            opacity : 0
          }
          PropertyChanges {
            target : deleteAction.anchors;
            rightMargin : -deleteAction.width
          }
        }
      ]
      transitions : [
        Transition {
          from : ""
          to   : "deleteFaded"
          PropertyAnimation {
            target : deleteAction
            properties : "opacity"
            duration: 500
            easing.type: "OutQuad"
          }
        },
        Transition {
          from : "deleteFaded"
          to   : ""
          SequentialAnimation {
            PauseAnimation {
              // delay a bit
              duration: {
                // TODO: figure out how to do this.
                0
              }
            }
            PropertyAnimation {
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
