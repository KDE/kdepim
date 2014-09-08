/******************************************************************************
 *
 *  Copyright 2008 Szymon Tomasz Stefanek <pragma@kvirc.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#include "utils/comboboxutils.h"

#include <QVariant>

#include <KComboBox>

using namespace MessageList::Utils;

void ComboBoxUtils::fillIntegerOptionCombo(KComboBox *combo, const QList< QPair< QString, int > > &optionDescriptors)
{
    int val = getIntegerOptionComboValue(combo, -1);
    combo->clear();
    int valIdx = -1;
    int idx = 0;

    QList< QPair< QString, int > >::ConstIterator end(optionDescriptors.end());

    for (QList< QPair< QString, int > >::ConstIterator it = optionDescriptors.constBegin(); it != end; ++it) {
        if (val == (*it).second) {
            valIdx = idx;
        }
        combo->addItem((*it).first, QVariant((*it).second));
        ++idx;
    }
    if (idx == 0) {
        combo->addItem(QLatin1String("-"), QVariant((int)0));     // always default to 0
        combo->setEnabled(false);
    } else {
        if (!combo->isEnabled()) {
            combo->setEnabled(true);
        }
        if (valIdx >= 0) {
            combo->setCurrentIndex(valIdx);
        }
        if (combo->count() == 1) {
            combo->setEnabled(false);    // disable when there is no choice
        }
    }
}

void ComboBoxUtils::setIntegerOptionComboValue(KComboBox *combo, int value)
{
    if (combo->itemData(combo->currentIndex()).toInt() == value) {
        return;
    }
    int index = combo->findData(value);
    if (index != -1) {
        combo->setCurrentIndex(index);
    } else {
        combo->setCurrentIndex(0);    // default
    }
}

int ComboBoxUtils::getIntegerOptionComboValue(KComboBox *combo, int defaultValue)
{
    const int idx = combo->currentIndex();
    if (idx < 0) {
        return defaultValue;
    }

    QVariant data = combo->itemData(idx);
    bool ok;
    const int val = data.toInt(&ok);
    if (!ok) {
        return defaultValue;
    }
    return val;
}

