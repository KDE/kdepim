/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Artur Duque de Souza <asouza@kde.org>
    Copyright (C) 2010 Anselmo Lacerda Silveira de Melo <anselmolsm@gmail.com>
    Copyright (C) 2010 Eduardo Madeira Fleury <efleury@gmail.com>

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

Dialog {
  id: calendarWidget
  property alias okEnabled: calendarWidgetOk.enabled
  property alias day: myCalendar.day
  property alias month: myCalendar.month
  property alias year: myCalendar.year

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
