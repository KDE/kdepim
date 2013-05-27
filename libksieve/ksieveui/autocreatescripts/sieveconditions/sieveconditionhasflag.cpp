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

#include "sieveconditionhasflag.h"

#include <KLocale>
#include <KLineEdit>

#include <QWidget>
#include <QHBoxLayout>
#include <QDebug>


//TODO need implement

using namespace KSieveUi;
SieveConditionHasFlag::SieveConditionHasFlag(QObject *parent)
    : SieveCondition(QLatin1String("hasflag"), i18n("Has Flag"), parent)
{
}

SieveCondition *SieveConditionHasFlag::newAction()
{
    return new SieveConditionHasFlag;
}

QWidget *SieveConditionHasFlag::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    return w;
}

QString SieveConditionHasFlag::code(QWidget *w) const
{
    //TODO
    return QString::fromLatin1("hasflag");
}

QStringList SieveConditionHasFlag::needRequires(QWidget *) const
{
    return QStringList() << QLatin1String("imap4flags");
}

bool SieveConditionHasFlag::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionHasFlag::serverNeedsCapability() const
{
    return QLatin1String("imap4flags");
}

QString SieveConditionHasFlag::help() const
{
    return i18n("The hasflag test evaluates to true if any of the variables matches any flag name.");
}

#include "sieveconditionhasflag.moc"

