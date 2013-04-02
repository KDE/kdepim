/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "sieveactionwidgetlister.h"

using namespace KSieveUi;

SieveActionWidget::SieveActionWidget(QWidget *parent)
    : QWidget(parent)
{

}

SieveActionWidget::~SieveActionWidget()
{
}


SieveActionWidgetLister::SieveActionWidgetLister(QWidget *parent)
    : KPIM::KWidgetLister(false, 1, 15, parent)
{
}

SieveActionWidgetLister::~SieveActionWidgetLister()
{

}

void SieveActionWidgetLister::slotAddWidget( QWidget *w )
{
  addWidgetAfterThisWidget( w );
  updateAddRemoveButton();
}

void SieveActionWidgetLister::slotRemoveWidget( QWidget *w )
{
  removeWidget( w );
  updateAddRemoveButton();
}


void SieveActionWidgetLister::updateAddRemoveButton()
{

}

void SieveActionWidgetLister::generatedScript(QString &script)
{
    //TODO
}

#include "sieveactionwidgetlister.moc"
