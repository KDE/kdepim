/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include <KLocalizedString>

#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include "libksieve_debug.h"

namespace KSieveUi
{
SelectConvertParameterWidget::SelectConvertParameterWidget(QWidget *parent)
    : QWidget(parent)
{
    initialize();
}

SelectConvertParameterWidget::~SelectConvertParameterWidget()
{
}

void SelectConvertParameterWidget::setCode(const QStringList &code, QString &error)
{
    if (code.isEmpty()) {
        return;
    }

    if (code.count() < 2) {
        error += i18n("Not enough arguments for SelectConvertParameterWidget. Expected 2 arguments.");
        qCDebug(LIBKSIEVE_LOG) << " SelectConvertParameterWidget::setCode parsing error ?";
        return;
    }
    if (code.count() > 2) {
        error += i18n("Too many arguments for SelectConvertParameterWidget, \"%1\"", code.count());
        qCDebug(LIBKSIEVE_LOG) << " too many argument " << code.count();
    }

    QString widthStr = code.at(0);
    widthStr = widthStr.remove(QStringLiteral("pix-x="));

    QString heightStr = code.at(1);
    heightStr = heightStr.remove(QStringLiteral("pix-y="));
    mWidth->setValue(widthStr.toInt());
    mHeight->setValue(heightStr.toInt());
}

QString SelectConvertParameterWidget::code() const
{
    return QStringLiteral("[\"pix-x=%1\",\"pix-y=%2\"]").arg(mWidth->value()).arg(mHeight->value());
}

void SelectConvertParameterWidget::initialize()
{
    QBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(0);
    mWidth = new QSpinBox;
    mWidth->setSuffix(i18n(" px"));
    mWidth->setMinimum(1);
    mWidth->setMaximum(9999);
    mWidth->setValue(300);
    hbox->addWidget(mWidth);
    connect(mWidth, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &SelectConvertParameterWidget::valueChanged);

    QLabel *lab = new QLabel(QStringLiteral("x"));
    hbox->addWidget(lab);

    mHeight = new QSpinBox;
    mHeight->setSuffix(i18n(" px"));
    mHeight->setMinimum(1);
    mHeight->setMaximum(9999);
    mHeight->setValue(200);
    hbox->addWidget(mHeight);

    connect(mHeight, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &SelectConvertParameterWidget::valueChanged);
    setLayout(hbox);
}

}

