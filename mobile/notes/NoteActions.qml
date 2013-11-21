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

import QtQuick 1.0
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
    ScriptActionItem { name : "to_selection_screen"; title : KDE.i18n( "Select Multiple Notebooks" ) }
    ActionListItem { name : "import_notes" }
  }

  FavoriteManager{ model : favoritesList }

  AgentInstanceList {
    name : "accounts_list"
    category : "home"
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
    ActionListItem { name : "export_account_notes" }
  }

  ActionList {
    category : "single_folder"
    name : "single_folder_folder_menu"
    text : KDE.i18n( "Folder" )
    ActionListItem { name : "akonadi_collection_sync" }
    ActionListItem { name : "export_selected_notes" }
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
    ActionListItem { name : "export_selected_notes" }
  }

  ActionList {
    name : "multi_folder_view_menu"
    category : "multiple_folder"
    text : KDE.i18n( "View" )
    ScriptActionItem { name : "add_as_favorite"; title : KDE.i18n( "Add View As Favorite" ); visible: !guiStateManager.inSearchResultScreenState }
    ScriptActionItem { name : "to_selection_screen"; title : KDE.i18n( "Select Notebooks" ) }
    ScriptActionItem { name : "start_maintenance"; title : KDE.i18n( "Switch To Editing Mode" ) }
  }

  ActionList {
    name : "note_viewer_edit_menu"
    category : "note_viewer"
    text : KDE.i18n( "Edit" )
    ActionListItem { name : "akonadi_item_copy_to_dialog" }
    ActionListItem { name : "akonadi_item_move_to_dialog" }
    ActionListItem { name : "akonadi_item_delete" }
  }

  ApplicationGeneralActions {
    category : "standard"
    name : "application_menu"
    text : KDE.i18n( "Notes" )
    type : "note"
    configureActionVisible: false

    addNewActionName: "add_new_note"
    //TODO enable when SearchWidget::query() is implemented
    //searchActionTitle: KDE.i18n( "Search For Notes" )
  }
}
