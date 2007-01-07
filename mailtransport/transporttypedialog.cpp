/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

    Based on KMail code by:
    Copyright (C) 2001-2003 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "transporttypedialog.h"
#include "ui_transporttypedialog.h"

using namespace MailTransport;

TransportTypeDialog::TransportTypeDialog(QWidget * parent) :
    KDialog( parent )
{
  setCaption( i18n("Add Transport") );
  setButtons( Ok|Cancel );
  Ui::TransportTypeDialog ui;
  ui.setupUi( mainWidget() );
  mButtonGroup = ui.kcfg_type;
  mButtonGroup->setSelected( 0 );
}

int TransportTypeDialog::transportType() const
{
  return mButtonGroup->selected();
}
