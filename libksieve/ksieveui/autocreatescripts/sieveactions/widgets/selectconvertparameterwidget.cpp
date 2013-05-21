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

#include "selectconvertparameterwidget.h"

#include <KLocale>

#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>

namespace KSieveUi {
SelectConvertParameterWidget::SelectConvertParameterWidget(QWidget *parent)
    : QWidget(parent)
{
    initialize();
}

SelectConvertParameterWidget::~SelectConvertParameterWidget()
{

}

QString SelectConvertParameterWidget::code() const
{
    //TODO
    return QString();
}

void SelectConvertParameterWidget::initialize()
{
    QBoxLayout *hbox = new QHBoxLayout;
    mWidth = new QSpinBox;
    hbox->addWidget(mWidth);

    QLabel *lab = new QLabel(i18n("x"));
    hbox->addWidget(lab);

    mHeight = new QSpinBox;
    hbox->addWidget(mHeight);

    setLayout(hbox);
}

}

#include "selectconvertparameterwidget.moc"
