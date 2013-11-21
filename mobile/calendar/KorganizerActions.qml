/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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
import org.kde.pim.mobileui 4.5
import "../mobileui/ScreenFunctions.js" as Screen

ActionMenuContainer {

  menuStyle : true

  actionItemHeight: Screen.partition( height, 6 ) - actionItemSpacing
  actionItemWidth : 200
  actionItemSpacing : 2

  ActionList {
    category : "home"
    name : "home_menu"
    text : KDE.i18n( "Home" )
    ActionListItem { name : "synchronize_all_items" }
    ScriptActionItem { name : "to_selection_screen"; title : KDE.i18n( "Select Multiple Calendars" ) }
    ActionListItem { name : "import_events" }
    ActionListItem { name : "configure_categories" }
  }

  FavoriteManager{ model : favoritesList }

  AgentInstanceList {
    category : "home"
    name : "accounts_list"
    text : KDE.i18n( "Accounts" )

    model : agentInstanceList
  }

  ActionList {
    category : "account"
    name : "account_menu"
    text : KDE.i18n( "Account" )
    ActionListItem { name : "akonadi_resource_synchronize" }
    ActionListItem { name : "akonadi_resource_properties" }
    ActionListItem { name : "akonadi_collection_properties" }
    ActionListItem { name : "akonadi_collection_create" }
    ActionListItem { name : "archive_old_entries" }
  }

  ActionList {
    category : "account"
    name : "account_view_menu"
    text : KDE.i18n( "View" )
    ScriptActionItem { name : "add_as_favorite"; title : KDE.i18n( "Add View As Favorite" ) }
    ScriptActionItem { name : "start_maintenance"; title : KDE.i18n( "Switch To Editing Mode" ) }
  }

  ActionList {
    category : "single_folder"
    name : "single_folder_folder_menu"
    text : KDE.i18n( "Folder" )
    ActionListItem { name : "akonadi_collection_sync" }
    ActionListItem { name : "export_selected_events" }
    ActionListItem { name : "archive_old_entries" }
  }

  ActionList {
    category : "single_folder"
    name : "single_folder_edit_menu"
    text : KDE.i18n( "Edit" )
    ActionListItem { name : "akonadi_collection_properties" }
    ActionListItem { name : "akonadi_collection_create" }
    ActionListItem { name : "akonadi_collection_move_to_dialog" }
    ActionListItem { name : "akonadi_collection_copy_to_dialog" }
    ActionListItem { name : "akonadi_collection_delete" }
  }

  ActionList {
    category : "single_folder"
    name : "single_folder_view_menu"
    text : KDE.i18n( "View" )
    ScriptActionItem { name : "add_as_favorite"; title : KDE.i18n( "Add View As Favorite" ) }
    ScriptActionItem { name : "start_maintenance"; title : KDE.i18n( "Switch To Editing Mode" ) }
  }

  ActionList {
    category : "multiple_folder"
    name : "multi_folder_folder_menu"
    text : KDE.i18n( "Folders" )

    ActionListItem { name : "akonadi_collection_sync" }
    ActionListItem { name : "export_selected_events" }
    ActionListItem { name : "archive_old_entries" }
  }

  ActionList {
    category : "multiple_folder"
    name : "multi_folder_view_menu"
    text : KDE.i18n( "View" )
    ScriptActionItem { name : "to_selection_screen"; title : KDE.i18n( "Select Calendars" ) }
    ScriptActionItem { name : "add_as_favorite"; title : KDE.i18n( "Add View As Favorite" ) }
    ScriptActionItem { name : "start_maintenance"; title : KDE.i18n( "Switch To Editing Mode" ) }
  }

  ActionList {
    category : "single_calendar"
    name : "single_calendar_calendar_menu"
    text : KDE.i18n( "Choice" )
    ActionListItem { name : "akonadi_collection_sync" }
    ScriptActionItem { name : "show_today"; title : KDE.i18n( "Show Today" ) }
    ScriptActionItem { name : "day_layout"; title : KDE.i18n( "Day View" ) }
    ScriptActionItem { name : "three_day_layout"; title : KDE.i18n( "Next Three Days View" ) }
    ScriptActionItem { name : "week_layout"; title : KDE.i18n( "Week View" ) }
    ScriptActionItem { name : "work_week_layout"; title : KDE.i18n( "Work Week View" ) }
    ScriptActionItem { name : "month_layout"; title : KDE.i18n( "Month View" ) }
    ScriptActionItem { name : "eventlist_layout"; title : KDE.i18n( "Event List View" ) }
    ScriptActionItem { name : "timeline_layout"; title : KDE.i18n( "Timeline" ) }
  }

  ActionList {
    category : "single_calendar"
    name : "single_calendar_view_menu"
    text : KDE.i18n( "View" )
    ScriptActionItem { name : "add_as_favorite"; title : KDE.i18n( "Add View As Favorite" ) }
    ActionListItem { name : "set_calendar_colour" }
    ScriptActionItem { name : "to_selection_screen"; title : KDE.i18n( "Back To Folder Selection" ) }
  }

  ActionList {
    category : "multiple_calendar"
    name : "multi_calendar_calendars_menu"
    text : KDE.i18n( "Calendars" )
    ActionListItem { name : "akonadi_collection_sync" }
    ScriptActionItem { name : "show_today"; title : KDE.i18n( "Show Today" ) }
    ScriptActionItem { name : "day_layout"; title : KDE.i18n( "Day View" ) }
    ScriptActionItem { name : "three_day_layout"; title : KDE.i18n( "Next Three Days View" ) }
    ScriptActionItem { name : "week_layout"; title : KDE.i18n( "Week View" ) }
    ScriptActionItem { name : "work_week_layout"; title : KDE.i18n( "Work Week View" ) }
    ScriptActionItem { name : "month_layout"; title : KDE.i18n( "Month View" ) }
    ScriptActionItem { name : "eventlist_layout"; title : KDE.i18n( "Event List View" ) }
    ScriptActionItem { name : "timeline_layout"; title : KDE.i18n( "Timeline" ) }
  }

  ActionList {
    category : "multiple_calendar"
    name : "multi_calendar_view_menu"
    text : KDE.i18n( "View" )
    ScriptActionItem { name : "add_as_favorite"; title : KDE.i18n( "Add View As Favorite" ) }
    ScriptActionItem { name : "to_selection_screen"; title : KDE.i18n( "Select Calendars" ) }
  }

  ActionList {
    category : "event_viewer"
    name : "event_viewer_event_menu"
    text : KDE.i18n( "Event" )
    ActionListItem { name : "publish_item_information" }
    ActionListItem { name : "send_invitations_to_attendees" }
    ActionListItem { name : "send_status_update" }
    ActionListItem { name : "send_cancellation_to_attendees" }
    ActionListItem { name : "request_update" }
    /* doesn't do anything useful atm */
    /*ActionListItem { name : "request_change" }*/
    ActionListItem { name : "send_as_icalendar" }
    ActionListItem { name : "mail_freebusy" }
    ActionListItem { name : "upload_freebusy" }
  }

  ActionList {
    category : "event_viewer"
    name : "event_viewer_attachments_menu"
    text : KDE.i18n( "Attachments" )
    ActionListItem { name : "save_all_attachments" }
  }

  ActionList {
    category : "event_viewer"
    name : "event_viewer_edit_menu"
    text : KDE.i18n( "Edit" )
    ActionListItem { name : "akonadi_incidence_edit" }
    ActionListItem { name : "akonadi_item_copy_to_dialog" }
    ActionListItem { name : "akonadi_item_move_to_dialog" }
    ActionListItem { name : "akonadi_item_delete" }
  }

  ApplicationGeneralActions {
    category : "standard"
    name : "application_menu"
    text : KDE.i18n( "Calendar" )
    type : "event"

    addNewActionName: "akonadi_event_create"

    //TODO enable when SearchWidget::query() is implemented
    //searchActionTitle: KDE.i18n( "Search For Events" )
    configureActionTitle: KDE.i18n( "Configure Calendar" )
  }
}
