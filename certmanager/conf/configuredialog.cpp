/*  -*- mode: C++; c-file-style: "gnu"; c-basic-offset: 2 -*-
    configuredialog.cpp

    This file is part of kgpgcertmanager
    Copyright (C) 2000 Espen Sand, espen@kde.org
    Copyright (C) 2001-2002 Marc Mutz <mutz@kde.org>
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#include "configuredialog.h"
#include "configuredialog_p.h"
#include <klocale.h>
#include <kwin.h>
#include <qlayout.h>
#include <kapplication.h>
#include <kconfig.h>


ConfigureDialog::ConfigureDialog( QWidget *parent, const char *name, bool modal )
  : KCMultiDialog( KDialogBase::IconList, i18n( "Configure" ), parent, name, modal )
{
  KWin::setIcons( winId(), kapp->icon(), kapp->miniIcon() );
  showButton( User1, true );

  addModule ( "kgpgcertmanager_config_dirserv", false );

  // We store the size of the dialog on hide, because otherwise
  // the KCMultiDialog starts with the size of the first kcm, not
  // the largest one. This way at least after the first showing of
  // the largest kcm the size is kept.
  KConfigGroup geometry( KGlobal::config(), "Geometry" );
  int width = geometry.readNumEntry( "ConfigureDialogWidth" );
  int height = geometry.readNumEntry( "ConfigureDialogHeight" );
  if ( width != 0 && height != 0 ) {
     setMinimumSize( width, height );
  }

}

void ConfigureDialog::hideEvent( QHideEvent * ) {
  KConfigGroup geometry( KGlobal::config(), "Geometry" );
  geometry.writeEntry( "ConfigureDialogWidth", width() );
  geometry.writeEntry( "ConfigureDialogHeight",height() );
}

ConfigureDialog::~ConfigureDialog() {
}

#include "configuredialog.moc"
