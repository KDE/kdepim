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
//       id: startPanel
      titleIcon: KDE.iconPath( "view-pim-contacts", 48 )
      handlePosition: 30
      handleHeight: 120
    }
    SlideoutPanel {
      anchors.fill: parent
//       id: startPanel
      titleIcon: KDE.iconPath( "appointment-recurring", 48 )
      handlePosition: 30 + 120
      handleHeight: 120
    }
    SlideoutPanel {
      anchors.fill: parent
//       id: startPanel
      titleIcon: KDE.iconPath( "mail-attachment", 48 )
      handlePosition: 30 + 120 + 120
      handleHeight: 100
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
    contentHeight: generalEditor.height + dateTimeEditor.height;
    clip: true;
    flickDirection: "VerticalFlick"

    Column {
      anchors.fill: parent
      IncidenceEditors.GeneralEditor {
        id: generalEditor;
        width: parent.width;
      }

      IncidenceEditors.DateTimeEditor {
        id: dateTimeEditor;
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