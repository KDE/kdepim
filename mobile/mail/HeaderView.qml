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

import Qt 4.7 as QML
import org.kde.pim.mobileui 4.5 as KPIM

/** Akonadi Message Header List View
 */
KPIM.ItemListView {
  delegate: [
    KPIM.ItemListViewDelegate {
      height : itemListView.height / 7
      summaryContent : [
        QML.Text {
          id : subjectLabel
          anchors.top : parent.top
          anchors.topMargin : 4
          anchors.left : parent.left
          anchors.leftMargin : 20
          text : model.from
          color : "#0C55BB"
        },
        QML.Text {
          anchors.top : subjectLabel.bottom
          anchors.topMargin : 4
          anchors.left : parent.left
          anchors.leftMargin : 20
          height : 30;
          text : model.subject
          // No indication of new yet. Possibly does not make sense on mobile anyway.
          color : (model.isUnread) ? "#E10909" : "#3B3B3B"
        },
        QML.Image {
          id : importantFlagImage
          anchors.verticalCenter : parent.verticalCenter;
          anchors.left : parent.left
          anchors.leftMargin : 15
          source : KDE.iconPath("emblem-important.png", parent.height + 16)
          opacity : model.is_important ? 0.25 : 0
        },
        QML.Image {
          id : actionFlagImage
          anchors.verticalCenter : parent.verticalCenter;
          anchors.left : importantFlagImage.right
          source : KDE.iconPath("mail-mark-task.png", parent.height + 16)
          opacity : model.is_action_item ? 0.25 : 0
        }
      ]
    }
  ]
}
