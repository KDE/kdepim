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

#include "sieveconditionheader.h"
#include "autocreatescripts/commonwidgets/selectmatchtypecombobox.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "widgets/selectheadertypecombobox.h"

#include <KLocale>
#include <KLineEdit>

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

using namespace KSieveUi;

SieveConditionHeader::SieveConditionHeader(QObject *parent)
    : SieveCondition(QLatin1String("header"), i18n("Header"), parent)
{
}

SieveCondition *SieveConditionHeader::newAction()
{
    return new SieveConditionHeader;
}

QWidget *SieveConditionHeader::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    SelectMatchTypeComboBox *matchTypeCombo = new SelectMatchTypeComboBox;
    matchTypeCombo->setObjectName(QLatin1String("matchtypecombobox"));
    lay->addWidget(matchTypeCombo);

    SelectHeaderTypeComboBox *headerType = new SelectHeaderTypeComboBox;
    headerType->setObjectName(QLatin1String("headertype"));
    lay->addWidget(headerType);

    QLabel *lab = new QLabel(i18n("With value:"));
    lay->addWidget(lab);

    KLineEdit *value = new KLineEdit;
    value->setObjectName(QLatin1String("value"));
    lay->addWidget(value);
    return w;
}

QString SieveConditionHeader::code(QWidget *w) const
{
    const SelectMatchTypeComboBox *matchTypeCombo = w->findChild<SelectMatchTypeComboBox*>( QLatin1String("matchtypecombobox") );
    bool isNegative = false;
    const QString matchString = matchTypeCombo->code(isNegative);

    const SelectHeaderTypeComboBox *headerType = w->findChild<SelectHeaderTypeComboBox*>( QLatin1String("headertype") );
    const QString headerStr = headerType->code();

    const KLineEdit *value = w->findChild<KLineEdit*>( QLatin1String("value") );
    const QString valueStr = value->text();

    return AutoCreateScriptUtil::negativeString(isNegative) + QString::fromLatin1("header %1 %2 \"%3\"").arg(matchString).arg(headerStr).arg(valueStr);
}

QString SieveConditionHeader::help() const
{
    return i18n("The \"header\" test evaluates to true if the value of any of the named headers, ignoring leading and trailing whitespace, matches any key.");
}

#include "sieveconditionheader.moc"
