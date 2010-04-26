/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
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
import org.qt 4.7 // Qt widget wrappers
import org.kde 4.5
import org.kde.akonadi 4.5
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.kcal 4.5 as KCal
import org.kde.calendarviews 4.5 as CalendarViews

KPIM.MainView {
  id: korganizerMobile

  SystemPalette { id: palette; colorGroup: "Active" }

  KCal.IncidenceView {
    id: eventView
    anchors { fill: parent; topMargin: 48; leftMargin: 48 }
    visible: false

    z: 0

    itemId: -1
    swipeLength: 0.2 // Require at least 20% of screenwidth to trigger next or prev

    onNextItemRequest: {
      // Only go to the next message when currently a valid item is set.
      if ( eventView.itemId >= 0 )
        itemList.nextItem();
    }

    onPreviousItemRequest: {
      // Only go to the previous message when currently a valid item is set.
      if ( eventView.itemId >= 0 )
        itemList.previousItem();
    }
  }

  Rectangle {
    id: agendaView
    anchors.fill: parent
    color: palette.window

    CalendarViews.AgendaView {
      anchors { fill: parent; topMargin: 10; leftMargin: 40 }
      calendar: calendarModel
      startDate: "2010-04-26"
      endDate: "2010-05-02"
    }
  }

  SlideoutPanelContainer {
    anchors.fill: parent

    SlideoutPanel {
      id: folderPanel
      titleText: "Folders"
      handleHeight: 150
      anchors.fill : parent
      content: [
        Item {
          anchors.fill: parent

           AkonadiBreadcrumbNavigationView {
             id : collectionView
             width: 1/3 * folderPanel.contentWidth
             anchors.top: parent.top
             anchors.bottom: parent.bottom
             anchors.left: parent.left
             anchors.rightMargin: 4
             breadcrumbItemsModel : breadcrumbCollectionsModel
             selectedItemModel : selectedCollectionModel
             childItemsModel : childCollectionsModel
             onCollectionSelected: {
               //folderPanel.collapse()
             }
           }

           Column {
             anchors.left: collectionView.right
             spacing: 2
             Row {
               Text {
                 id: dateText
                 height: 30
                 verticalAlignment: "AlignVCenter"
                 text: "Show the date:"
               }
               QmlDateEdit {
                 anchors.left: dateText.right // WTF is the QmlDateEdit overlapping the dateText when this is not set?
                 id: dateEdit
                 height: 30
               }
             }
             Text {
               height: 30
               verticalAlignment: "AlignVCenter"
               text: "Using the following calendar view:"
             }
             Row {
               spacing: 2
               KPIM.Button {
                 id: dayButton
                 buttonText: "Day"
                 height: 40
                 width: 100
               }
               KPIM.Button {
                 id: weekButton
                 buttonText: "Week"
                 height: 40
                 width: 100
               }
               KPIM.Button {
                 id: monthButton
                 buttonText: "Month"
                 height: 40
                 width: 100
               }
               KPIM.Button {
                 id: yearButton
                 buttonText: "Year"
                 height: 40
                 width: 100
               }
            }
            Rectangle {
              width:parent.width
              height: 2
              color: "#000000"
            }
            KPIM.Button {
              id: newAppointmentButton
              buttonText: "New appointment"
              height: 40
              width: 150
            }
            KPIM.Button {
              id: searchAppointmentButton
              buttonText: "Search appointment"
              height: 40
              width: 150
            }
            KPIM.Button {
              id: configureAccountButton
              buttonText: "Configure account"
              height: 40
              width: 150
            }
          }

        }
      ]
    }

    SlideoutPanel {
      id: actionPanel
      titleText: "Actions"
      handleHeight: 150
      handlePosition : 150
      anchors.fill : parent
      contentWidth: 240
      content: [
          KPIM.Button {
            id: moveButton
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            height: parent.height / 6
            buttonText: "Move"
            onClicked: actionPanel.collapse();
          },
          KPIM.Button {
             id: deleteButton
             anchors.top: moveButton.bottom;
             anchors.horizontalCenter: parent.horizontalCenter;
             width: parent.width - 10
             height: parent.height / 6
             buttonText: "Delete"
             onClicked: actionPanel.collapse();
           },
           KPIM.Button {
             id: previousButton
             anchors.top: deleteButton.bottom;
             anchors.horizontalCenter: parent.horizontalCenter;
             width: parent.width - 10
             height: parent.height / 6
             buttonText: "Previous"
             onClicked: {
               itemList.previousItem()
               actionPanel.collapse()
             }
           },
           KPIM.Button {
             anchors.top: previousButton.bottom;
             anchors.horizontalCenter: parent.horizontalCenter;
             width: parent.width - 10
             height: parent.height / 6
             buttonText: "Next"
             onClicked: {
               itemList.nextItem();
               actionPanel.collapse();
             }
           }
      ]
    }

  }

   Connections {
     target: collectionView
     onChildCollectionSelected : { application.setSelectedChildCollectionRow( row ); }
   }

   Connections {
     target: collectionView
     onBreadcrumbCollectionSelected : { application.setSelectedBreadcrumbCollectionRow( row ); }
   }
}
