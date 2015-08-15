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

#include "sieveactionabstractflags.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "autocreatescripts/sieveeditorgraphicalmodewidget.h"
#include "widgets/selectflagswidget.h"

#include <QHBoxLayout>
#include <QDomNode>
#include "libksieve_debug.h"

using namespace KSieveUi;
SieveActionAbstractFlags::SieveActionAbstractFlags(const QString &name, const QString &label, QObject *parent)
    : SieveAction(name, label, parent)
{
}

QWidget *SieveActionAbstractFlags::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);
    SelectFlagsWidget *flagsWidget = new SelectFlagsWidget;
    connect(flagsWidget, &SelectFlagsWidget::valueChanged, this, &SieveActionAbstractFlags::valueChanged);
    flagsWidget->setObjectName(QStringLiteral("flagswidget"));
    lay->addWidget(flagsWidget);
    return w;
}

bool SieveActionAbstractFlags::setParamWidgetValue(const QDomElement &element, QWidget *w, QString &error)
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("list")) {
                SelectFlagsWidget *flagsWidget = w->findChild<SelectFlagsWidget *>(QStringLiteral("flagswidget"));
                flagsWidget->setFlags(AutoCreateScriptUtil::listValue(e));
            } else if (tagName == QLatin1String("str")) {
                SelectFlagsWidget *flagsWidget = w->findChild<SelectFlagsWidget *>(QStringLiteral("flagswidget"));
                flagsWidget->setFlags(QStringList() << e.text());
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qCDebug(LIBKSIEVE_LOG) << " SieveActionAbstractFlags::setParamWidgetValue unknown tag :" << tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString SieveActionAbstractFlags::code(QWidget *w) const
{
    const SelectFlagsWidget *flagsWidget = w->findChild<SelectFlagsWidget *>(QStringLiteral("flagswidget"));
    const QString flagCode = flagsWidget->code();
    const QString str = flagsCode();
    return str + QLatin1Char(' ') + (flagCode.isEmpty() ? QStringLiteral(";") : flagCode);
}

QStringList SieveActionAbstractFlags::needRequires(QWidget *) const
{
    if (SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QStringLiteral("imap4flags"))) {
        return QStringList() << QStringLiteral("imap4flags");
    } else {
        return QStringList() << QStringLiteral("imapflags");
    }
}

bool SieveActionAbstractFlags::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionAbstractFlags::serverNeedsCapability() const
{
    if (SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QStringLiteral("imap4flags"))) {
        return QStringLiteral("imap4flags");
    } else {
        return QStringLiteral("imapflags");
    }
}

