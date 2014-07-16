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


#include "selectrelationalmatchtype.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

#include <KComboBox>

#include <KLocalizedString>
#include <QHBoxLayout>

using namespace KSieveUi;

SelectRelationalMatchType::SelectRelationalMatchType(QWidget *parent)
    : QWidget(parent)
{
    initialize();
}

SelectRelationalMatchType::~SelectRelationalMatchType()
{

}

void SelectRelationalMatchType::setCode(const QString &type, const QString &comparatorStr, const QString &name, QString &error)
{
    int index = mType->findData(type);
    if (index != -1) {
        mType->setCurrentIndex(index);
    } else {
        AutoCreateScriptUtil::comboboxItemNotFound(type, name, error);
        mType->setCurrentIndex(0);
    }

    index = mMatch->findData(comparatorStr);
    if (index != -1) {
        mMatch->setCurrentIndex(index);
    } else {
        AutoCreateScriptUtil::comboboxItemNotFound(comparatorStr, name, error);
        mMatch->setCurrentIndex(0);
    }
}

QString SelectRelationalMatchType::code() const
{
    return QString::fromLatin1("%1 \"%2\"").arg(mType->itemData(mType->currentIndex()).toString()).arg(mMatch->itemData(mMatch->currentIndex()).toString());
}

void SelectRelationalMatchType::initialize()
{
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    setLayout(lay);

    mType = new KComboBox;
    mType->addItem(i18n("Value"), QLatin1String(":value"));
    mType->addItem(i18n("Count"), QLatin1String(":count"));
    lay->addWidget(mType);
    connect(mType, SIGNAL(activated(int)), this, SIGNAL(valueChanged()));

    mMatch = new KComboBox;
    mMatch->addItem(i18n("Greater than"), QLatin1String("gt"));
    mMatch->addItem(i18n("Greater than or equal"), QLatin1String("ge"));
    mMatch->addItem(i18n("Less than"), QLatin1String("lt"));
    mMatch->addItem(i18n("Less than or equal"), QLatin1String("le"));
    mMatch->addItem(i18n("Equal to"), QLatin1String("eq"));
    mMatch->addItem(i18n("Not equal to"), QLatin1String("ne"));
    connect(mMatch, SIGNAL(activated(int)), this, SIGNAL(valueChanged()));
    lay->addWidget(mMatch);
}

