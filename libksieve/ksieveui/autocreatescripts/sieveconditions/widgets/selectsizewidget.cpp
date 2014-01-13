/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "selectsizewidget.h"
#include "selectsizetypecombobox.h"

#include <QSpinBox>

#include <QHBoxLayout>

using namespace KSieveUi;

SelectSizeWidget::SelectSizeWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(0);

    mSpinBoxSize = new QSpinBox;
    mSpinBoxSize->setMinimum(1);
    mSpinBoxSize->setMaximum(9999);
    hbox->addWidget(mSpinBoxSize);
    connect(mSpinBoxSize, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged()));

    mSelectSizeType = new SelectSizeTypeComboBox;
    hbox->addWidget(mSelectSizeType);

    setLayout(hbox);
}

SelectSizeWidget::~SelectSizeWidget()
{

}

QString SelectSizeWidget::code() const
{
    const QString type = mSelectSizeType->code();
    return QString::fromLatin1("%1%2").arg(mSpinBoxSize->value()).arg(type);
}

void SelectSizeWidget::setCode(qlonglong value, const QString &identifier, const QString &name, QString &error)
{
    if (identifier == QLatin1String("K")) {
        value /= 1024;
    } else if (identifier == QLatin1String("M")) {
        value /= (1024*1024);
    } else if (identifier == QLatin1String("G")) {
        value /= (1024*1024*1024);
    }
    mSelectSizeType->setCode(identifier, name, error);
    mSpinBoxSize->setValue(value);
}


