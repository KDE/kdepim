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

#include "sieveactionreject.h"
#include "editor/sieveeditorutil.h"
#include "widgets/multilineedit.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

#include <KLocalizedString>

#include <QLabel>
#include <QHBoxLayout>
#include <QDomNode>
#include "libksieve_debug.h"

using namespace KSieveUi;
SieveActionReject::SieveActionReject(QObject *parent)
    : SieveAction(QStringLiteral("reject"), i18n("Reject"), parent)
{
}

SieveAction *SieveActionReject::newAction()
{
    return new SieveActionReject;
}

QWidget *SieveActionReject::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);
    QLabel *lab = new QLabel(i18n("text:"));
    lay->addWidget(lab);

    MultiLineEdit *edit = new MultiLineEdit;
    connect(edit, &MultiLineEdit::textChanged, this, &SieveActionReject::valueChanged);
    edit->setObjectName(QStringLiteral("rejectmessage"));
    lay->addWidget(edit);
    return w;
}

bool SieveActionReject::setParamWidgetValue(const QDomElement &element, QWidget *w , QString &error)
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QStringLiteral("str")) {
                const QString tagValue = e.text();
                MultiLineEdit *edit = w->findChild<MultiLineEdit *>(QStringLiteral("rejectmessage"));
                edit->setText(AutoCreateScriptUtil::quoteStr(tagValue));
            } else if (tagName == QStringLiteral("crlf")) {
                //nothing
            } else if (tagName == QStringLiteral("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qCDebug(LIBKSIEVE_LOG) << " SieveActionReject::setParamWidgetValue unknown tagName " << tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString SieveActionReject::code(QWidget *w) const
{
    const MultiLineEdit *edit = w->findChild<MultiLineEdit *>(QStringLiteral("rejectmessage"));
    const QString text = edit->toPlainText();

    return QStringLiteral("reject text:%1").arg(AutoCreateScriptUtil::createMultiLine(text));
}

QStringList SieveActionReject::needRequires(QWidget *) const
{
    return QStringList() << QStringLiteral("reject");
}

QString SieveActionReject::serverNeedsCapability() const
{
    return QStringLiteral("reject");
}

bool SieveActionReject::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionReject::help() const
{
    return i18n(" The \"reject\" action cancels the implicit keep and refuses delivery of a message.");
}

QString SieveActionReject::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}
