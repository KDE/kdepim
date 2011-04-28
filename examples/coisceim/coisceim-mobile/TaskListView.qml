/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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

import Qt 4.7 as QML
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM

KPIM.ItemListView {
  id: itemList
  property bool showCheckBox
  property variant checkModel
  property bool showCompletionSlider: true

  delegate: [
    KPIM.ItemListViewDelegate {
      id : listDelegate
      showCheckBox : itemList.showCheckBox
      checkModel : itemList.checkModel
      navigationModel : itemList.navigationModel
      height : itemListView.height / 7
      color: model.backgroundColor
      summaryContent: [
        QML.Text {
          id : summaryLabel
          anchors.top : parent.top
          anchors.topMargin : 1
          anchors.left : parent.left
          anchors.leftMargin : model.isSubTask ? 30 : 10
          anchors.right: parent.right
          anchors.rightMargin: completionSlider.width
          text: model.display
          color : "#0C55BB"
          font.pixelSize: 16
          elide: "ElideRight"
        },
        QML.Text {
          anchors.top : summaryLabel.bottom
          anchors.topMargin : 1
          anchors.left : parent.left
          anchors.leftMargin : model.isSubTask ? 30 : 10
          anchors.right: parent.right
          anchors.rightMargin: completionSlider.width
          height : 30;
//           text: KDE.i18n( "Details: %1", model.description )
          color: "#3B3B3B"
          font.pixelSize: 18
          elide: "ElideRight"
        },
        KPIM.CompletionSlider {
          id: completionSlider
          visible: showCompletionSlider
          width : 100
          anchors.top: parent.top
          anchors.right: parent.right
          onPercentageUpdated : {
            application.setPercentComplete(model.index, value);
          }
        },
        QML.Image {
          id : importantFlagImage
          anchors.verticalCenter : parent.verticalCenter;
          anchors.left : parent.left
          anchors.leftMargin : 15
          source : KDE.iconPath("emblem-important.png", parent.height + 16)
          opacity : model.is_important ? 0.25 : 0
        }
      ]
    }
  ]
}
