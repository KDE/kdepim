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

  function showDate(date)
  {
    console.log("QML showDate called");
    korganizerActions.showOnlyCategory("single_calendar")
    agenda.showRange( date, 0 /* "Day" */ );
  }

  function showEventView()
  {
    console.log("QML showEventView called");
    korganizerActions.showOnlyCategory("event_viewer")
    mainWorkView.visible = false
    agendaView.visible = true
  }

  function backToAgendaView()
  {
    eventView.visible = false;
    agendaView.visible = true;
  }

  function updateContextActionsStates()
  {
    if (collectionView.numBreadcrumbs == 0 && collectionView.numSelected == 0) { // root is selected
      korganizerActions.showOnlyCategory("home")
    } else if (collectionView.numBreadcrumbs == 0 && collectionView.numSelected != 0) { // top-level is selected
      korganizerActions.showOnlyCategory("account")
    } else if ( collectionView.numSelected > 1 ) { // something else is selected
      korganizerActions.showOnlyCategory( "multiple_folder" );
    } else {
      korganizerActions.showOnlyCategory("single_folder")
    }
  }

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
      anchors.bottom: backButton.top
      anchors.right: parent.right
      anchors.margins: 12
      width: 70
      height: 70
      icon: KDE.locate( "data", "mobileui/edit-button.png" );
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
      width: 70
      height: 70
      icon: KDE.locate( "data", "mobileui/back-to-list-button.png" );
      onClicked: {
        backToAgendaView();
      }
    }
  }

  Rectangle {
    id: agendaView
    visible: false
    anchors.fill: parent
    color: "#D2D1D0" // TODO: make palette work correctly. palette.window

    Rectangle {
      id : backToMessageListButton
      height: 48
      width: 48
      z: 5
      color: "#00000000"
      anchors.right : parent.right
      anchors.rightMargin : 70
      anchors.bottom : parent.bottom
      anchors.bottomMargin : 70
      Image {
        source : KDE.locate( "data", "mobileui/back-to-list-button.png" );
        MouseArea {
          anchors.fill : parent;
          onClicked : {
            agendaView.visible = false;
            mainWorkView.visible = true;
            selectButton.visible = true
            korganizerActions.showOnlyCategory("home")
          }
        }
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
          korganizerActions.showOnlyCategory("event_viewer")
          eventView.visible = true;
          agendaView.visible = false;
          clearSelection();
        }
      }
    }

    onVisibleChanged : {
      if ( visible ) {
        if ( collectionView.numSelected > 1 )
          korganizerActions.showOnlyCategory("multiple_calendar")
        else
          korganizerActions.showOnlyCategory("single_calendar")
      }
    }
  }

  Item {
    id : mainWorkView
    anchors.top: parent.top
    anchors.topMargin : 12
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    width: 1/3 * parent.width

    Image {
      id: backgroundImage
      x: 0
      y: 0
      source: "korganizer-mobile-background.png"
    }

    AkonadiBreadcrumbNavigationView {
      id : collectionView
      anchors.top: parent.top
      anchors.bottom : selectButton.top
      anchors.left: parent.left
      anchors.right: parent.right

      breadcrumbComponentFactory : _breadcrumbNavigationFactory

      // It's not possible to get the number of items in a model. We have to
      // put the model in a view and count the items in the view.
      ListView { id : dummyItemView; model : calendarModel }
      multipleSelectionText : KDE.i18n("You have selected \n%1 folders\nfrom %2 accounts\n%3 events",
                                       collectionView.numSelected,
                                       application.numSelectedAccounts,
                                       dummyItemView.count)

      Component.onCompleted : updateContextActionsStates();
      onNumBreadcrumbsChanged : updateContextActionsStates();
      onNumSelectedChanged : updateContextActionsStates();
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
          onClicked : { application.newEvent(); }

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
          displayFormat: "MM.dd.yyyy"
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
            selectButton.visible = false
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
            selectButton.visible = false
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
            selectButton.visible = false
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
        onClicked : { application.newEvent(); }

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

    SlideoutPanel {
      id: actionPanelNew
      titleText: KDE.i18n( "Actions" )
      handlePosition : 125
      handleHeight: 150
      anchors.fill : parent

      content : [
        KorganizerActions {
          id : korganizerActions
          anchors.fill : parent

          scriptActions : [
            KPIM.ScriptAction {
              name : "show_about_dialog"
              script : {
                actionPanelNew.collapse();
                aboutDialog.visible = true
              }
            },
            KPIM.ScriptAction {
              name : "to_selection_screen"
              script : {
                actionPanelNew.collapse();
                favoriteSelector.visible = true;
                mainWorkView.visible = false;
              }
            },
            KPIM.ScriptAction {
              name : "add_as_favorite"
              script : {
                actionPanelNew.collapse();
                application.saveFavorite();
              }
            },
            KPIM.ScriptAction {
              name : "day_layout"
              script: {
                agenda.showRange( dateEdit.date, 0 /* "Day" */ );
                actionPanelNew.collapse();
              }
            },
            KPIM.ScriptAction {
              name : "week_layout"
              script: {
                agenda.showRange( dateEdit.date, 1 /* "Week" */ );
                actionPanelNew.collapse();
              }
            },
            KPIM.ScriptAction {
              name : "month_layout"
              script: {
                agenda.showRange( dateEdit.date, 2 /* "Month" */ );
                actionPanelNew.collapse();
              }
            }
          ]

          onTriggered : {
            console.log("Triggered was: " + triggeredName)
          }

        }
      ]
    }
    SlideoutPanel {
      anchors.fill: parent
      handlePosition : actionPanelNew.handlePosition + actionPanel.handleHeight
      id: attachmentPanel
      visible: eventView.attachmentModel.attachmentCount >= 1
      titleIcon: KDE.iconPath( "mail-attachment", 48 );
      handleHeight: parent.height - actionPanelNew.handlePosition - actionPanelNew.handleHeight - anchors.topMargin - anchors.bottomMargin
      content: [
        KPIM.AttachmentList {
          id: attachmentView
          model: eventView.attachmentModel
          anchors.fill: parent
        }
      ]
    }
  }
  KPIM.MultipleSelectionScreen {
    id : favoriteSelector
    anchors.fill : parent
    anchors.topMargin : 12
    visible : false
    backgroundImage : backgroundImage.source
    onFinished : {
      favoriteSelector.visible = false;
      mainWorkView.visible = true;
      application.multipleSelectionFinished();
    }
    onCanceled : {
      favoriteSelector.visible = false;
      mainWorkView.visible = true;
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
    target: eventView
    onIncidenceRemoved : {
      backToAgendaView();
    }
  }

  KPIM.AboutDialog {
    id : aboutDialog
    anchors.fill : parent
    visible : false
  }
}
