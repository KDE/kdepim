/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "sieveactionereject.h"
#include "editor/sieveeditorutil.h"
#include "widgets/multilineedit.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

#include <KLocalizedString>

#include <QLabel>
#include <QHBoxLayout>
#include <QDomNode>
#include "libksieve_debug.h"

using namespace KSieveUi;
SieveActionEReject::SieveActionEReject(QObject *parent)
    : SieveAction(QStringLiteral("ereject"), i18n("E-Reject"), parent)
{
}

SieveAction *SieveActionEReject::newAction()
{
    return new SieveActionEReject;
}

QWidget *SieveActionEReject::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);
    QLabel *lab = new QLabel(i18n("text:"));
    lay->addWidget(lab);

    MultiLineEdit *edit = new MultiLineEdit;
    connect(edit, &MultiLineEdit::textChanged, this, &SieveActionEReject::valueChanged);
    edit->setObjectName(QStringLiteral("rejectmessage"));
    lay->addWidget(edit);
    return w;
}

bool SieveActionEReject::setParamWidgetValue(const QDomElement &element, QWidget *w, QString &error)
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("str")) {
                const QString tagValue = e.text();
                MultiLineEdit *edit = w->findChild<MultiLineEdit *>(QStringLiteral("rejectmessage"));
                edit->setPlainText(AutoCreateScriptUtil::quoteStr(tagValue));
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qCDebug(LIBKSIEVE_LOG) << " SieveActionEReject::setParamWidgetValue unknown tagName " << tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString SieveActionEReject::code(QWidget *w) const
{
    const MultiLineEdit *edit = w->findChild<MultiLineEdit *>(QStringLiteral("rejectmessage"));
    const QString text = edit->toPlainText();

    return QStringLiteral("ereject text:%1").arg(AutoCreateScriptUtil::createMultiLine(text));
}

QStringList SieveActionEReject::needRequires(QWidget *) const
{
    return QStringList() << QStringLiteral("ereject");
}

QString SieveActionEReject::serverNeedsCapability() const
{
    return QStringLiteral("ereject");
}

bool SieveActionEReject::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionEReject::help() const
{
    return i18n(" The \"ereject\" action cancels the implicit keep and refuses delivery of a message.");
}

QUrl SieveActionEReject::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}
