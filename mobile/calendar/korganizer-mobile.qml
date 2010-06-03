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
      {
        itemList.nextItem();
        application.setCurrentEventItemId(eventView.itemId);
      }
    }

    onPreviousItemRequest: {
      // Only go to the previous message when currently a valid item is set.
      if ( eventView.itemId >= 0 )
      {
        itemList.previousItem();
        application.setCurrentEventItemId(eventView.itemId);
      }
    }

    KPIM.Button {
      anchors.bottom: parent.bottom
      anchors.right: backButton.left
      anchors.margins: 12
      width: parent.height / 6
      height: parent.height / 6
      icon: KDE.iconPath( "document-edit", width );
      onClicked: {
        application.editIncidence( parent.item, parent.activeDate );
        eventView.visible = false;
        agendaView.visible = true;
      }
    }
    KPIM.Button {
      id: backButton
      anchors.bottom: parent.bottom
      anchors.right: parent.right
      anchors.margins: 12
      width: parent.height / 6
      height: parent.height / 6
      icon: KDE.iconPath( "edit-undo", width );
      onClicked: {
        eventView.visible = false;
        agendaView.visible = true;
      }
    }
  }


  Rectangle {
    id: agendaView
    anchors.fill: parent
    color: "#D2D1D0" // TODO: make palette work correctly. palette.window

    CalendarViews.AgendaView {
      id: agenda
      anchors { fill: parent; topMargin: 10; leftMargin: 40 }
      calendar: calendarModel

      onItemSelected: {
        if ( selectedItemId > 0 ) {
          eventView.itemId = selectedItemId;
          eventView.activeDate = activeDate;
          application.setCurrentEventItemId(selectedItemId);
          eventView.visible = true;
          agendaView.visible = false;
          clearSelection();
        }
      }
    }
  }

  SlideoutPanelContainer {
    anchors.fill: parent

    SlideoutPanel {
      anchors.fill: parent
      id: startPanel
      titleIcon: KDE.iconPath( "office-calendar", 48 )
      handlePosition: 30
      handleHeight: 78
      content: [
        KPIM.StartCanvas {
          id : startPage
          anchors.fill : parent
          anchors.leftMargin : 50
          startText: KDE.i18n( "Calendar start page" )
          favoritesModel : favoritesList

          contextActions : [
            Column {
              anchors.fill: parent
              height: 480 / 6 * 3
              KPIM.Button {
                height : 480 / 6
                width : parent.width
                buttonText : KDE.i18n( "New Appointment" )
                onClicked : { startPanel.collapse(); application.newIncidence(); }
              }
              KPIM.Button {
                height : 480 / 6
                width : parent.width
                buttonText : KDE.i18n( "Add Calendar" )
                onClicked : { startPanel.collapse(); application.launchAccountWizard() }
              }
              KPIM.Button {
                height : 480 / 6
                width : parent.width
                buttonText : KDE.i18n( "Add Favorite" )
                onClicked : { favoriteSelector.visible = true; startPage.visible = false; }
              }
            }
          ]
        },
        FavoriteSelector {
          id : favoriteSelector
          anchors.fill : parent
          visible : false
          onFinished : {
            favoriteSelector.visible = false;
            startPage.visible = true;
            application.saveFavorite( favoriteSelector.favoriteName );
          }
        }
      ]
    }

    SlideoutPanel {
      id: folderPanel
      titleText: KDE.i18n( "Calendars" )
      handleHeight: 150
      anchors.fill : parent
      content: [
        Item {
          anchors.fill: parent //effect

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
           }

           Column {
             anchors.left: collectionView.right
             anchors.right: parent.right
             spacing: 2
             Row {
               height: 480 / 6
               width: parent.width
               QmlDateEdit {
                 id: dateEdit
                 width: parent.width
                 height: 480 / 6
                 displayFormat: "MMM d yy"
               }
             }
             Row {
               spacing: 2
               width: parent.width

               KPIM.Button {
                 id: dayButton
                 buttonText: KDE.i18n( "Day view" )
                 height: 480 / 6
                 width: parent.width / 3
                 onClicked: {
                   agenda.showRange( dateEdit.date, 0 /* "Day" */ );
                   folderPanel.collapse();
                 }
               }
               KPIM.Button {
                 id: weekButton
                 buttonText: KDE.i18n( "Week view" )
                 height: 480 / 6
                 width: parent.width / 3
                 onClicked: {
                   agenda.showRange( dateEdit.date, 1 /* "Week" */ );
                   folderPanel.collapse();
                 }
               }
               KPIM.Button {
                 id: monthButton
                 buttonText: KDE.i18n( "Month view" )
                 height: 480 / 6
                 width: parent.width / 3
                 onClicked: {
                   agenda.showRange( dateEdit.date, 2 /* "Month" */ );
                   folderPanel.collapse();
                 }
               }
            }
            Rectangle {
              width:parent.width
              height: 2
              color: "#000000"
            }
            KPIM.Button {
              id: newAppointmentButton
              height: 480 / 6
              width: parent.width
              anchors.horizontalCenter: parent.horizontalCenter
              buttonText: KDE.i18n( "New Appointment" )
              // TODO: Make sure that the correct default calender is selected in
              //       the incidence editor.
              onClicked : { application.newIncidence(); }

            }
            KPIM.Button {
              id: configureAccountButton
              anchors.horizontalCenter: parent.horizontalCenter
              height: 480 / 6
              width: parent.width
              buttonText: KDE.i18n( "Configure Account" )
            }
          }
        }
      ]
    }

    SlideoutPanel {
      id: actionPanel
      titleText: KDE.i18n( "Actions" )
      handleHeight: 150
      anchors.fill : parent
      contentWidth: 240
      content: [
          KPIM.Button {
            id: newButton
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            height: parent.height / 6
            buttonText: KDE.i18n( "New Appointment" )
            onClicked: { application.newIncidence(); actionPanel.collapse() }
          },
//           KPIM.Button {
//             id: moveButton
//             anchors.top: newButton.bottom
//             anchors.horizontalCenter: parent.horizontalCenter;
//             width: parent.width - 10
//             height: parent.height / 6
//             buttonText: KDE.i18n( "Move" )
//             onClicked: actionPanel.collapse();
//           },
          KPIM.Action {
             id: deleteButton
             anchors.top: newButton.bottom;
             anchors.horizontalCenter: parent.horizontalCenter;
             width: parent.width - 10
             height: parent.height / 6
             action : application.getAction("akonadi_item_delete")
             onTriggered : actionPanel.collapse();
           },
           KPIM.Button {
             id: previousButton
             anchors.top: deleteButton.bottom;
             anchors.horizontalCenter: parent.horizontalCenter;
             width: parent.width - 10
             height: parent.height / 6
             buttonText: KDE.i18n( "Previous" )
             onClicked: {
               agenda.gotoNext();
               actionPanel.collapse()
             }
           },
           KPIM.Button {
             anchors.top: previousButton.bottom;
             anchors.horizontalCenter: parent.horizontalCenter;
             width: parent.width - 10
             height: parent.height / 6
             buttonText: KDE.i18n( "Next" )
             onClicked: {
               agenda.gotoPrevious();
               actionPanel.collapse();
             }
           }
      ]
    }
    SlideoutPanel {
      anchors.fill: parent
      id: attachmentPanel
      visible: eventView.attachmentModel.attachmentCount >= 1
      titleIcon: KDE.iconPath( "mail-attachment", 48 );
      handleHeight: parent.height - startPanel.handlePosition - startPanel.handleHeight - actionPanel.handleHeight - folderPanel.handleHeight - anchors.topMargin - anchors.bottomMargin
      content: [
        KPIM.AttachmentList {
          id: attachmentView
          model: eventView.attachmentModel
          anchors.fill: parent
        }
      ]
    }

  }

  Connections {
    target: startPage
    onAccountSelected : {
      application.setSelectedAccount(row);
      application.showRegularCalendar();
      startPanel.collapse();
    }
  }
  Connections {
    target: startPage
    onFavoriteSelected : {
      application.loadFavorite(favName);
      application.showFavoriteCalendar();
      startPanel.collapse();
    }
  }
  Connections {
    target: collectionView
    onChildCollectionSelected : {
      application.setSelectedChildCollectionRow( row );
      application.showRegularCalendar();
    }
  }

  Connections {
    target: collectionView
    onBreadcrumbCollectionSelected : {
      application.setSelectedBreadcrumbCollectionRow( row );
      application.showRegularCalendar();
    }
  }
}
