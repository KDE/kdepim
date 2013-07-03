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

import QtQuick 1.1
import org.qt 4.7 // Qt widget wrappers
import org.kde 4.5
import org.kde.akonadi 4.5
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.kcal 4.5 as KCal
import org.kde.calendarviews 4.5 as CalendarViews
import org.kde.akonadi.events 4.5 as Events
import org.kde.akonadi.calendar 4.5 as Calendar
import "../mobileui/ScreenFunctions.js" as Screen

KPIM.MainView {
  id: korganizerMobile

  Connections {
    target: guiStateManager
    onGuiStateChanged: { updateContextActionStates() }
  }

  Component.onCompleted : updateContextActionStates();

  function updateContextActionStates()
  {
    if ( guiStateManager.inHomeScreenState ) {
      korganizerActions.showOnlyCategory( "home" )
    } else if ( guiStateManager.inAccountScreenState ) {
      korganizerActions.showOnlyCategory( "account" )
    } else if ( guiStateManager.inSingleFolderScreenState ) {
      korganizerActions.showOnlyCategory( "single_folder" )
    } else if ( guiStateManager.inMultipleFolderScreenState || guiStateManager.inSearchResultScreenState ) {
      korganizerActions.showOnlyCategory( "multiple_folder" )
    } else if ( guiStateManager.inViewSingleItemState ) {
      korganizerActions.showOnlyCategory( "event_viewer" )
    } else if ( guiStateManager.inViewDayState || guiStateManager.inViewWeekState || guiStateManager.inViewMonthState|| guiStateManager.inViewTimelineState || guiStateManager.inViewEventListState ) {
      if ( collectionView.numSelected > 1 )
        korganizerActions.showOnlyCategory( "multiple_calendar" )
      else
        korganizerActions.showOnlyCategory( "single_calendar" )
    }
  }

  function showDate(date)
  {
    agenda.showRange( date, 0 /* "Day" */ );
    guiStateManager.pushState( Events.EventsGuiStateManager.ViewDayState );
    application.bringToFront();
  }

  KCal.IncidenceView {
    id: eventView
    anchors { fill: parent; topMargin: 40; leftMargin: 40 }
    visible: guiStateManager.inViewSingleItemState
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

    KPIM.ItemEditButton {
      actionName: "akonadi_incidence_edit"
      anchors.bottom: backButton.top
      anchors.right: parent.right
      anchors.margins: 12
      onClicked: {
        application.editIncidence( parent.item, parent.activeDate );
        guiStateManager.popState();
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
        _itemActionModel.select(-1, 1)
        _itemNavigationModel.select(-1, 1)
        guiStateManager.popState();
      }
    }
  }

  Loader {
    anchors.fill: parent
    source: guiStateManager.inViewMonthState ? "MonthViewComponent.qml" : ""

    onLoaded: {
     item.showMonth( dateEdit.date );
    }
  }

  Rectangle {
    id: agendaView
    visible: guiStateManager.inViewDayState || guiStateManager.inViewWeekState
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
            _itemActionModel.select(-1, 1)
            _itemNavigationModel.select(-1, 1)
            guiStateManager.popState();
          }
        }
      }
    }

    CalendarViews.AgendaView {
      id: agenda
      anchors { fill: parent; topMargin: 10; leftMargin: 40 }
      calendar: calendarModel
      swipeLength: 0.2 // Require at least 20% of screenwidth to trigger next or prev

      onItemSelected: {
        if ( selectedItemId > 0 ) {
          eventView.itemId = selectedItemId;
          eventView.activeDate = activeDate;
          application.setCurrentEventItemId(selectedItemId);
          guiStateManager.pushUniqueState( KPIM.GuiStateManager.ViewSingleItemState );
          clearSelection();
        }
      }
    }
  }

  Loader {
    anchors.fill: parent
    source: guiStateManager.inViewTimelineState ? "TimelineView.qml" : ""
    onLoaded: { item.showRange( dateEdit.date, 4 /* Next 7 days */ ); }
  }

  Loader {
    anchors.fill: parent
    source: guiStateManager.inViewEventListState ? "EventListComponent.qml" : ""
  }

  Item {
    id : mainWorkView
    visible: { guiStateManager.inHomeScreenState ||
               guiStateManager.inAccountScreenState ||
               guiStateManager.inSingleFolderScreenState ||
               guiStateManager.inMultipleFolderScreenState
             }
    anchors.top: parent.top
    anchors.topMargin : 12
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    width: 1/3 * parent.width

    Image {
      id: backgroundImage
      x: 0
      y: 0
// FIXME: too big, costs about 1.5Mb RAM
//      source: "korganizer-mobile-background.png"
    }

    AkonadiBreadcrumbNavigationView {
      id : collectionView
      anchors.top: parent.top
      anchors.bottom : selectButton.top
      anchors.left: parent.left
      anchors.right: parent.right
      itemHeight: Screen.partition( height, 7 )

      breadcrumbComponentFactory : _breadcrumbNavigationFactory

      multipleSelectionText : KDE.i18nc("%1 is e.g. 3 folders, %2 is e.g. from 2 accounts, %3 is e.g. 9 events",
                                        "You have selected \n%1\n%2\n%3",
                                        KDE.i18np("1 folder","%1 folders",collectionView.numSelected),
                                        KDE.i18np("from 1 account","from %1 accounts",application.numSelectedAccounts),
                                        KDE.i18np("1 event","%1 events", calendarModel.incidencesCount))

      onSelectedClicked : {
        guiStateManager.pushState( KPIM.GuiStateManager.BulkActionScreenState );
      }

      KPIM.AgentStatusIndicator {
        anchors { top: parent.top; right: parent.right; rightMargin: 10; topMargin: 10 }
      }
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
        application.persistCurrentSelection("preFavSelection");
        guiStateManager.pushState( KPIM.GuiStateManager.MultipleFolderSelectionScreenState );
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
          // TODO: Make sure that the correct default calendar is selected in
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
          // MM.dd.yyyy
          displayFormat: KDE.i18n( "yyyy-MM-dd" )
        }
      }
      Row {
        spacing: 2
        width: parent.width - 5

        KPIM.Button2 {
          id: dayButton
          buttonText: KDE.i18n( "Day view" )
          width: parent.width / 4
          onClicked: {
            agenda.showRange( dateEdit.date, 0 /* "Day" */ );
            guiStateManager.pushState( Events.EventsGuiStateManager.ViewDayState );
          }
        }
        KPIM.Button2 {
          id: weekButton
          buttonText: KDE.i18n( "Week view" )
          width: parent.width / 4
          onClicked: {
            agenda.showRange( dateEdit.date, 1 /* "Week" */ );
            guiStateManager.pushState( Events.EventsGuiStateManager.ViewWeekState );
          }
        }
        KPIM.Button2 {
          id: monthButton
          buttonText: KDE.i18n( "Month view" )
          width: parent.width / 4
          onClicked: {
            guiStateManager.pushState( Events.EventsGuiStateManager.ViewMonthState );
          }
        }

        KPIM.Button2 {
          id: timelineButton
          buttonText: KDE.i18n( "Timeline" )
          width: parent.width / 4
          onClicked: {
            guiStateManager.pushState( Events.EventsGuiStateManager.ViewTimelineState );
          }
        }
      }

      KPIM.Button2 {
        id: newAppointmentButton
        width: 2/3 * parent.width
        anchors.horizontalCenter: parent.horizontalCenter
        buttonText: KDE.i18n( "New Appointment" )
        // TODO: Make sure that the correct default calendar is selected in
        //       the incidence editor.
        onClicked : { application.newEventWithDate( dateEdit.date ); }

      }
    }
  }

  SlideoutPanelContainer {
    anchors.fill: parent
    z: 100

    visible: !guiStateManager.inBulkActionScreenState &&
             !guiStateManager.inMultipleFolderSelectionScreenState &&
             !guiStateManager.inConfigScreenState &&
             !guiStateManager.inSearchScreenState

    SlideoutPanel {
      id: actionPanel
      titleText: KDE.i18n( "Actions" )
      handlePosition : 125
      handleHeight: 150
      anchors.fill : parent

      Component.onCompleted: {
        actionPanel.expanded.connect( korganizerActions, korganizerActions.refresh );
      }

      content : [
        KorganizerActions {
          id : korganizerActions
          anchors.fill : parent

          scriptActions : [
            KPIM.ScriptAction {
              name : "show_about_dialog"
              script : {
                actionPanel.collapse();
                aboutDialog.visible = true
              }
            },
            KPIM.ScriptAction {
              name : "configure"
              script : {
                actionPanel.collapse();
                guiStateManager.pushState( KPIM.GuiStateManager.ConfigScreenState );
              }
            },
            KPIM.ScriptAction {
              name : "search_event"
              script : {
                actionPanel.collapse();
                guiStateManager.pushState( KPIM.GuiStateManager.SearchScreenState );
              }
            },
            KPIM.ScriptAction {
              name : "to_selection_screen"
              script : {
                actionPanel.collapse();
                guiStateManager.pushState( KPIM.GuiStateManager.MultipleFolderSelectionScreenState );
              }
            },
            KPIM.ScriptAction {
              name : "add_as_favorite"
              script : {
                actionPanel.collapse();
                application.saveFavorite();
              }
            },
            KPIM.ScriptAction {
              name : "day_layout"
              script: {
                guiStateManager.switchState( Events.EventsGuiStateManager.ViewWeekState );
                agenda.showRange( dateEdit.date, 0 /* "Day" */ );
                actionPanel.collapse();
              }
            },
            KPIM.ScriptAction {
              name : "three_day_layout"
              script : {
                guiStateManager.switchState( Events.EventsGuiStateManager.ViewWeekState );
                agenda.showRange( dateEdit.date, 3 /** 3 days*/ );
                actionPanel.collapse();
              }
            },
            KPIM.ScriptAction {
              name : "week_layout"
              script: {
                guiStateManager.switchState( Events.EventsGuiStateManager.ViewWeekState );
                agenda.showRange( dateEdit.date, 1 /* "Week" */ );
                actionPanel.collapse();
              }
            },
            KPIM.ScriptAction {
              name : "work_week_layout"
              script: {
                guiStateManager.switchState( Events.EventsGuiStateManager.ViewWeekState );
                agenda.showRange( dateEdit.date, 2 /* "WorkWeek" */ );
                actionPanel.collapse();
              }
            },
            KPIM.ScriptAction {
              name : "month_layout"
              script: {
                guiStateManager.switchState( Events.EventsGuiStateManager.ViewMonthState );
                actionPanel.collapse();
              }
            },
            KPIM.ScriptAction {
              name : "eventlist_layout"
              script: {
                guiStateManager.switchState( Events.EventsGuiStateManager.ViewEventListState );
                actionPanel.collapse();
              }
            },
            KPIM.ScriptAction {
              name : "timeline_layout"
              script: {
                guiStateManager.switchState( Events.EventsGuiStateManager.ViewTimelineState );
                actionPanel.collapse();
              }
            },
            KPIM.ScriptAction {
              name : "show_today"
              script : {
                agenda.showToday();
                guiStateManager.switchState( Events.EventsGuiStateManager.ViewDayState );
                actionPanel.collapse();
              }
            },
            KPIM.ScriptAction {
              name : "start_maintenance"
              script : {
                actionPanel.collapse();
                guiStateManager.pushState( KPIM.GuiStateManager.BulkActionScreenState );
              }
            }
          ]

          onDoCollapse : actionPanel.collapse();
        }
      ]
    }
    SlideoutPanel {
      anchors.fill: parent
      handlePosition : actionPanel.handlePosition + actionPanel.handleHeight
      id: attachmentPanel
      visible: (eventView.attachmentModel.attachmentCount >= 1) && guiStateManager.inViewSingleItemState
      titleIcon: KDE.iconPath( "mail-attachment", 48 );
      handleHeight: parent.height - actionPanel.handlePosition - actionPanel.handleHeight - anchors.topMargin - anchors.bottomMargin
      content: [
        KPIM.AttachmentList {
          id: attachmentView
          model: eventView.attachmentModel
          anchors.fill: parent

          onOpenAttachment: {
            application.openAttachment(url, mimeType);
          }

          onSaveAttachment: {
            application.saveAttachment(url, fileName);
          }
        }
      ]
    }
  }

  Loader {
    anchors.fill: parent
    source: guiStateManager.inMultipleFolderSelectionScreenState ? KDE.locate( "module", "imports/org/kde/pim/mobileui/MultipleSelectionScreen.qml" ) : ""
    onLoaded: { item.backgroundImage = backgroundImage.source; }
  }

  Loader {
    anchors.fill: parent
    source: guiStateManager.inBulkActionScreenState ? "BulkActionComponent.qml" : ""
    onLoaded: {
      item.backgroundImage = backgroundImage.source
      item.itemHeight = Screen.partition( item.height, 7 )
    }
  }

  KPIM.SearchResultScreen {
    id : searchResultScreen
    anchors.top: parent.top
    anchors.topMargin : 12
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right : parent.right

    itemView: EventListView {
      id: searchEventListView
      model: itemModel
      checkModel : _itemActionModel
      navigationModel : _itemNavigationModel
      anchors.fill : parent
      itemHeight: Screen.partition( height, 7 )
    }

    resultText: KDE.i18np( "One event found", "%1 events found", searchEventListView.count )
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
      if ( guiStateManager.inViewSingleItemState ) {
        guiStateManager.popState();
      }
    }
  }

  Loader {
    id : aboutDialog
    anchors.fill: parent
    visible: false
    source: visible ? KDE.locate( "module", "imports/org/kde/pim/mobileui/AboutDialog.qml" ) : ""
    onLoaded: { item.backgroundSource = backgroundImage.source; }
  }

  Loader {
    anchors.fill: parent
    source: guiStateManager.inConfigScreenState ? "ConfigDialog.qml" : ""
    onLoaded: item.load();
  }

  KPIM.SearchDialog {
    id: searchDialog
    searchWidget: Calendar.SearchWidget {
      anchors.fill: parent
    }
  }
}
