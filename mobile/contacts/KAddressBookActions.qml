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

import Qt 4.7 as QML
import org.kde 4.5
import org.kde.pim.mobileui 4.5

ActionMenuContainer {

  actionItemHeight : height / 6 - actionItemSpacing
  actionItemWidth : 200
  actionItemSpacing : 2

  ReorderList {
    category : "home"
    name : KDE.i18n( "Favorites" )

    delegate : QML.Component {
      QML.Text { height : 20; text : model.display }
    }
    upAction : "fav_up"
    downAction : "fav_down"
    deleteAction : "fav_delete"
    model : favoritesList

    customActions : [
      FakeAction { name : "delete_favorite" },
      FakeAction { name : "un_mark_as_default" }
    ]
  }

  AgentInstanceList {
    category : "home"
    name : KDE.i18n( "Address Books" )

    model : agentInstanceList

    customActions : [
      ActionListItem { name : "akonadi_agentinstance_delete" },
      ActionListItem { name : "akonadi_agentinstance_configure" },
      ActionListItem { name : "akonadi_agentinstance_create" }
    ]
  }

  ActionList {
    category : "home"
    name : KDE.i18n( "View" )
    FakeAction { name : "select_multiple_folders" }
  }

  ActionList {
    category : "resource"
    name : KDE.i18n( "Address Book" )
    ActionListItem { name : "akonadi_resource_properties" }
    ActionListItem { name : "akonadi_collection_create" }
  }

  ActionList {
    category : "single_folder"
    name : KDE.i18n( "Folder" )
    ActionListItem { name : "akonadi_collection_properties" }
    ActionListItem { name : "akonadi_collection_create" }
    ActionListItem { name : "akonadi_collection_copy_to_menu" }
    ActionListItem { name : "akonadi_collection_move_to_menu" }
    ActionListItem { name : "akonadi_collection_sync" }
    ActionListItem { name : "akonadi_collection_delete" }
  }

  ActionList {
    category : "single_folder"
    name : KDE.i18n( "View" )
    FakeAction { name : "save_view_as_favorite" }
    FakeAction { name : "start_maintenance" }
    FakeAction { name : "filter_view" }
    FakeAction { name : "view_options" }
  }

  ActionList {
    category : "multiple_folder"
    name : KDE.i18n( "View" )
    FakeAction { name : "save_view_as_favorite" }
    FakeAction { name : "start_maintenance" }
  }

  ActionList {
    category : "contact_viewer"
    name : KDE.i18n( "Contact" )
    ActionListItem { name : "akonadi_item_copy_to_menu" }
    ActionListItem { name : "akonadi_item_move_to_menu" }
    ActionListItem { name : "akonadi_item_delete" }
    ActionListItem { name : "akonadi_contact_item_edit" }
  }

  ApplicationGeneralActions {
    category : "standard"
    name : KDE.i18n( "KAddressBook" )
    type : "contacts"
  }
}
