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
    visible: false
    anchors.fill: parent
    color: "#D2D1D0" // TODO: make palette work correctly. palette.window

    KPIM.Button {
      id : backToMessageListButton
      y : 250
      width : 48
      height : 48
      icon: KDE.iconPath("korganizer", 48)
      onClicked: {
        agendaView.visible = false;
        mainWorkView.visible = true;
      }
    }

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

  FavoriteSelector {
    id : favoriteSelector
    anchors.fill : parent
    anchors.topMargin : 12
    anchors.leftMargin: 40
    visible : false
    onFinished : {
      favoriteSelector.visible = false;
      mainWorkView.visible = true;
    }
    onCanceled : {
      favoriteSelector.visible = false;
      mainWorkView.visible = true;
    }
    styleSheet: window.styleSheet
  }

  Item {
    id : mainWorkView
    anchors.top: parent.top
    anchors.topMargin : 12
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    width: 1/3 * parent.width

    AkonadiBreadcrumbNavigationView {
      id : collectionView
      anchors.top: parent.top
      anchors.bottom : selectButton.top
      anchors.left: parent.left
      anchors.right: parent.right
      breadcrumbItemsModel : breadcrumbCollectionsModel
      selectedItemModel : selectedCollectionModel
      childItemsModel : childCollectionsModel

      // It's not possible to get the number of items in a model. We have to
      // put the model in a view and count the items in the view.
      ListView { id : dummyItemView; model : calendarModel }
      multipleSelectionText : KDE.i18na("You have selected \n%1 folders\nfrom %2 accounts\n%3 events", [collectionView.numSelected,
                                                                                                        application.numSelectedAccounts,
                                                                                                        dummyItemView.count])
    }

    KPIM.Button2 {
      id : selectButton
      anchors.left: collectionView.left
      anchors.right: collectionView.right
      anchors.bottom : parent.bottom
      anchors.bottomMargin : collectionView.hasSelection ? -selectButton.height : 0
      buttonText : KDE.i18n("Select")
      opacity : collectionView.hasSelection ? 0 : 1
      onClicked : {
        favoriteSelector.visible = true;
        mainWorkView.visible = false;
      }
    }
  }

  KPIM.StartCanvas {
    id: homePage
    anchors.left: mainWorkView.right
    anchors.right: parent.right
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    anchors.topMargin: 12
    visible: mainWorkView.visible && !collectionView.hasSelection

    showAccountsList : false
    favoritesModel : favoritesList

    contextActions : [
      Column {
        anchors.fill: parent
        height : 70
        KPIM.Button2 {
          id: newAppointmentButton2
          width: 2/3 * parent.width
          anchors.horizontalCenter: parent.horizontalCenter
          buttonText: KDE.i18n( "New Appointment" )
          // TODO: Make sure that the correct default calender is selected in
          //       the incidence editor.
          onClicked : { application.newIncidence(); }

        }
      }
    ]
  }

  Item {
    id : calendarPage
    anchors.left: mainWorkView.right
    anchors.right: parent.right
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    visible: mainWorkView.visible && collectionView.hasSelection

    Column {
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.right: parent.right
      height: parent.height
      spacing: 10
      Row {
        height: 480 / 6
        width: parent.width
        QmlDateEdit {
          id: dateEdit
          width: parent.width
          height: 480 / 6
          displayFormat: "MM dd yyyy"
          styleSheet: window.styleSheet
        }
      }
      Row {
        spacing: 2
        width: parent.width - 5

        KPIM.Button2 {
          id: dayButton
          buttonText: KDE.i18n( "Day view" )
          width: parent.width / 3
          onClicked: {
            agenda.showRange( dateEdit.date, 0 /* "Day" */ );
            mainWorkView.visible = false
            agendaView.visible = true
          }
        }
        KPIM.Button2 {
          id: weekButton
          buttonText: KDE.i18n( "Week view" )
          width: parent.width / 3
          onClicked: {
            agenda.showRange( dateEdit.date, 1 /* "Week" */ );
            mainWorkView.visible = false
            agendaView.visible = true
          }
        }
        KPIM.Button2 {
          id: monthButton
          buttonText: KDE.i18n( "Month view" )
          width: parent.width / 3
          onClicked: {
            agenda.showRange( dateEdit.date, 2 /* "Month" */ );
            mainWorkView.visible = false
            agendaView.visible = true
          }
        }
      }

      KPIM.Button2 {
        id: newAppointmentButton
        width: 2/3 * parent.width
        anchors.horizontalCenter: parent.horizontalCenter
        buttonText: KDE.i18n( "New Appointment" )
        // TODO: Make sure that the correct default calender is selected in
        //       the incidence editor.
        onClicked : { application.newIncidence(); }

      }
      KPIM.Button2 {
        id: configureAccountButton
        anchors.horizontalCenter: parent.horizontalCenter
        width: 2/3 * parent.width
        visible: (collectionView.hasSelection && !collectionView.hasBreadcrumbs) ? true : false
        buttonText: KDE.i18n( "Configure Account" )
        onClicked : { application.configureCurrentAccount(); }
      }

      KPIM.Button2 {
        id: addCalendarAccountButton
        anchors.horizontalCenter: parent.horizontalCenter
        width: 2/3 * parent.width
        buttonText: KDE.i18n( "Add calendar" )
        onClicked : { application.launchAccountWizard() }
      }
      KPIM.Button2 {
        id : saveFavoriteButton
        anchors.horizontalCenter: parent.horizontalCenter
        buttonText: KDE.i18n( "Save calendar selection" )
        width: 2/3 * parent.width
        visible : collectionView.hasSelection
        onClicked : {
          application.saveFavorite();
        }
      }
    }
  }

  SlideoutPanelContainer {
    anchors.fill: parent
    visible: agendaView.visible || eventView.visible

    SlideoutPanel {
      id: actionPanel
      titleText: KDE.i18n( "Actions" )
      handleHeight: 150
      handlePosition : 40
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
               agenda.gotoPrevious();
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
               agenda.gotoNext();
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
    target: homePage
    onAccountSelected : {
      application.setSelectedAccount(row);
    }
  }

  Connections {
    target: homePage
    onFavoriteSelected : {
      application.loadFavorite(favName);
    }
  }

  Connections {
    target: collectionView
    onChildCollectionSelected : {
      application.setSelectedChildCollectionRow( row );
    }
  }

  Connections {
    target: collectionView
    onBreadcrumbCollectionSelected : {
      application.setSelectedBreadcrumbCollectionRow( row );
    }
  }
}
