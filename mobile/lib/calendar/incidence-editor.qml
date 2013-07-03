/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Artur Duque de Souza <asouza@kde.org>
    Copyright (C) 2010 Anselmo Lacerda Silveira de Melo <anselmolsm@gmail.com>

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

KPIM.MainView {
  id: mainview

  Connections {
    target: _incidenceview
    onShowCalendarWidget: {
       calendarWidget.expand()
       calendarWidget.okEnabled = false

       calendarWidget.day = day;
       calendarWidget.month = month;
       calendarWidget.year = year;
    }
  }

  Connections {
    target: _incidenceview
    onShowClockWidget: {
       clockWidget.expand()
       clockWidget.okEnabled = false

       // set the initial values
       clockWidget.hours = hour;
       clockWidget.minutes = minute;
    }
  }

  Connections {
    target: clockWidget
    onTimeChanged: {
      _incidenceview.setNewTime( hour, minute );
    }
  }

  Connections {
    target: calendarWidget
    onDateChanged: {
      _incidenceview.setNewDate( day, month, year );
    }
  }

  KPIM.MorePanel {
    anchors.fill: parent
  }

  KPIM.CalendarDialog {
    id: calendarWidget
    anchors.fill: parent
  }

  KPIM.ClockDialog {
    id: clockWidget
    anchors.fill: parent
  }

  KPIM.DecoratedFlickable {
    anchors.top: parent.top
    anchors.bottom: collectionCombo.top
    anchors.left: parent.left
    anchors.right: parent.right

    anchors.topMargin: 40
    anchors.leftMargin: 40;

    contentHeight: generalEditor.height;

    content.children: [
      Item {
        IncidenceEditors.GeneralEditor {
          id: generalEditor;
          anchors.fill: parent
        }
      }
    ]
  }

  IncidenceEditors.CollectionCombo {
    id: collectionCombo
    anchors.bottom: parent.bottom;
    anchors.right: cancelButton.left;
    anchors.left: parent.left;

    height: parent.height / 6;
  }

  KPIM.Button2 {
    id: cancelButton
    anchors.bottom: parent.bottom;
    anchors.right: okButton.left;
    width: height * 1.5;
    height: collectionCombo.height
    icon: KDE.iconPath( "dialog-cancel", 64 );
    onClicked: window.cancel();
  }

  KPIM.Button2 {
    id: okButton;
    anchors.bottom: parent.bottom;
    anchors.right: parent.right;
    width: height * 1.5;
    height: collectionCombo.height
    icon: KDE.iconPath( "document-save", 64 );
    onClicked: window.save();
  }
}
