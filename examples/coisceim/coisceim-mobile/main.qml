/*
    This file is part of Akonadi.

    Copyright (c) 2011 Stephen Kelly <steveire@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

import Qt 4.7
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.kcal 4.5 as KCal

Item {
  Image {
    id: backgroundImage
    x: 0
    y: 0
    source: "notes-mobile-background.png"
    visible: collectionView.visible
  }

  Item {
    anchors.fill : parent
    anchors.leftMargin : 30
    CreateTrip {
      anchors.fill : parent
      visible : triplist.currentIndex == triplist.count - 1
    }

    Trip {
      anchors.fill : parent

      trip : triplist.currentItem.trip
      visible : triplist.currentIndex != triplist.count - 1
    }
  }

  SlideoutPanelContainer {
    id: panelContainer
    anchors.fill: parent

    SlideoutPanel {
      id: folderPanel
      titleText: KDE.i18n( "Trips" )
      handleHeight: 150
      contentWidth : 200
      content: [
        ListView {
          id : triplist
          anchors.top : parent.top
          anchors.bottom : parent.bottom
          width : 100

          currentIndex : count - 1

          model : _tripModel

          delegate : Text {
            property variant trip : model.trip
            height : 30
            text : model.display

            MouseArea {
              anchors.fill : parent
              onClicked : {
                triplist.currentIndex = model.index
              }
            }
          }
        }
      ]
    }
    SlideoutPanel {
      id: folderPanel2
      titleText: KDE.i18n( "Current trip" )
      handleHeight: 150
      handlePosition : 200
      content: [
        KCal.IncidenceView {
          id: eventView
          width: parent.width
          height: parent.height - deleteButton.height

          itemId: triplist.currentItem.trip ? triplist.currentItem.trip.id : -1
        },
        KPIM.Button2 {
          id : deleteButton
          height : 70
          width : parent.width
          anchors.bottom : parent.bottom
          buttonText : "Delete"
          onClicked : {

          }
        }
      ]
    }
  }

}
