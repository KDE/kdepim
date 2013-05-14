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


#include "sieveactionenclose.h"

#include <KLocale>
#include <KLineEdit>

#include <QCheckBox>
#include <QHBoxLayout>

using namespace KSieveUi;
SieveActionEnclose::SieveActionEnclose(QObject *parent)
    : SieveAction(QLatin1String("enclose"), i18n("Enclose"), parent)
{
}

SieveAction* SieveActionEnclose::newAction()
{
  return new SieveActionEnclose;
}

QString SieveActionEnclose::code(QWidget *w) const
{
    //TODO
    return QString();
}

QWidget *SieveActionEnclose::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);
    //TODO
    return w;
}

QStringList SieveActionEnclose::needRequires() const
{
    return QStringList() << QLatin1String("enclose");
}

bool SieveActionEnclose::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionEnclose::serverNeedsCapability() const
{
    return QLatin1String("enclose");
}


#include "sieveactionenclose.moc"
