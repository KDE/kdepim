/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

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

#include "transporttypedialog.h"
#include "ui_transporttypedialog.h"

using namespace KPIM;

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
