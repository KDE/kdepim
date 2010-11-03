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
import org.kde.akonadi.events 4.5 as Events

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
    } else if ( guiStateManager.inMultipleFolderScreenState ) {
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

  SystemPalette { id: palette; colorGroup: "Active" }

  function showDate(date)
  {
    agenda.showRange( date, 0 /* "Day" */ );
    guiStateManager.pushState( Events.EventsGuiStateManager.ViewDayState );
  }

  function showEventView()
  {
    guiStateManager.pushState( KPIM.GuiStateManager.ViewSingleItemState );
  }

  KCal.IncidenceView {
    id: eventView
    anchors { fill: parent; topMargin: 48; leftMargin: 48 }
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

    KPIM.Button {
      anchors.bottom: backButton.top
      anchors.right: parent.right
      anchors.margins: 12
      width: 70
      height: 70
      icon: KDE.locate( "data", "mobileui/edit-button.png" );
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

  Rectangle {
    id: monthView
    visible: guiStateManager.inViewMonthState
    anchors.fill: parent

    Rectangle {
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

  Rectangle {
    // TODO: reuse the button? we have it 3x here.
    id: timlineView
    visible: guiStateManager.inViewTimelineState
    anchors.fill: parent

    Rectangle {
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

    CalendarViews.TimelineView {
      id: timeline
      anchors { fill: parent; topMargin: 10; leftMargin: 40 }
      calendar: calendarModel
      swipeLength: 0.2 // Require at least 20% of screenwidth to trigger next or prev

      onItemSelected: {
        if ( selectedItemId > 0 ) {
          timelineView.itemId = selectedItemId;
          timelineView.activeDate = activeDate;
          application.setCurrentEventItemId(selectedItemId);
          guiStateManager.pushUniqueState( KPIM.GuiStateManager.ViewSingleItemState );
        }
      }
    }
  }

  Rectangle {
    id: eventListView
    visible: guiStateManager.inViewEventListState
    anchors.fill: parent
    color: "#D2D1D0" // TODO: make palette work correctly. palette.window

    Rectangle {
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

    EventListView {
      showCheckBox : false
      id: eventList
      model: itemModel
      checkModel : _itemActionModel
      anchors { fill: parent; topMargin: 30; leftMargin: 40 }

      navigationModel : _itemNavigationModel
    }
    Connections {
      target : _itemNavigationModel
      onCurrentRowChanged : {
        application.setCurrentEventItemId(_itemNavigationModel.currentItemIdHack);
        guiStateManager.pushUniqueState( KPIM.GuiStateManager.ViewSingleItemState )
        _itemActionModel.select( _itemNavigationModel.currentRow, 3 );
      }
    }
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

      breadcrumbComponentFactory : _breadcrumbNavigationFactory

      // It's not possible to get the number of items in a model. We have to
      // put the model in a view and count the items in the view.
      ListView { id : dummyItemView; model : calendarModel }

      multipleSelectionText : KDE.i18nc("%1 is e.g. 3 folders, %2 is e.g. from 2 accounts, %3 is e.g. 9 events",
                                        "You have selected \n%1\n%2\n%3",
                                        KDE.i18np("1 folder","%1 folders",collectionView.numSelected),
                                        KDE.i18np("from 1 account","from %1 accounts",application.numSelectedAccounts),
                                        KDE.i18np("1 event","%1 events",dummyItemView.count))

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
          // FIXME we'd need to translate that, QDateEdit should default to the current locale, I think
          //displayFormat: "MM.dd.yyyy"
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
            month.showMonth( dateEdit.date );
            guiStateManager.pushState( Events.EventsGuiStateManager.ViewMonthState );
          }
        }

        KPIM.Button2 {
          id: timelineButton
          buttonText: KDE.i18n( "Timeline view" )
          width: parent.width / 4
          onClicked: {
            timeline.showRange( dateEdit.date, 1 /* "Week" */ );
            guiStateManager.pushState( Events.EventsGuiStateManager.ViewTimelineState );
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
    }
  }

  SlideoutPanelContainer {
    anchors.fill: parent
    visible: !guiStateManager.inBulkActionScreenState && !guiStateManager.inMultipleFolderSelectionScreenState

    SlideoutPanel {
      id: actionPanel
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
                month.showMonth( dateEdit.date );
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
              name : "show_today"
              script : {
                agenda.showToday();
                actionPanel.collapse();
              }
            },
            KPIM.ScriptAction {
              name : "start_maintenance"
              script : {
                actionPanel.collapse();
                guiStateManager.pushState( KPIM.GuiStateManager.BulkActionScreenState );
              }
            },
            KPIM.ScriptAction {
              name : "edit_event"
              script: {
                actionPanel.collapse();
                application.editIncidence( eventView.item, eventView.activeDate );
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
            application.saveAttachment(url);
          }
        }
      ]
    }
  }

  KPIM.MultipleSelectionScreen {
    id : favoriteSelector
    anchors.fill : parent
    anchors.topMargin : 12
    visible : guiStateManager.inMultipleFolderSelectionScreenState
    backgroundImage : backgroundImage.source
    onFinished : {
      guiStateManager.popState();
      application.multipleSelectionFinished();
    }
    onCanceled : {
      guiStateManager.popState();
    }
  }

  KPIM.BulkActionScreen {
    id : bulkActionScreen
    visible : guiStateManager.inBulkActionScreenState
    anchors.top: parent.top
    anchors.topMargin : 12
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right : parent.right
    backgroundImage : backgroundImage.source

    actionListWidth : 1/3 * parent.width
    multipleText : KDE.i18np("1 calendar", "%1 calendars", collectionView.numSelected)
    selectedItemModel : _breadcrumbNavigationFactory.qmlSelectedItemModel();
    headerList : EventListView {
      showCheckBox : true
      id: bulkActionHeaderList
      model: itemModel
      checkModel : _itemActionModel
      anchors.fill : parent
    }
    onBackClicked : {
      guiStateManager.popState();
    }
  }

  KPIM.SearchResultScreen {
    id : searchResultScreen
    visible : guiStateManager.inSearchResultScreenState
    anchors.top: parent.top
    anchors.topMargin : 12
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right : parent.right
    backgroundImage : backgroundImage.source

    actionListWidth : 1/3 * parent.width
    multipleText : KDE.i18np("1 calendar", "%1 calendars", collectionView.numSelected)
    selectedItemModel : _breadcrumbNavigationFactory.qmlSelectedItemModel();
    headerList : EventListView {
      showCheckBox : true
      id: searchResultHeaderList
      model: itemModel
      checkModel : _itemActionModel
      anchors.fill : parent
    }
    onBackClicked : {
      searchManager.stopSearch();
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
      guiStateManager.popState();
    }
  }

  KPIM.AboutDialog {
    id : aboutDialog
    source: backgroundImage.source
  }

  ConfigDialog {
    id: configDialog
  }

  SearchDialog {
    id: searchDialog
  }
}
