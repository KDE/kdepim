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
#include "widgets/selectflagswidget.h"

#include <QHBoxLayout>

#include <KLocale>

using namespace KSieveUi;
SieveActionAbstractFlags::SieveActionAbstractFlags(const QString &name, const QString &label, QObject *parent)
    : SieveAction(name, label, parent)
{
}

QWidget *SieveActionAbstractFlags::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);
    SelectFlagsWidget *flagsWidget = new SelectFlagsWidget;
    flagsWidget->setObjectName(QLatin1String("flagswidget"));
    lay->addWidget(flagsWidget);
    return w;
}

void SieveActionAbstractFlags::setParamWidgetValue( const QDomElement &element, QWidget *parent )
{

}

QString SieveActionAbstractFlags::code(QWidget *w) const
{
    const SelectFlagsWidget *flagsWidget = w->findChild<SelectFlagsWidget*>( QLatin1String("flagswidget") );
    const QString flagCode = flagsWidget->code();
    const QString str = flagsCode();
    return str + QLatin1Char(' ') + flagCode;
}

QStringList SieveActionAbstractFlags::needRequires(QWidget *) const
{
    return QStringList() << QLatin1String("imapflags");
}

bool SieveActionAbstractFlags::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionAbstractFlags::serverNeedsCapability() const
{
    return QLatin1String("imapflags");
}




#include "sieveactionabstractflags.moc"
