/*
    Copyright (c) 2010 Kevin Krammer <kevin.krammer@gmx.at>

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
import org.kde.contacteditors 4.5 as ContactEditors

KPIM.MainView {
  Flickable {
    anchors.top: parent.top
    anchors.bottom: cancelButton.top
    anchors.left: parent.left
    anchors.right: parent.right

    anchors.topMargin: 40
    anchors.leftMargin: 40;
    anchors.rightMargin: 4;

    width: parent.width;
    height: parent.height;
    contentHeight: editorGeneral.height;
    clip: true;
    flickableDirection: "VerticalFlick"

    Column {
      anchors.fill: parent
      ContactEditors.ContactEditorGeneral {
        id: editorGeneral;
        width: parent.width;
      }
    }
  }
}
