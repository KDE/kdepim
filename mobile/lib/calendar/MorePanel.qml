/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Artur Duque de Souza <asouza@kde.org>
    Copyright (C) 2010 Anselmo Lacerda Silveira de Melo <anselmolsm@gmail.com>
    Copyright (c) 2010 Eduardo Madeira Fleury <efleury@gmail.com>

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

import QtQuick 1.1
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.incidenceeditors 4.5 as IncidenceEditors

SlideoutPanel {
  titleText: KDE.i18n("More...");
  handlePosition: 250
  handleHeight: 120
  // TODO: Better icons for the buttons probably.
  content: [
    Column {
      id: buttonColumn
      width: 64;
      height: parent.height;
      KPIM.Button {
        id: btnGeneral
        width: 64
        height: 64
        icon: KDE.iconPath( "accessories-text-editor", 64 )
        onClicked: {
          moreEditors.currentIndex = 0
        }
      }
      KPIM.Button {
        id: btnAttendees
        width: 64
        height: 64
        icon: KDE.iconPath( "view-pim-contacts", 64 )
        onClicked: {
          moreEditors.currentIndex = 1
        }
      }
      KPIM.Button {
        id: btnReminder
        width: 64
        height: 64
        icon: KDE.iconPath( "appointment-reminder", 64 )
        onClicked: {
          moreEditors.currentIndex = 2
        }
      }
      KPIM.Button {
        id: btnRecurrence
        width: 64
        height: 64
        icon: KDE.iconPath( "appointment-recurring", 64 )
        onClicked: {
          moreEditors.currentIndex = 3
        }
      }
      KPIM.Button {
        id: btnAttachment
        width: 64
        height: 64
        icon: KDE.iconPath( "mail-attachment", 64 )
        onClicked: {
          moreEditors.currentIndex = 4
        }
      }
    },

    IncidenceEditors.MoreEditor {
      id : moreEditors
      anchors.top: parent.top
      anchors.bottom: parent.bottom
      anchors.left: buttonColumn.right
      anchors.right: parent.right
      currentIndex : 0
    }
  ]
}
