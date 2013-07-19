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

#include "sieveconditionmetadataexists.h"

#include <KLocale>
#include <KLineEdit>

#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

using namespace KSieveUi;
SieveConditionMetaDataExists::SieveConditionMetaDataExists(QObject *parent)
    : SieveCondition(QLatin1String("metadataexists"), i18n("Metadata exists"), parent)
{
}

SieveCondition *SieveConditionMetaDataExists::newAction()
{
    return new SieveConditionMetaDataExists;
}

QWidget *SieveConditionMetaDataExists::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    QLabel *lab = new QLabel(i18n("Mailbox:"));
    lay->addWidget(lab);

    KLineEdit *mailbox = new KLineEdit;
    mailbox->setObjectName(QLatin1String("mailbox"));
    lay->addWidget(mailbox);

    lab = new QLabel(i18n("Annotation:"));
    lay->addWidget(lab);

    KLineEdit *value = new KLineEdit;
    value->setObjectName(QLatin1String("value"));
    lay->addWidget(value);

    return w;
}

QString SieveConditionMetaDataExists::code(QWidget *w) const
{
    const KLineEdit *mailbox = w->findChild<KLineEdit*>( QLatin1String("mailbox") );
    const QString mailboxStr = mailbox->text();

    const KLineEdit *value = w->findChild<KLineEdit*>( QLatin1String("value") );
    const QString valueStr = value->text();
    return QString::fromLatin1("metadataexists \"%1\" \"%2\"").arg(mailboxStr).arg(valueStr);
}

QStringList SieveConditionMetaDataExists::needRequires(QWidget *) const
{
    return QStringList() << QLatin1String("mboxmetadata");
}

bool SieveConditionMetaDataExists::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionMetaDataExists::serverNeedsCapability() const
{
    return QLatin1String("mboxmetadata");
}

QString SieveConditionMetaDataExists::help() const
{
    return i18n("The \"metadataexists\" test is true if all of the annotations listed in the \"annotation-names\" argument exist for the specified mailbox.");
}

void SieveConditionMetaDataExists::setParamWidgetValue(const QDomDocument &doc, QWidget *parent ) const
{

}

#include "sieveconditionmetadataexists.moc"
