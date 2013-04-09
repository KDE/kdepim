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

#include "sieveactionabstractflags.h"

#include "pimcommon/minimumcombobox.h"

#include <KLocale>

using namespace KSieveUi;
SieveActionAbstractFlags::SieveActionAbstractFlags(const QString &name, const QString &label, QObject *parent)
    : SieveAction(name, label, parent)
{
    //TODO add flags
}

QWidget *SieveActionAbstractFlags::createParamWidget( QWidget *parent ) const
{
    PimCommon::MinimumComboBox *comboBox = new PimCommon::MinimumComboBox( parent );
    comboBox->setObjectName("flags");
    //TODO
    return comboBox;
}

QString SieveActionAbstractFlags::code(QWidget *w) const
{
    PimCommon::MinimumComboBox *comboBox = w->findChild<PimCommon::MinimumComboBox*>( "flags" );
    QString str = flagsCode(w);
    //TODO
    return str;
}

QStringList SieveActionAbstractFlags::needRequires() const
{
    return QStringList() <<QLatin1String("imapflags");
}

#include "sieveactionabstractflags.moc"
