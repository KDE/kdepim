/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "addsieveserverdialog.h"

#include <KLocalizedString>
#include <QVBoxLayout>

AddSieveServerDialog::AddSieveServerDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Add Server Sieve" ) );
    setButtons( Cancel | Ok  );

    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);
    setMainWidget(w);
}

AddSieveServerDialog::~AddSieveServerDialog()
{

}
