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

#include "sieveactionkeep.h"
#include "editor/sieveeditorutil.h"
#include "widgets/selectflagswidget.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "autocreatescripts/sieveeditorgraphicalmodewidget.h"


#include <KLocalizedString>
#include <QLabel>
#include <QHBoxLayout>
#include <QDomNode>
#include <QDebug>

using namespace KSieveUi;
SieveActionKeep::SieveActionKeep(QObject *parent)
    : SieveAction(QLatin1String("keep"), i18n("Keep"), parent)
{
    mHasImapFlag4Support = SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QLatin1String("imap4flags"));
    mHasFlagSupport = SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QLatin1String("imapflags")) || mHasImapFlag4Support;
}

SieveAction* SieveActionKeep::newAction()
{
    return new SieveActionKeep;
}

QString SieveActionKeep::code(QWidget *w) const
{
    if (mHasFlagSupport) {
        const SelectFlagsWidget *flagsWidget = w->findChild<SelectFlagsWidget*>( QLatin1String("flagswidget") );
        const QString flagCode = flagsWidget->code();
        if (flagCode.isEmpty()) {
            return QLatin1String("keep;");
        } else {
            return QLatin1String("keep :flags") + QLatin1Char(' ') + flagCode;
        }
    } else {
        return QLatin1String("keep;");
    }
}

QString SieveActionKeep::help() const
{
    return i18n("The \"keep\" action is whatever action is taken in lieu of all other actions, if no filtering happens at all; generally, this simply means to file the message into the user's main mailbox.");
}

QWidget *SieveActionKeep::createParamWidget( QWidget *parent ) const
{
    if (mHasFlagSupport) {
        QWidget *w = new QWidget(parent);
        QHBoxLayout *lay = new QHBoxLayout;
        lay->setMargin(0);
        w->setLayout(lay);
        QLabel *addFlags = new QLabel(i18n("Add flags:"));
        lay->addWidget(addFlags);

        SelectFlagsWidget *flagsWidget = new SelectFlagsWidget;
        connect(flagsWidget, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()));
        flagsWidget->setObjectName(QLatin1String("flagswidget"));
        lay->addWidget(flagsWidget);
        return w;
    } else {
        return 0;
    }
}

bool SieveActionKeep::setParamWidgetValue( const QDomElement &element, QWidget *w, QString &error )
{
    if (mHasFlagSupport) {
        QDomNode node = element.firstChild();
        while (!node.isNull()) {
            QDomElement e = node.toElement();
            if (!e.isNull()) {
                const QString tagName = e.tagName();
                if (tagName == QLatin1String("list")) {
                    SelectFlagsWidget *flagsWidget = w->findChild<SelectFlagsWidget*>( QLatin1String("flagswidget") );
                    flagsWidget->setFlags(AutoCreateScriptUtil::listValue(e));
                } else if (tagName == QLatin1String("str")) {
                    SelectFlagsWidget *flagsWidget = w->findChild<SelectFlagsWidget*>( QLatin1String("flagswidget") );
                    flagsWidget->setFlags(QStringList()<<e.text());
                } else if (tagName == QLatin1String("tag") && e.text() == QLatin1String("flags")) {
                    //nothing :)
                } else if (tagName == QLatin1String("crlf")) {
                    //nothing
                } else if (tagName == QLatin1String("comment")) {
                    //implement in the future ?
                } else {
                    unknownTag(tagName, error);
                    qDebug()<<" SieveActionAbstractFlags::setParamWidgetValue unknown tag :"<<tagName;
                }
            }
            node = node.nextSibling();
        }
    } else {
        qDebug()<<" Server doesn't support imapflags";
    }
    return true;
}

QStringList SieveActionKeep::needRequires(QWidget *) const
{
    QStringList requiresLst;
    if (mHasImapFlag4Support)
        requiresLst << QLatin1String("imap4flags");
    else if (mHasFlagSupport)
        requiresLst << QLatin1String("imapflags");
    return requiresLst;
}

QString SieveActionKeep::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}

