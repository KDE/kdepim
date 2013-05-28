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

#include "sieveconditionservermetadata.h"

#include <KLocale>
#include <KLineEdit>

#include <QWidget>
#include <QHBoxLayout>
#include <QDebug>

//TODO implement it
using namespace KSieveUi;
SieveConditionServerMetaData::SieveConditionServerMetaData(QObject *parent)
    : SieveCondition(QLatin1String("servermetadata"), i18n("Server Meta Data"), parent)
{
}

SieveCondition *SieveConditionServerMetaData::newAction()
{
    return new SieveConditionServerMetaData;
}

QWidget *SieveConditionServerMetaData::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    return w;
}

QString SieveConditionServerMetaData::code(QWidget *w) const
{
    //TODO
    return QString::fromLatin1("servermetadata;");
}

QStringList SieveConditionServerMetaData::needRequires(QWidget *) const
{
    return QStringList() << QLatin1String("servermetadata");
}

bool SieveConditionServerMetaData::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionServerMetaData::serverNeedsCapability() const
{
    return QLatin1String("servermetadata");
}

QString SieveConditionServerMetaData::help() const
{
    return i18n("This test retrieves the value of the server annotation \"annotation-name\".  The retrieved value is compared to the \"key-list\". The test returns true if the annotation exists and its value matches any of the keys.");
}

#include "sieveconditionservermetadata.moc"
