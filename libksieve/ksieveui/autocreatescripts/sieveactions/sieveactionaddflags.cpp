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


#include "sieveactionaddflags.h"
#include "pimcommon/minimumcombobox.h"

#include <KLocale>

using namespace KSieveUi;
SieveActionAddFlags::SieveActionAddFlags(QObject *parent)
    : SieveAction(QLatin1String("addflags"), i18n("Add Flags"), parent)
{
    //TODO add flags
}

SieveAction* SieveActionAddFlags::newAction()
{
    return new SieveActionAddFlags;
}

QWidget *SieveActionAddFlags::createParamWidget( QWidget *parent ) const
{
    PimCommon::MinimumComboBox *comboBox = new PimCommon::MinimumComboBox( parent );
    comboBox->setObjectName("flags");
    //TODO
    return comboBox;
}

QString SieveActionAddFlags::code(QWidget *w) const
{
    PimCommon::MinimumComboBox *comboBox = w->findChild<PimCommon::MinimumComboBox*>( "flags" );
    //TODO
    return QString();
}

QStringList SieveActionAddFlags::needRequires() const
{
    return QStringList() <<QLatin1String("imapflags");
}

#include "sieveactionaddflags.moc"
