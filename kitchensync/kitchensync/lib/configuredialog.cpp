/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
† † Copyright (c) 2002 Maximilian Reiﬂ <harlekin@handhelds.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qlayout.h>
#include <config.h>
#include <klocale.h>

#include "configuredialog.h"

using namespace KSync;


ConfigureDialog::ConfigureDialog( QWidget *parent,
				  const char *name,
				  bool modal )
  : KDialogBase( IconList,
		 i18n("Configure KitchenSync"),
		 Ok|Cancel,
		 Ok, parent, name, modal, true ) {

  setIconListAllVisible( false );
  resize(500,400);

}



ConfigureDialog::~ConfigureDialog() {
}

void ConfigureDialog::show() {
  if( !isVisible() ) {
    setup();
  }
  KDialogBase::show();
}

void ConfigureDialog::slotOk() {
  apply( true );
  accept();
  emit ok();
}


void ConfigureDialog::slotCancel() {
  apply( false );
  reject();
}


void ConfigureDialog::addWidget(QWidget* widget, const QString &name, QPixmap* pixmap) {
  QFrame *frame = addPage(name, name, *pixmap);
  QHBoxLayout *lay = new QHBoxLayout( frame );
  widget->reparent(frame, QPoint(0,0));
  lay->addWidget( widget );

}

void ConfigureDialog::setup() {
}

void ConfigureDialog::unload() {
}

void ConfigureDialog::apply(bool ) {

}

#include "configuredialog.moc"
