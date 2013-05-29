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

#include "sieveconditionservermetadataexists.h"

#include <KLocale>
#include <KLineEdit>

#include <QHBoxLayout>
#include <QDebug>
#include <QLabel>

using namespace KSieveUi;
SieveConditionServerMetaDataExists::SieveConditionServerMetaDataExists(QObject *parent)
    : SieveCondition(QLatin1String("servermetadataexists"), i18n("Server Meta Data Exists"), parent)
{
}

SieveCondition *SieveConditionServerMetaDataExists::newAction()
{
    return new SieveConditionServerMetaDataExists;
}

QWidget *SieveConditionServerMetaDataExists::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    QLabel *lab = new QLabel(i18n("Annotation:"));
    lay->addWidget(lab);

    KLineEdit *value = new KLineEdit;
    value->setObjectName(QLatin1String("value"));
    lay->addWidget(value);

    return w;
}

QString SieveConditionServerMetaDataExists::code(QWidget *w) const
{
    const KLineEdit *value = w->findChild<KLineEdit*>( QLatin1String("value") );
    const QString valueStr = value->text();
    return QString::fromLatin1("servermetadataexists \"%1\"").arg(valueStr);
}

QStringList SieveConditionServerMetaDataExists::needRequires(QWidget *) const
{
    return QStringList() << QLatin1String("servermetadata");
}

bool SieveConditionServerMetaDataExists::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionServerMetaDataExists::serverNeedsCapability() const
{
    return QLatin1String("servermetadata");
}

QString SieveConditionServerMetaDataExists::help() const
{
    return i18n("The \"servermetadataexists\" test is true if all of the server annotations listed in the \"annotation-names\" argument exist.");
}

#include "sieveconditionservermetadataexists.moc"
