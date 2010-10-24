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

import Qt 4.7
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.incidenceeditors 4.5 as IncidenceEditors

KPIM.MainView {
  id: mainview

  Connections {
    target: _incidenceview
    onShowCalendarWidget: {
        calendarWidget.expand()
        calendarWidgetOk.enabled = false

        myCalendar.day = day;
        myCalendar.month = month;
        myCalendar.year = year;
    }
  }

  Connections {
    target: _incidenceview
    onShowClockWidget: {
        clockWidget.expand()
        clockWidgetOk.enabled = false

        // set the initial values
        myClock.hours = hour;
        myClock.minutes = minute;
    }
  }

  SlideoutPanelContainer {
    anchors.fill: parent
    z: 50

  Dialog {
    id: calendarWidget
    anchors.fill: parent
    content: [
      Item {
        anchors.fill: parent

          KPIM.Calendar {
            id: myCalendar
            anchors {
              left: parent.left
              top: parent.top
              bottom: parent.bottom
            }
          }

          Column {
            spacing: 5
            anchors {
              top: parent.top
              left: myCalendar.right
              right: parent.right

              topMargin: 20
            }

            KPIM.VerticalSelector {
              id: daySelector
              height: 100
              model: myCalendar.daysInMonth
              beginWith: 1

              onValueChanged: {
                // selector change -> update calendar
                myCalendar.day = value;
                calendarWidgetOk.enabled = true;
              }
              onSelected: {
                monthSelector.state = "unselected";
                yearSelector.state = "unselected";
              }
            }

            KPIM.VerticalSelector {
              id: monthSelector
              height: 100
              model: 12
              beginWith: 1

              value: myCalendar.month
              onValueChanged: {
                // selector change -> update calendar
                myCalendar.month = value;
                calendarWidgetOk.enabled = true;
              }
              onSelected: {
                daySelector.state = "unselected";
                yearSelector.state = "unselected";
              }
            }

            KPIM.VerticalSelector {
              id: yearSelector
              height: 100
              // high enough == 2050 :)
              model: 51
              // value - 2000 because the index starts at '0'
              beginWith: 2000

              value: myCalendar.year
              onValueChanged: {
                myCalendar.year = value;
                calendarWidgetOk.enabled = true;
              }
              onSelected: {
                daySelector.state = "unselected";
                monthSelector.state = "unselected";
              }
            }
          }
          Row {
            spacing: 5
            anchors{
              bottom: parent.bottom
              right: parent.right
            }
            KPIM.Button2 {
              id: calendarkWidgetCancel
              buttonText: KDE.i18n( "Cancel" );
              width: 100
              onClicked: {
                calendarWidget.collapse()
                //### + reset widget
              }
            }
            KPIM.Button2 {
              id: calendarWidgetOk
              buttonText: KDE.i18n( "Ok" );
              width: 100
              onClicked: {
                calendarWidget.collapse()
                _incidenceview.setNewDate(myCalendar.day, myCalendar.month, myCalendar.year);
              }
            }
          }
      }
    ]
  }

    Dialog {
      id: clockWidget
      anchors.fill: parent
      content: [
        Item {
          anchors.fill: parent

          KPIM.Clock {
            id: myClock
            anchors {
              left: parent.left
              top: parent.top
              bottom: parent.bottom

              topMargin: 25
              bottomMargin: 25
            }
          }

          Column {
            spacing: 5
            anchors {
              top: parent.top
              left: myClock.right
              right: parent.right

              topMargin: 100
              leftMargin: 60
            }

            KPIM.VerticalSelector {
              id: hourSelector
              height: 100
              model: 24

              onValueChanged: {
                myClock.hours = value;
                clockWidgetOk.enabled = true;
              }
              onSelected: {
                minuteSelector.state = "unselected";
              }
            }

            KPIM.VerticalSelector {
              id: minuteSelector
              height: 100
              model: 60

              onValueChanged: {
                myClock.minutes = value;
                clockWidgetOk.enabled = true;
              }
              onSelected: {
                hourSelector.state = "unselected";
              }
            }
          }
          Row {
            spacing: 5
            anchors{
              bottom: parent.bottom
              right: parent.right
            }
            KPIM.Button2 {
              id: clockWidgetCancel
              buttonText: KDE.i18n( "Cancel" );
              width: 100
              onClicked: {
                clockWidget.collapse()
                //### + reset widget
              }
            }
            KPIM.Button2 {
              id: clockWidgetOk
              enabled: false
              buttonText: KDE.i18n( "Ok" );
              width: 100
              onClicked: {
                clockWidget.collapse()
                _incidenceview.setNewTime(myClock.hours, myClock.minutes);
              }
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

    contentHeight: generalEditor.height;
    clip: true;
    flickableDirection: "VerticalFlick"

    IncidenceEditors.GeneralEditor {
      id: generalEditor;
      anchors.fill: parent
    }
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
