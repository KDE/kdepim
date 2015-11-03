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

#include "sieveactionkeep.h"
#include "editor/sieveeditorutil.h"
#include "widgets/selectflagswidget.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "autocreatescripts/sieveeditorgraphicalmodewidget.h"

#include <KLocalizedString>
#include <QLabel>
#include <QHBoxLayout>
#include <QDomNode>
#include "libksieve_debug.h"

using namespace KSieveUi;
SieveActionKeep::SieveActionKeep(QObject *parent)
    : SieveAction(QStringLiteral("keep"), i18n("Keep"), parent)
{
    mHasImapFlag4Support = SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QStringLiteral("imap4flags"));
    mHasFlagSupport = SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QStringLiteral("imapflags")) || mHasImapFlag4Support;
}

SieveAction *SieveActionKeep::newAction()
{
    return new SieveActionKeep;
}

QString SieveActionKeep::code(QWidget *w) const
{
    if (mHasFlagSupport) {
        const SelectFlagsWidget *flagsWidget = w->findChild<SelectFlagsWidget *>(QStringLiteral("flagswidget"));
        const QString flagCode = flagsWidget->code();
        if (flagCode.isEmpty()) {
            return QStringLiteral("keep;");
        } else {
            return QStringLiteral("keep :flags") + QLatin1Char(' ') + flagCode;
        }
    } else {
        return QStringLiteral("keep;");
    }
}

QString SieveActionKeep::help() const
{
    return i18n("The \"keep\" action is whatever action is taken in lieu of all other actions, if no filtering happens at all; generally, this simply means to file the message into the user's main mailbox.");
}

QWidget *SieveActionKeep::createParamWidget(QWidget *parent) const
{
    if (mHasFlagSupport) {
        QWidget *w = new QWidget(parent);
        QHBoxLayout *lay = new QHBoxLayout;
        lay->setMargin(0);
        w->setLayout(lay);
        QLabel *addFlags = new QLabel(i18n("Add flags:"));
        lay->addWidget(addFlags);

        SelectFlagsWidget *flagsWidget = new SelectFlagsWidget;
        connect(flagsWidget, &SelectFlagsWidget::valueChanged, this, &SieveActionKeep::valueChanged);
        flagsWidget->setObjectName(QStringLiteral("flagswidget"));
        lay->addWidget(flagsWidget);
        return w;
    } else {
        return Q_NULLPTR;
    }
}

bool SieveActionKeep::setParamWidgetValue(const QDomElement &element, QWidget *w, QString &error)
{
    if (mHasFlagSupport) {
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
                } else if (tagName == QLatin1String("tag") && e.text() == QLatin1String("flags")) {
                    //nothing :)
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
    } else {
        qCDebug(LIBKSIEVE_LOG) << " Server doesn't support imapflags";
    }
    return true;
}

QStringList SieveActionKeep::needRequires(QWidget *) const
{
    QStringList requiresLst;
    if (mHasImapFlag4Support) {
        requiresLst << QStringLiteral("imap4flags");
    } else if (mHasFlagSupport) {
        requiresLst << QStringLiteral("imapflags");
    }
    return requiresLst;
}

QUrl SieveActionKeep::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}

