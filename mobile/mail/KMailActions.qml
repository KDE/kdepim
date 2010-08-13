/*
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (C) 2010 Andras Mantia <andras@kdab.net>

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

  actionItemHeight : 70
  actionItemWidth : 200
  actionItemSpacing : 2
/*
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
  }

  ReorderList {
    category : "home"
    name : KDE.i18n( "Address Books" )

    delegate : QML.Component {
      QML.Text { height : 20; text : model.display }
    }
    upAction : "resource_up"
    downAction : "resource_down"
    deleteAction : "resource_delete"
    model : allFoldersModel
  }

  ActionList {
    category : "home"
    name : KDE.i18n( "View" )
    FakeAction { name : "select_multiple_folders" }
  }

  ActionList {
    category : "resource"
    name : KDE.i18n( "Address Book" )
    ActionListItem { name : "akonadi_addressbook_properties" }
    ActionListItem { name : "akonadi_collection_create" }
  }
*/
  ActionList {
    category: "mail_viewer"
    name: KDE.i18n( "Message" )
    ActionListItem { name : "message_reply" }
    ActionListItem { name : "message_reply_to_all" }
  }

  ActionList {
    category : "single_folder"
    name : KDE.i18n( "Emails" )
    ActionListItem { name : "akonadi_collection_sync" }
    ActionListItem { name : "akonadi_mark_all_as"
                     argument : "R"
                     title : KDE.i18n("Mark All As Read")
                   }
    ActionListItem { name : "akonadi_move_all_to_trash" }
    ActionListItem { name : "akonadi_remove_duplicates" }
  }
  ActionList {
    category : "single_folder"
    name : KDE.i18n( "Folder" )
    ActionListItem { name : "akonadi_collection_properties" }
    ActionListItem { name : "akonadi_collection_create" }
    ActionListItem { name : "akonadi_collection_delete" }
  }

 ActionList {
    category: "standard"
    name: KDE.i18n("Mail")
    ActionListItem { name : "write_new_email" }
    FakeAction { name : "find_message" }
  }

ActionList {
    category: "standard"
    name: KDE.i18n("Settings")
    FakeAction { name : "work_offline" }
    FakeAction { name : "configure_kmail" }
    FakeAction { name : "configure_notifications" }
  }
  
  ApplicationGeneralActions {
    category : "standard"
    name : KDE.i18n( "Application" )
    type : "mail"
  }
}
