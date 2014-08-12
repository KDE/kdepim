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

#include "sieveconditionmetadataexists.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "editor/sieveeditorutil.h"
#include <KLocalizedString>
#include <QLineEdit>

#include <QLabel>
#include <QDebug>
#include <QDomNode>
#include <QGridLayout>

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
    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);
    w->setLayout(grid);

    QLabel *lab = new QLabel(i18n("Mailbox:"));
    grid->addWidget(lab, 0, 0);

    QLineEdit *mailbox = new QLineEdit;
    connect(mailbox, SIGNAL(textChanged(QString)), this, SIGNAL(valueChanged()));
    mailbox->setObjectName(QLatin1String("mailbox"));
    grid->addWidget(mailbox, 0, 1);

    lab = new QLabel(i18n("Annotation:"));
    grid->addWidget(lab, 1, 0);

    QLineEdit *value = new QLineEdit;
    connect(value, SIGNAL(textChanged(QString)), this, SIGNAL(valueChanged()));
    value->setObjectName(QLatin1String("value"));
    grid->addWidget(value, 1, 1);

    return w;
}

QString SieveConditionMetaDataExists::code(QWidget *w) const
{
    const QLineEdit *mailbox = w->findChild<QLineEdit*>( QLatin1String("mailbox") );
    const QString mailboxStr = mailbox->text();

    const QLineEdit *value = w->findChild<QLineEdit*>( QLatin1String("value") );
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

bool SieveConditionMetaDataExists::setParamWidgetValue(const QDomElement &element, QWidget *w, bool /*notCondition*/, QString &error )
{
    int index = 0;
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("str")) {
                const QString tagValue = e.text();
                if (index == 0) {
                    QLineEdit *mailbox = w->findChild<QLineEdit*>( QLatin1String("mailbox") );
                    mailbox->setText(tagValue);
                } else if (index == 1) {
                    QLineEdit *value = w->findChild<QLineEdit*>( QLatin1String("value") );
                    value->setText(AutoCreateScriptUtil::quoteStr(tagValue));
                } else {
                    tooManyArgument(tagName, index, 2, error);
                    qDebug()<<" SieveConditionServerMetaDataExists::setParamWidgetValue to many attribute "<<index;
                }
                ++index;
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qDebug()<<" SieveConditionServerMetaDataExists::setParamWidgetValue unknown tagName "<<tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString SieveConditionMetaDataExists::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}
