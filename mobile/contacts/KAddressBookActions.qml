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
import org.kde.pim.mobileui 4.5

ActionMenuContainer {

  actionItemHeight : 70
  actionItemWidth : 200
  actionItemSpacing : 2
  ReorderList {
    category : "home"
    name : "Favorites"

    delegate : QML.Component {
      QML.Text { height : 20; text : model.display }
    }
    upAction : "fav_up"
    downAction : "fav_down"
    deleteAction : "fav_delete"
    model : favoritesList
  }
  ReorderList {
    category : "home"
    name : "Addressbooks"

    delegate : QML.Component {
      QML.Text { height : 20; text : model.display }
    }
    upAction : "addressbook_up"
    downAction : "addressbook_down"
    deleteAction : "addressbook_delete"
    model : allFoldersModel
  }
  ActionList {
    category : "home"
    name : "View"
    FakeAction { name : "select_multiple_folders" }
  }

  ActionList {
    category : "addressbook"
    name : "Addressbook"
    ActionListItem { name : "edit_addressbook" }
    FakeAction{ name : "add_subfolder" }
  }

  ActionList {
    category : "single_folder"
    name : "Folder"
    ActionListItem { name : "edit_folder" }
    ActionListItem { name : "delete_folder" }
    ActionListItem { name : "add_subfolder" }
  }
  ActionList {
    category : "single_folder"
    name : "View"
    ActionListItem { name : "save_view_as_favorite" }
    ActionListItem { name : "start_maintenance" }
    ActionListItem { name : "filter_view" }
    ActionListItem { name : "view_options" }
  }

  ActionList {
    category : "multiple_folder"
    name : "View"
    ActionListItem { name : "save_view_as_favorite" }
    ActionListItem { name : "start_maintenance" }
  }

  ActionList {
    category : "contact_viewer"
    name : "Contact"
    ActionListItem { name : "copy_to_addressbook" }
    ActionListItem { name : "move_to_addressbook" }
    ActionListItem { name : "delete" }
    ActionListItem { name : "edit" }
  }
  ApplicationGeneralActions {
    category : "standard"
    name : "KAddressBook"
    type : "contacts"
  }
}
