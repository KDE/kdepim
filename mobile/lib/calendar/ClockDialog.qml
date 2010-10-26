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

import Qt 4.7
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM

Dialog {
  id: clockWidget
  property alias okEnabled: clockWidgetOk.enabled

  property alias hours: myClock.hours
  property alias minutes: myClock.minutes

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
