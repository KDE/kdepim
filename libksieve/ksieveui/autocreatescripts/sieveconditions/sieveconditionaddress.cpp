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
#include <KLineEdit>
#include <KLocale>

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
    KLineEdit *edit = new KLineEdit(parent);
    edit->setObjectName(QLatin1String("editaddress"));
    return edit;
}

QString SieveConditionAddress::code(QWidget *w) const
{
    KLineEdit *edit = w->findChild<KLineEdit*>( QLatin1String("editaddress") );
    //TODO
    return QString::fromLatin1("address:%1").arg(edit->text());
}

#include "sieveconditionaddress.moc"
