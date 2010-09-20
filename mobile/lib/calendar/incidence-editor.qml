/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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

  Connections {
    target: calendarWidget
    onCollapsed: {
        calendarWidget.visible = false;
        _incidenceview.setNewDate(myCalendar.day, myCalendar.month, myCalendar.year);
    }
  }

  Connections {
    target: _incidenceview
    onShowCalendarWidget: {
        calendarWidget.expand();
        calendarWidget.visible = true;
        myCalendar.day = day;
        myCalendar.month = month;
        myCalendar.year = year;
    }
  }

  Connections {
    target: clockWidget
    onCollapsed: {
        clockWidget.visible = false;
        _incidenceview.setNewTime(myClock.hours, myClock.minutes, myClock.seconds);
    }
  }

  Connections {
    target: _incidenceview
    onShowClockWidget: {
        clockWidget.expand();
        clockWidget.visible = true;
        myClock.hours = hour;
        myClock.minutes = minute;
        myClock.seconds = second;
    }
  }

  SlideoutPanelContainer {
    anchors.fill: parent
    z: 50

    SlideoutPanel {
      id: calendarWidget
      anchors.fill: parent
      titleText: KDE.i18n("Date");
      handlePosition: 150
      handleHeight: 120
      visible: false
      content: [
          Row {
            spacing: 15

            KPIM.Calendar {
              id: myCalendar
            }

            Column {
              spacing: 10
              anchors.top: parent.top
              anchors.topMargin: 50

              KPIM.VerticalFadeSelector {
                id: daySelector
                height: 100
                model: myCalendar.daysInMonth
                currentIndex: myCalendar.day
                onValueChanged: myCalendar.day = value;
              }

              KPIM.VerticalFadeSelector {
                id: monthSelector
                height: 100
                model: 12
                currentIndex: myCalendar.month
                onValueChanged: myCalendar.month = value;
              }

              KPIM.VerticalFadeSelector {
                id: yearSelector
                height: 100
                // high enough :)
                model: 3000
                currentIndex: myCalendar.year
                onValueChanged: myCalendar.year = value;
              }
            }
          }
      ]
    }

    SlideoutPanel {
      id: clockWidget
      anchors.fill: parent
      titleText: KDE.i18n("Time");
      handlePosition: 150
      handleHeight: 120
      visible: false
      content: [
          Row {
            spacing: 15

            KPIM.Clock {
              id: myClock
            }

            Column {
              spacing: 10
              anchors.top: parent.top
              anchors.topMargin: 50

              KPIM.VerticalFadeSelector {
                id: hourSelector
                height: 100
                model: 24
                currentIndex: myClock.hours
                onValueChanged: myClock.hours = value;
              }

              KPIM.VerticalFadeSelector {
                id: minuteSelector
                height: 100
                model: 60
                currentIndex: myClock.minutes
                onValueChanged: myClock.minutes = value;
              }

              KPIM.VerticalFadeSelector {
                id: secondSelector
                height: 100
                model: 60
                currentIndex: myClock.seconds
                onValueChanged: myClock.seconds = value;
              }
            }
          }
      ]
    }

    SlideoutPanel {
      anchors.fill: parent
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
  }

  Flickable {
    anchors.top: parent.top
    anchors.bottom: collectionCombo.top
    anchors.left: parent.left
    anchors.right: parent.right
    
    anchors.topMargin: 40
    anchors.leftMargin: 40;
    anchors.rightMargin: 10;

    contentHeight: generalEditor.height;
    clip: true;
    flickableDirection: "VerticalFlick"

    Item {
      IncidenceEditors.GeneralEditor {
        id: generalEditor;
        anchors.fill: parent
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

  KPIM.Button2 {
    id: cancelButton
    anchors.bottom: parent.bottom;
    anchors.right: okButton.left;
    width: 100;
    buttonText: KDE.i18n( "Cancel" );
    onClicked: window.cancel();
  }

  KPIM.Button2 {
    id: okButton;
    anchors.bottom: parent.bottom;
    anchors.right: parent.right;
    width: 100;
    buttonText: KDE.i18n( "Ok" );
    onClicked: window.save();
  }
}
