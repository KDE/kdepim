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


#include "sieveactionextracttext.h"

#include <KLocale>
#include <KLineEdit>

#include <QLabel>
#include <QHBoxLayout>

using namespace KSieveUi;
SieveActionExtractText::SieveActionExtractText(QObject *parent)
    : SieveAction(QLatin1String("extracttext"), i18n("Extra Text"), parent)
{
}

SieveAction* SieveActionExtractText::newAction()
{
  return new SieveActionExtractText;
}

QWidget *SieveActionExtractText::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);
    //TODO
    return w;
}

QString SieveActionExtractText::code(QWidget *w) const
{
    QString result = QLatin1String("extracttext ");
    //TODO
    return result;
}


QStringList SieveActionExtractText::needRequires() const
{
    return QStringList() << QLatin1String("extracttext");
}

bool SieveActionExtractText::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionExtractText::serverNeedsCapability() const
{
    return QLatin1String("extracttext");
}


#include "sieveactionextracttext.moc"
