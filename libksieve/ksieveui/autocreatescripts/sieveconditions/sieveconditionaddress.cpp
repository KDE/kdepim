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

#include "sieveconditionaddress.h"
#include "widgets/selectaddresspartcombobox.h"
#include "widgets/selectmatchtypecombobox.h"
#include "widgets/selectheadertypecombobox.h"

#include <KLineEdit>
#include <KLocale>

#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

using namespace KSieveUi;

SieveConditionAddress::SieveConditionAddress(QObject *parent)
    : SieveCondition(QLatin1String("address"), i18n("Address"), parent)
{
}

SieveCondition *SieveConditionAddress::newAction()
{
    return new SieveConditionAddress;
}

QWidget *SieveConditionAddress::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    w->setLayout(lay);


    SelectAddressPartComboBox *selectAddressPart = new SelectAddressPartComboBox;
    selectAddressPart->setObjectName(QLatin1String("addresspartcombobox"));
    lay->addWidget(selectAddressPart);

    SelectMatchTypeComboBox *selectMatchCombobox = new SelectMatchTypeComboBox;
    selectMatchCombobox->setObjectName(QLatin1String("matchtypecombobox"));
    lay->addWidget(selectMatchCombobox);

    SelectHeaderTypeComboBox *selectHeaderType = new SelectHeaderTypeComboBox;
    selectHeaderType->setObjectName(QLatin1String("headertypecombobox"));
    lay->addWidget(selectHeaderType);

    QLabel *lab = new QLabel(i18n("address:"));
    lay->addWidget(lab);

    KLineEdit *edit = new KLineEdit;
    lay->addWidget(edit);
    edit->setObjectName(QLatin1String("editaddress"));
    return w;
}

QString SieveConditionAddress::code(QWidget *w) const
{
    SelectMatchTypeComboBox *selectMatchCombobox = w->findChild<SelectMatchTypeComboBox*>(QLatin1String("matchtypecombobox"));
    const QString matchTypeStr = selectMatchCombobox->code();

    SelectAddressPartComboBox *selectAddressPart = w->findChild<SelectAddressPartComboBox*>(QLatin1String("addresspartcombobox"));
    const QString selectAddressPartStr = selectAddressPart->code();

    SelectHeaderTypeComboBox *selectHeaderType = w->findChild<SelectHeaderTypeComboBox*>(QLatin1String("headertypecombobox"));
    const QString selectHeaderTypeStr = selectHeaderType->code();


    KLineEdit *edit = w->findChild<KLineEdit*>( QLatin1String("editaddress") );
    const QString addressStr = edit->text();
    return QString::fromLatin1("address %1 %2 \"%3\" \"%4\"").arg(selectAddressPartStr).arg(matchTypeStr).arg(selectHeaderTypeStr).arg(addressStr);
}

#include "sieveconditionaddress.moc"
