/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "filteractionwithstringlist.h"

#include <pimcommon/widgets/minimumcombobox.h>

using namespace MailCommon;

FilterActionWithStringList::FilterActionWithStringList(const QString &name, const QString &label, QObject *parent)
    : FilterActionWithString(name, label, parent)
{
}

QWidget *FilterActionWithStringList::createParamWidget(QWidget *parent) const
{
    PimCommon::MinimumComboBox *comboBox = new PimCommon::MinimumComboBox(parent);
    comboBox->setEditable(false);
    comboBox->addItems(mParameterList);
    setParamWidgetValue(comboBox);

    connect(comboBox, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(filterActionModified()));

    return comboBox;
}

void FilterActionWithStringList::applyParamWidgetValue(QWidget *paramWidget)
{
    mParameter = static_cast<PimCommon::MinimumComboBox *>(paramWidget)->currentText();
}

void FilterActionWithStringList::setParamWidgetValue(QWidget *paramWidget) const
{
    const int index = mParameterList.indexOf(mParameter);
    static_cast<PimCommon::MinimumComboBox *>(paramWidget)->setCurrentIndex(index >= 0 ? index : 0);
}

void FilterActionWithStringList::clearParamWidget(QWidget *paramWidget) const
{
    static_cast<PimCommon::MinimumComboBox *>(paramWidget)->setCurrentIndex(0);
}

void FilterActionWithStringList::argsFromString(const QString &argsStr)
{
    int index = mParameterList.indexOf(argsStr);
    if (index < 0) {
        mParameterList.append(argsStr);
        index = mParameterList.count() - 1;
    }

    mParameter = mParameterList.at(index);
}

