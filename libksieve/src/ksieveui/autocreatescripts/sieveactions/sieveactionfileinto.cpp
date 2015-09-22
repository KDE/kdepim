/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "sieveactionfileinto.h"
#include "editor/sieveeditorutil.h"
#include "autocreatescripts/sieveeditorgraphicalmodewidget.h"
#include <KLocalizedString>
#include <QLineEdit>

#include <QCheckBox>
#include <QHBoxLayout>
#include "libksieve_debug.h"
#include <QDomNode>
//Add support for adding flags
using namespace KSieveUi;
SieveActionFileInto::SieveActionFileInto(QObject *parent)
    : SieveAction(QStringLiteral("fileinto"), i18n("File Into"), parent)
{
    mHasCopySupport = SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QStringLiteral("copy"));
    mHasMailBoxSupport = SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QStringLiteral("mailbox"));
}

SieveAction *SieveActionFileInto::newAction()
{
    return new SieveActionFileInto;
}

QString SieveActionFileInto::code(QWidget *w) const
{
    QString result = QStringLiteral("fileinto ");
    const QLineEdit *edit = w->findChild<QLineEdit *>(QStringLiteral("fileintolineedit"));
    const QString text = edit->text();
    if (mHasCopySupport) {
        const QCheckBox *copy = w->findChild<QCheckBox *>(QStringLiteral("copy"));
        if (copy->isChecked()) {
            result += QLatin1String(":copy ");
        }
    }
    if (mHasMailBoxSupport) {
        const QCheckBox *create = w->findChild<QCheckBox *>(QStringLiteral("create"));
        if (create->isChecked()) {
            result += QLatin1String(":create ");
        }
    }
    return result + QStringLiteral("\"%1\";").arg(text);
}

bool SieveActionFileInto::setParamWidgetValue(const QDomElement &element, QWidget *w, QString &error)
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("tag")) {
                const QString tagValue = e.text();
                if (tagValue == QLatin1String("copy")) {
                    if (mHasCopySupport) {
                        QCheckBox *copy = w->findChild<QCheckBox *>(QStringLiteral("copy"));
                        copy->setChecked(true);
                    } else {
                        error += i18n("Action \"fileinto\" has \"copy\" argument but current server does not support it") + QLatin1Char('\n');
                        qCDebug(LIBKSIEVE_LOG) << "SieveActionFileInto::setParamWidgetValue has not copy support ";
                    }
                } else if (tagValue == QLatin1String("create")) {
                    if (mHasMailBoxSupport) {
                        QCheckBox *create = w->findChild<QCheckBox *>(QStringLiteral("create"));
                        create->setChecked(true);
                    } else {
                        serverDoesNotSupportFeatures(QStringLiteral("fileinto"), error);
                        qCDebug(LIBKSIEVE_LOG) << "SieveActionFileInto::setParamWidgetValue server has not create support ";
                    }
                }
            } else if (tagName == QLatin1String("str")) {
                const QString tagValue = e.text();
                QLineEdit *edit = w->findChild<QLineEdit *>(QStringLiteral("fileintolineedit"));
                edit->setText(tagValue);
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qCDebug(LIBKSIEVE_LOG) << " SieveActionFileInto::setParamWidgetValue unknown tagName " << tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QWidget *SieveActionFileInto::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    if (mHasCopySupport) {
        QCheckBox *copy = new QCheckBox(i18n("Keep a copy"));
        copy->setObjectName(QStringLiteral("copy"));
        lay->addWidget(copy);
        connect(copy, &QCheckBox::clicked, this, &SieveActionFileInto::valueChanged);
    }
    if (mHasMailBoxSupport) {
        QCheckBox *create = new QCheckBox(i18n("Create folder"));
        create->setObjectName(QStringLiteral("create"));
        connect(create, &QCheckBox::clicked, this, &SieveActionFileInto::valueChanged);
        lay->addWidget(create);
    }

    //TODO improve it.
    //Use widgets/selectfileintowidget
    QLineEdit *edit = new QLineEdit;
    connect(edit, &QLineEdit::textChanged, this, &SieveActionFileInto::valueChanged);
    lay->addWidget(edit);
    edit->setObjectName(QStringLiteral("fileintolineedit"));
    return w;
}

QStringList SieveActionFileInto::needRequires(QWidget *parent) const
{
    QStringList lst;
    lst << QStringLiteral("fileinto");
    if (mHasCopySupport) {
        const QCheckBox *copy = parent->findChild<QCheckBox *>(QStringLiteral("copy"));
        if (copy->isChecked()) {
            lst << QStringLiteral("copy");
        }
    }
    if (mHasMailBoxSupport) {
        const QCheckBox *create = parent->findChild<QCheckBox *>(QStringLiteral("create"));
        if (create->isChecked()) {
            lst << QStringLiteral("mailbox");
        }
    }
    return lst;
}

bool SieveActionFileInto::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionFileInto::serverNeedsCapability() const
{
    return QStringLiteral("fileinto");
}

QString SieveActionFileInto::help() const
{
    QString helpStr = i18n("The \"fileinto\" action delivers the message into the specified mailbox.");
    if (mHasMailBoxSupport) {
        helpStr += QLatin1Char('\n') + i18n("If the optional \":create\" argument is specified, it instructs the Sieve interpreter to create the specified mailbox, if needed, before attempting to deliver the message into the specified mailbox.");
    }
    if (mHasCopySupport) {
        helpStr += QLatin1Char('\n') + i18n("If the optional \":copy\" keyword is specified, the tagged command does not cancel the implicit \"keep\". Instead, it merely files or redirects a copy in addition to whatever else is happening to the message.");
    }
    return helpStr;
}

QUrl KSieveUi::SieveActionFileInto::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}
