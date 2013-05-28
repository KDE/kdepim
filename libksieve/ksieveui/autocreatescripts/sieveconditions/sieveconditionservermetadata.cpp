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
#include "autocreatescripts/commonwidgets/selectmatchtypecombobox.h"

#include <KLocale>
#include <KLineEdit>

#include <QWidget>
#include <QHBoxLayout>
#include <QDebug>
#include <QLabel>

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

    SelectMatchTypeComboBox *selectType = new SelectMatchTypeComboBox;
    selectType->setObjectName(QLatin1String("selecttype"));
    lay->addWidget(selectType);

    QLabel *lab = new QLabel(i18n("Mailbox:"));
    lay->addWidget(lab);

    KLineEdit *mailbox = new KLineEdit;
    mailbox->setObjectName(QLatin1String("mailbox"));
    lay->addWidget(mailbox);

    lab = new QLabel(i18n("Annotations:"));
    lay->addWidget(lab);

    KLineEdit *annotation = new KLineEdit;
    annotation->setObjectName(QLatin1String("annotation"));
    lay->addWidget(annotation);

    lab = new QLabel(i18n("Value:"));
    lay->addWidget(lab);

    KLineEdit *value = new KLineEdit;
    value->setObjectName(QLatin1String("value"));
    lay->addWidget(value);

    return w;
}

QString SieveConditionServerMetaData::code(QWidget *w) const
{
    const SelectMatchTypeComboBox *selectType = w->findChild<SelectMatchTypeComboBox*>( QLatin1String("selecttype"));
    bool isNegative = false;
    const QString matchString = selectType->code(isNegative);

    QString result = (isNegative ? QLatin1String("not ") : QString()) + QString::fromLatin1("servermetadata %1 ").arg(matchString);


    const KLineEdit *mailbox = w->findChild<KLineEdit*>( QLatin1String("mailbox"));
    const QString mailboxStr = mailbox->text();

    result += QString::fromLatin1("\"%1\" ").arg(mailboxStr);

    const KLineEdit *annotation = w->findChild<KLineEdit*>( QLatin1String("annotation"));
    const QString annotationStr = annotation->text();

    result += QString::fromLatin1("\"%1\" ").arg(annotationStr);

    const KLineEdit *value = w->findChild<KLineEdit*>( QLatin1String("value"));
    const QString valueStr = value->text();

    result += QString::fromLatin1("\"%1\";").arg(valueStr);
    return result;
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
