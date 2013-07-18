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


#include "sieveactionnotify.h"
#include "widgets/selectimportancecombobox.h"

#include <KLocale>
#include <KLineEdit>


#include <QHBoxLayout>
#include <QLabel>

using namespace KSieveUi;

SieveActionNotify::SieveActionNotify(QObject *parent)
    : SieveAction(QLatin1String("notify"), i18n("Notify"), parent)
{
}

SieveAction* SieveActionNotify::newAction()
{
    return new SieveActionNotify;
}

QWidget *SieveActionNotify::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    SelectImportanceCombobox *importanceCombobox = new SelectImportanceCombobox;
    importanceCombobox->setObjectName(QLatin1String("importancecombo"));
    lay->addWidget(importanceCombobox);

    QLabel *lab = new QLabel(i18n("message:"));
    lay->addWidget(lab);

    KLineEdit *message = new KLineEdit;
    message->setObjectName(QLatin1String("message"));
    lay->addWidget(message);

    return w;
}

void SieveActionNotify::setParamWidgetValue(QWidget *w ) const
{

}

QString SieveActionNotify::code(QWidget *w) const
{
    const SelectImportanceCombobox *importance = w->findChild<SelectImportanceCombobox*>( QLatin1String("importancecombo") );
    const QString importanceStr = importance->code();

    const KLineEdit *message = w->findChild<KLineEdit*>( QLatin1String("message") );
    const QString messageStr = message->text();

    return QString::fromLatin1("notify :importance \"%1\" :message \"%2\";").arg(importanceStr).arg(messageStr);
}

QString SieveActionNotify::serverNeedsCapability() const
{
    return QLatin1String("enotify");
}

bool SieveActionNotify::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionNotify::help() const
{
    return i18n("The \"notify\" action specifies that a notification should be sent to a user.");
}

#include "sieveactionnotify.moc"
