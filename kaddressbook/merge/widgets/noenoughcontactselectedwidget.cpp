/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "noenoughcontactselectedwidget.h"
#include <KLocalizedString>

using namespace KABMergeContacts;
NoEnoughContactSelectedWidget::NoEnoughContactSelectedWidget(QWidget *parent)
    : QLabel(parent)
{
    QFont font;
    font.setBold(true);
    setText(i18n("You must select at least two elements."));
    setObjectName(QLatin1String("noenoughcontactselected"));
    setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    setFont(font);
}

NoEnoughContactSelectedWidget::~NoEnoughContactSelectedWidget()
{

}
