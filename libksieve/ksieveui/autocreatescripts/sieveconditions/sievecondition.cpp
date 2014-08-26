/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "sievecondition.h"

#include <KLocalizedString>

#include <QWidget>

using namespace KSieveUi;

SieveCondition::SieveCondition(const QString &name, const QString &label, QObject *parent)
    : QObject(parent), mName(name), mLabel(label)
{
}

SieveCondition::~SieveCondition()
{
}

QString SieveCondition::name() const
{
    return mName;
}

QString SieveCondition::label() const
{
    return mLabel;
}

SieveCondition *SieveCondition::newAction()
{
    return 0;
}

QWidget *SieveCondition::createParamWidget(QWidget *parent) const
{
    return new QWidget(parent);
}

QString SieveCondition::code(QWidget *parent) const
{
    return QString();
}

QStringList SieveCondition::needRequires(QWidget *parent) const
{
    return QStringList();
}

bool SieveCondition::needCheckIfServerHasCapability() const
{
    return false;
}

QString SieveCondition::serverNeedsCapability() const
{
    return QString();
}

QString SieveCondition::help() const
{
    return QString();
}

bool SieveCondition::setParamWidgetValue(const QDomElement &/*element*/, QWidget */*parent*/, bool /*notCondition*/, QString &/*error*/)
{
    return true;
}

void SieveCondition::unknownTag(const QString &tag, QString &error)
{
    error += i18n("Unknown tag \"%1\" during parsing condition \"%2\"", tag, name()) + QLatin1Char('\n');
}

void SieveCondition::unknowTagValue(const QString &tagValue, QString &error)
{
    error += i18n("An unknown tag value \"%1\" was found during parsing condition \"%2\".", tagValue, name()) + QLatin1Char('\n');
}

void SieveCondition::tooManyArgument(const QString &tagName, int index, int maxValue, QString &error)
{
    error += i18n("Too many argument found for \"%1\", max value is %2, number of value found %3 for %4", name(), maxValue, index, tagName) + QLatin1Char('\n');
}

void SieveCondition::serverDoesNotSupportFeatures(const QString &feature, QString &error)
{
    error += i18n("A feature \"%1\" in condition \"%2\" is not supported by server", feature, name()) + QLatin1Char('\n');
}

QString KSieveUi::SieveCondition::href() const
{
    return QString();
}
