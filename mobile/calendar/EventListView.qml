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

/** Akonadi Event List View
 */
KPIM.ItemListView {
  id : _top
  property bool showCheckBox
  property variant checkModel

  delegate: [
    KPIM.ItemListViewDelegate {
      id : _delegate
      showCheckBox : _top.showCheckBox
      checkModel : _top.checkModel
      navigationModel : _top.navigationModel
      height : _top.itemHeight
      summaryContent : [
        QML.Text {
          id: fromLabel
          anchors.top : parent.top
          anchors.topMargin : 1
          anchors.left : parent.left
          anchors.leftMargin : 10
          text : KDE.i18n( "%1 (%2)", model.begin, model.duration );
          color : "#0C55BB"
          font.pixelSize: 16
          elide: "ElideRight"
          width: parent.width - anchors.leftMargin - anchors.rightMargin
        },
        QML.Text {
          id: subjectLabel
          anchors.top : fromLabel.bottom
          anchors.topMargin : 1
          anchors.left : parent.left
          anchors.leftMargin : 10
          anchors.right: parent.right
          height : 30;
          text : model.summary
          font.pixelSize: 18
          elide: "ElideRight"
        }
      ]
    }
  ]
}
