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

Item {
  anchors.topMargin: 12
  anchors.leftMargin: 48

  Text {
    id: subjectLabel
    text: KDE.i18n( "Subject:" );
    anchors.leftMargin: 48
    anchors.top: parent.top
    anchors.left: parent.left
  }

  TextInput {
    id: subject
    anchors.left: subjectLabel.right
    anchors.top: parent.top
    anchors.right: parent.right
  }

  TextEdit {
    id: messageContent
    anchors.top: subject.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
  }
}
