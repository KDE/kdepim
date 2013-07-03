/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

  actionItemHeight : Screen.partition( height, 6 ) - actionItemSpacing
  actionItemWidth : 200
  actionItemSpacing : 2

  ActionList {
    category : "home"
    name : "home_menu"
    text : KDE.i18n( "Home" )
    ActionListItem { name : "synchronize_all_items" }
    ScriptActionItem { name : "to_selection_screen"; title : KDE.i18n( "Select Multiple Task Lists" ) }
    ActionListItem { name : "import_tasks" }
    ActionListItem { name : "export_account_tasks" }
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
    ActionListItem { name : "akonadi_collection_create" }
    ActionListItem { name : "archive_old_entries" }
    ActionListItem { name : "purge_completed_tasks" }
  }

  ActionList {
    name : "single_folder_folder_menu"
    category : "single_folder"
    text : KDE.i18n( "Folder" )
    ActionListItem { name : "akonadi_collection_sync" }
    ActionListItem { name : "export_selected_tasks" }
    ActionListItem { name : "archive_old_entries" }
    ActionListItem { name : "purge_completed_tasks" }
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
    ActionListItem { name : "export_selected_tasks" }
    ActionListItem { name : "archive_old_entries" }
    ActionListItem { name : "purge_completed_todos" }
  }

  ActionList {
    category : "multiple_folder"
    name : "multi_folder_view_menu"
    text : KDE.i18n( "View" )
    ScriptActionItem { name : "add_as_favorite"; title : KDE.i18n( "Add View As Favorite" ); visible: !guiStateManager.inSearchResultScreenState }
    ScriptActionItem { name : "to_selection_screen"; title : KDE.i18n( "Select Task Lists" ) }
    ScriptActionItem { name : "start_maintenance"; title : KDE.i18n( "Switch To Editing Mode" ) }
  }

  ActionList {
    category : "todo_viewer"
    name : "todo_viewer_todo_menu"
    text : KDE.i18n( "Task" )
    ActionListItem { name : "akonadi_subtodo_create" }
    ActionListItem { name : "make_subtask_independent" }
    ActionListItem { name : "make_all_subtasks_independent" }
  }

  ActionList {
    category : "todo_viewer"
    name : "todo_viewer_attachment_menu"
    text : KDE.i18n( "Attachments" )
    ActionListItem { name : "save_all_attachments" }
  }

  ActionList {
    category : "todo_viewer"
    name : "todo_viewer_edit_menu"
    text : KDE.i18n( "Edit" )
    ActionListItem { name : "akonadi_incidence_edit" }
    ActionListItem { name : "akonadi_item_copy_to_dialog" }
    ActionListItem { name : "akonadi_item_move_to_dialog" }
    ActionListItem { name : "akonadi_item_delete" }
  }

  ApplicationGeneralActions {
    name : "application_menu"
    category : "standard"
    text : KDE.i18n( "Tasks" )
    type : "task"

    addNewActionName: "akonadi_todo_create"

    searchActionTitle: KDE.i18n( "Search For Tasks" )
    configureActionTitle: KDE.i18n( "Configure Tasks" )
  }
}
