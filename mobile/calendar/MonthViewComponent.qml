/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
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

import QtQuick 1.1 as QML
import org.kde 4.5
import org.kde.akonadi 4.5
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.calendarviews 4.5 as CalendarViews
import org.kde.akonadi.events 4.5 as Events

QML.Rectangle {
  anchors.fill: parent

  function showMonth( date )
  {
    month.showMonth( date );
  }

  QML.Rectangle {
    height: 48
    width: 48
    z: 5
    color: "#00000000"
    anchors.right : parent.right
    anchors.rightMargin : 70
    anchors.bottom : parent.bottom
    anchors.bottomMargin : 70
    QML.Image {
      source : KDE.locate( "data", "mobileui/back-to-list-button.png" );
      QML.MouseArea {
        anchors.fill : parent;
        onClicked : {
          _itemActionModel.select(-1, 1)
          _itemNavigationModel.select(-1, 1)
          guiStateManager.popState();
        }
      }
    }
  }

  CalendarViews.MonthView {
    id: month
    anchors { fill: parent; topMargin: 10; leftMargin: 40 }
    calendar: calendarModel
    swipeLength: 0.2 // Require at least 20% of screenwidth to trigger next or prev

    onDateClicked: {
      agenda.showRange( date, 0 );
      guiStateManager.switchState( Events.EventsGuiStateManager.ViewDayState );
    }

    onItemSelected: {
      if ( selectedItemId > 0 ) {
        eventView.itemId = selectedItemId;
        eventView.activeDate = activeDate;
        application.setCurrentEventItemId(selectedItemId);
        guiStateManager.pushUniqueState( KPIM.GuiStateManager.ViewSingleItemState );
      }
    }
  }
}

