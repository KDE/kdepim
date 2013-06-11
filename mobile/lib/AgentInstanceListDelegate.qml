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

import QtQuick 1.1 as QML
import org.kde 4.5
import org.kde.pim.mobileui 4.5

QML.Item {
  id: _delegateTopLevel
  clip: true
  width: QML.ListView.width
  height: 100

  function iconForStatus( online, status )
  {
    if ( !online ) {
      return "images/status/offline.png";
    }

    if ( status == 0 ) {
      return "images/status/online.png"
    } else if ( status == 1 ) {
      return "images/status/receiving.png";
    } else {
      return KDE.iconPath( "dialog-warning", 26 );
    }
  }

  function statusMessage( msg, progress )
  {
    if ( progress <= 0 || progress >= 100 )
      return msg;
    return KDE.i18nc( "status message (50%)", "%1 (%2%)", msg, progress );
  }

  QML.Rectangle {
    id: background
    anchors.fill: parent
    opacity: (_delegateTopLevel.QML.ListView.isCurrentItem ? 0.25 : 0)
    color: "lightsteelblue"
  }

  QML.Item {
    anchors.margins: 4
    anchors.fill: parent

    QML.Image {
      id: statusIcon
      anchors { top: parent.top; left: parent.left }
      source: iconForStatus( model.online, model.status )
    }

    QML.Text {
      id: nameLabel
      anchors { verticalCenter: statusIcon.verticalCenter; left: statusIcon.right }
      width: parent.width - statusIcon.width
      elide: QML.Text.ElideRight
      text : model.display
    }

    QML.Text {
      id: statusLabel
      anchors { left: nameLabel.left; top: nameLabel.bottom; topMargin: 2 }
      width: nameLabel.width
      font.pixelSize: 16
      elide: QML.Text.ElideRight
      text: statusMessage( model.statusMessage, model.progress )
    }

  }

  QML.MouseArea {
    anchors.fill: parent
    onClicked: { _delegateTopLevel.QML.ListView.view.currentIndex = model.index; }
  }
}
