/*
    Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

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
import org.kde.incidenceeditors 4.5 as IncidenceEditors

KPIM.MainView {

  SlideoutPanelContainer {
    anchors.fill: parent
    z: 50

    SlideoutPanel {
      anchors.fill: parent
      titleText: "More..."
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
              generalTab.visible = true
              attendeeTab.visible = false
              remindertabTab.visible = false
              recurrenceTab.visible = false
              attachmentTab.visible = false
            }
          }
          KPIM.Button {
            id: btnAttendees
            width: 64
            height: 64
            icon: KDE.iconPath( "view-pim-contacts", 64 )
            onClicked: {
              generalTab.visible = false
              attendeeTab.visible = true
              remindertabTab.visible = false
              recurrenceTab.visible = false
              attachmentTab.visible = false
            }
          }
          KPIM.Button {
            id: btnReminder
            width: 64
            height: 64
            icon: KDE.iconPath( "appointment-reminder", 64 )
            onClicked: {
              generalTab.visible = false
              attendeeTab.visible = false
              remindertabTab.visible = true
              recurrenceTab.visible = false
              attachmentTab.visible = false
            }
          }
          KPIM.Button {
            id: btnRecurrence
            width: 64
            height: 64
            icon: KDE.iconPath( "appointment-recurring", 64 )
            onClicked: {
              generalTab.visible = false
              attendeeTab.visible = false
              remindertabTab.visible = false
              recurrenceTab.visible = true
              attachmentTab.visible = false
            }
          }
          KPIM.Button {
            id: btnAttachment
            width: 64
            height: 64
            icon: KDE.iconPath( "mail-attachment", 64 )
            onClicked: {
              generalTab.visible = false
              attendeeTab.visible = false
              remindertabTab.visible = false
              recurrenceTab.visible = false
              attachmentTab.visible = true
            }
          }
        },

        // TODO: Add general tab, add attendees tab
        Rectangle {
          id: generalTab;
          color: "#ffffff"
          visible: true
          anchors.left: buttonColumn.right
          anchors.top: parent.top
          anchors.right: parent.right
          anchors.bottom: parent.bottom
        },
        Rectangle {
          id: attendeeTab;
          color: "#ffdddd"
          visible: false
          anchors.left: buttonColumn.right
          anchors.top: parent.top
          anchors.right: parent.right
          anchors.bottom: parent.bottom
        },
        Rectangle {
          id: remindertabTab;
          color: "#ffbbbb"
          visible: false
          anchors.left: buttonColumn.right
          anchors.top: parent.top
          anchors.right: parent.right
          anchors.bottom: parent.bottom
        },
        Rectangle {
          id: recurrenceTab;
          color: "#ff9999"
          visible: false
          anchors.left: buttonColumn.right
          anchors.top: parent.top
          anchors.right: parent.right
          anchors.bottom: parent.bottom
        },
        Rectangle {
          id: attachmentTab;
          color: "#ff7777"
          visible: false
          anchors.left: buttonColumn.right
          anchors.top: parent.top
          anchors.right: parent.right
          anchors.bottom: parent.bottom
        }

      ]
    }
  }

  Flickable {
    anchors.top: parent.top
    anchors.bottom: cancelButton.top
    anchors.left: parent.left
    anchors.right: parent.right
    
    anchors.topMargin: 40
    anchors.leftMargin: 40;
    anchors.rightMargin: 4;
    
    width: parent.width;
    height: parent.height - parent.height / 6 - collectionCombo.height;
    contentHeight: generalEditor.height;
    clip: true;
    flickableDirection: "VerticalFlick"

    Column {
      anchors.fill: parent
      IncidenceEditors.GeneralEditor {
        id: generalEditor;
        width: parent.width;
      }
    }
  }

  IncidenceEditors.CollectionCombo {
    id: collectionCombo
    anchors.bottom: parent.bottom;
    anchors.right: cancelButton.left;
    anchors.left: parent.left;
    
    width: parent.width;
    height: parent.height / 6;
  }

  KPIM.Button {
    id: cancelButton
    anchors.bottom: parent.bottom;
    anchors.right: okButton.left;
    width: 100;
    height: parent.height / 6;
    buttonText: KDE.i18n( "Cancel" );
    onClicked: window.cancel();
  }

  KPIM.Button {
    id: okButton;
    anchors.bottom: parent.bottom;
    anchors.right: parent.right;
    width: 100;
    height: parent.height / 6;
    buttonText: KDE.i18n( "Ok" );
    onClicked: window.save();
  }
}
