/*
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (C) 2010 Volker Krause <vkrause@kde.org>

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
import org.kde.pim.mobileui 4.5

QML.Item {
  id: _delegateTopLevel
  clip: true
  width: QML.ListView.width
  height: 100

  QML.Rectangle {
    id: background
    anchors.fill: parent
   opacity: (_delegateTopLevel.QML.ListView.isCurrentItem ? 0.25 : 0)
    color: "lightsteelblue"
  }

  QML.Text {
    anchors.fill: parent
    text : model.display
    horizontalAlignment: QML.Text.AlignHCenter
    verticalAlignment: QML.Text.AlignVCenter
  }

  QML.MouseArea {
    anchors.fill: parent
    onClicked: { _delegateTopLevel.QML.ListView.view.currentIndex = model.index; }
  }
}
