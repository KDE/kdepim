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

#include <QHBoxLayout>

#include <KLocale>

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

    return w;
}


QString SieveActionNotify::code(QWidget *w) const
{
    const SelectImportanceCombobox *importance = w->findChild<SelectImportanceCombobox*>( QLatin1String("importancecombo") );
    const QString importanceStr = importance->code();
    //TODO
    return QString::fromLatin1("notify :importance \"%1\"").arg(importanceStr);
}

QString SieveActionNotify::serverNeedsCapability() const
{
    return QLatin1String("enotify");
}

bool SieveActionNotify::needCheckIfServerHasCapability() const
{
    return true;
}


#include "sieveactionnotify.moc"
