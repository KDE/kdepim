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

#include "sieveconditionexists.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "widgets/selectheadertypecombobox.h"

#include <KLocale>

#include <KLineEdit>

#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>

using namespace KSieveUi;

SieveConditionExists::SieveConditionExists(QObject *parent)
    : SieveCondition(QLatin1String("exists"), i18n("Exists"), parent)
{
}

SieveCondition *SieveConditionExists::newAction()
{
    return new SieveConditionExists;
}

QWidget *SieveConditionExists::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    QComboBox *combo = new QComboBox;
    combo->setObjectName(QLatin1String("existscheck"));
    combo->addItem(i18n("exists"), QLatin1String("exists"));
    combo->addItem(i18n("not exists"), QLatin1String("not exists"));
    lay->addWidget(combo);

    QLabel *lab = new QLabel(i18n("headers:"));
    lay->addWidget(lab);

    SelectHeaderTypeComboBox *value = new SelectHeaderTypeComboBox;
    value->setObjectName(QLatin1String("headervalue"));

    lay->addWidget(value);
    return w;
}

QString SieveConditionExists::code(QWidget *w) const
{
    const QComboBox *combo = w->findChild<QComboBox*>( QLatin1String("existscheck") );
    const QString comparaison = combo->itemData(combo->currentIndex()).toString();

    const SelectHeaderTypeComboBox *value = w->findChild<SelectHeaderTypeComboBox*>( QLatin1String("headervalue") );
    return QString::fromLatin1("%1 %2").arg(comparaison).arg(value->code());
}

QString SieveConditionExists::help() const
{
    return i18n("The \"exists\" test is true if the headers listed in the header-names argument exist within the message.  All of the headers must exist or the test is false.");
}

void SieveConditionExists::setParamWidgetValue(const QDomElement &element, QWidget *parent ) const
{

}

#include "sieveconditionexists.moc"
