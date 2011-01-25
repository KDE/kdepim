/* -*- mode: javascript; c-basic-offset:2 -*-
    KleopatraActions.qml

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2010 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

import Qt 4.7 as QML
import org.kde 4.5
import org.kde.pim.mobileui 4.5

ActionMenuContainer {

  menuStyle : true

  actionItemHeight : height / 7 - actionItemSpacing
  actionItemWidth : 200
  actionItemSpacing : 2

  ActionList {
    category : "home"
    name : "file_menu"
    text : KDE.i18n( "File" )
    ActionListItem { name : "file_new_certificate" }
    ActionListItem { name : "file_lookup_certificates" }
    ActionListItem { name : "file_import_certificates" }
    ActionListItem { name : "file_export_certificates" }
    ActionListItem { name : "file_export_secret_keys" }
    /* ActionListItem { name : "file_export_certificates_to_server" } */
    ActionListItem { name : "file_quit" }
  }

  ActionList {
    category : "home"
    name : "view_menu"
    text : KDE.i18n( "View" )
    ActionListItem { name : "view_redisplay" }
    ActionListItem { name : "view_stop_operations" }
    ActionListItem { name : "view_certificate_details" }
  }

  ActionList {
    category : "home"
    name : "cert_menu"
    text : KDE.i18n( "Certificates" )
    ActionListItem { name : "certificates_change_owner_trust" }
    ActionListItem { name : "certificates_trust_root" }
    ActionListItem { name : "certificates_distrust_root" }
    ActionListItem { name : "certificates_certify_certificate" }
    ActionListItem { name : "certificates_change_expiry" }
    ActionListItem { name : "certificates_change_passphrase" }
    ActionListItem { name : "certificates_delete" }
    ActionListItem { name : "certificates_dump_certificate" }
  }

  ActionList {
    category : "home"
    name : "tools_menu"
    text : KDE.i18n( "Tools" )
    ActionListItem { name : "tools_refresh_openpgp_certificates" }
    ActionListItem { name : "tools_refresh_x509_certificates" }
    ActionListItem { name : "crl_import_crl" }
    ActionListItem { name : "crl_clear_crl_cache" }
    ActionListItem { name : "crl_dump_crl_cache" }
  }

  ActionList {
    category : "home"
    name : "settings_menu"
    text : KDE.i18n( "Settings" )
    ActionListItem { name : "settings_self_test" }
    ActionListItem { name : "options_configure"  }
  }

/*
  ActionList {
    category : "home"
    name : "help_menu"
    text : KDE.i18n( "Help" )
    ActionListItem { name : "help_about_kleopatra" }
  }
*/

}
