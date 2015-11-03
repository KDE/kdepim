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

#include "sieveactionredirect.h"
#include "editor/sieveeditorutil.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "autocreatescripts/sieveeditorgraphicalmodewidget.h"
#include "widgets/addresslineedit.h"

#include <KLocalizedString>
#include <QLineEdit>

#include <QHBoxLayout>
#include <QCheckBox>
#include <QDomNode>
#include "libksieve_debug.h"

using namespace KSieveUi;

SieveActionRedirect::SieveActionRedirect(QObject *parent)
    : SieveAction(QStringLiteral("redirect"), i18n("Redirect"), parent)
{
    mHasCopySupport = SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QStringLiteral("copy"));
    mHasListSupport = SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QStringLiteral("extlists"));
}

SieveAction *SieveActionRedirect::newAction()
{
    return new SieveActionRedirect;
}

QWidget *SieveActionRedirect::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);
    if (mHasCopySupport) {
        QCheckBox *copy = new QCheckBox(i18n("Keep a copy"));
        copy->setObjectName(QStringLiteral("copy"));
        connect(copy, &QCheckBox::clicked, this, &SieveActionRedirect::valueChanged);
        lay->addWidget(copy);
    }
    if (mHasListSupport) {
        QCheckBox *list = new QCheckBox(i18n("Use list"));
        list->setObjectName(QStringLiteral("list"));
        connect(list, &QCheckBox::clicked, this, &SieveActionRedirect::valueChanged);
        lay->addWidget(list);
    }
    AddressLineEdit *edit = new AddressLineEdit;
    edit->setObjectName(QStringLiteral("RedirectEdit"));
    connect(edit, &AddressLineEdit::valueChanged, this, &SieveActionRedirect::valueChanged);
    lay->addWidget(edit);
    return w;
}

bool SieveActionRedirect::setParamWidgetValue(const QDomElement &element, QWidget *w, QString &error)
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("str")) {
                QLineEdit *edit = w->findChild<AddressLineEdit *>(QStringLiteral("RedirectEdit"));
                const QString tagValue = e.text();
                edit->setText(AutoCreateScriptUtil::quoteStr(tagValue));
            } else if (tagName == QLatin1String("tag")) {
                const QString tagValue = e.text();
                if (tagValue == QLatin1String("copy")) {
                    if (mHasCopySupport) {
                        QCheckBox *copy = w->findChild<QCheckBox *>(QStringLiteral("copy"));
                        copy->setChecked(true);
                    } else {
                        serverDoesNotSupportFeatures(QStringLiteral("copy"), error);
                    }
                } else if (tagValue == QLatin1String("list")) {
                    if (mHasListSupport) {
                        QCheckBox *list = w->findChild<QCheckBox *>(QStringLiteral("list"));
                        list->setChecked(true);
                    } else {
                        serverDoesNotSupportFeatures(QStringLiteral("list"), error);
                    }
                } else {
                    unknowTagValue(tagValue, error);
                    qCDebug(LIBKSIEVE_LOG) << " SieveActionRedirect::setParamWidgetValue tagValue unknown" << tagValue;
                }
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qCDebug(LIBKSIEVE_LOG) << " SieveActionRedirect::setParamWidgetValue unknown tagName " << tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString SieveActionRedirect::code(QWidget *w) const
{
    QString result = QStringLiteral("redirect ");
    const QLineEdit *edit = w->findChild<AddressLineEdit *>(QStringLiteral("RedirectEdit"));
    const QString text = edit->text();

    if (mHasCopySupport) {
        const QCheckBox *copy = w->findChild<QCheckBox *>(QStringLiteral("copy"));
        if (copy->isChecked()) {
            result += QLatin1String(":copy ");
        }
    }

    if (mHasListSupport) {
        const QCheckBox *list = w->findChild<QCheckBox *>(QStringLiteral("list"));
        if (list->isChecked()) {
            result += QLatin1String(":list ");
        }
    }

    return result + QStringLiteral("\"%1\";").arg(text);
}

QStringList SieveActionRedirect::needRequires(QWidget *parent) const
{
    QStringList lst;
    if (mHasCopySupport) {
        const QCheckBox *copy = parent->findChild<QCheckBox *>(QStringLiteral("copy"));
        if (copy->isChecked()) {
            lst << QStringLiteral("copy");
        }
    }
    if (mHasListSupport) {
        const QCheckBox *list = parent->findChild<QCheckBox *>(QStringLiteral("list"));
        if (list->isChecked()) {
            lst << QStringLiteral("extlists");
        }
    }
    return lst;
}

QString SieveActionRedirect::help() const
{
    QString helpStr = i18n("The \"redirect\" action is used to send the message to another user at a supplied address, as a mail forwarding feature does.  The \"redirect\" action makes no changes to the message body or existing headers, but it may add new headers.");
    if (mHasCopySupport) {
        helpStr += QLatin1Char('\n') + i18n("If the optional \":copy\" keyword is specified, the tagged command does not cancel the implicit \"keep\". Instead, it redirects a copy in addition to whatever else is happening to the message.");
    }
    //TODO add list info
    return helpStr;
}

QUrl SieveActionRedirect::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}

