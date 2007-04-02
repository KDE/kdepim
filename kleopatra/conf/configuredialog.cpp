/*
    configuredialog.cpp

    This file is part of kleopatra
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

#include "configuredialog.h"

#include <kwin.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kcmultidialog.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include <QApplication>
#include <QIcon>
#include <QHideEvent>

ConfigureDialog::ConfigureDialog( QWidget *parent, bool modal )
  : KCMultiDialog( parent )
{
  setFaceType( KPageDialog::List );
  setCaption( i18n( "Configure" ) );
  setModal( modal );
#ifdef Q_OS_UNIX
  KWin::setIcons( winId(), qApp->windowIcon().pixmap( IconSize( K3Icon::Desktop ), IconSize( K3Icon::Desktop ) ),
                  qApp->windowIcon().pixmap( IconSize( K3Icon::Small ), IconSize( K3Icon::Small ) ) );
#endif
  showButton( User1, true );

  addModule( "kleopatra_config_dirserv" );
  addModule( "kleopatra_config_appear" );
  addModule( "kleopatra_config_dnorder" );

  // We store the size of the dialog on hide, because otherwise
  // the KCMultiDialog starts with the size of the first kcm, not
  // the largest one. This way at least after the first showing of
  // the largest kcm the size is kept.
  const KConfigGroup geometry( KGlobal::config(), "Geometry" );
  const int width = geometry.readEntry( "ConfigureDialogWidth", 0);
  const int height = geometry.readEntry( "ConfigureDialogHeight", 0 );
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
