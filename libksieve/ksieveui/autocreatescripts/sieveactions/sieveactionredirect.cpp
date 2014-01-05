/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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
#include <KLineEdit>

#include <QHBoxLayout>
#include <QCheckBox>
#include <QDomNode>
#include <QDebug>

using namespace KSieveUi;

SieveActionRedirect::SieveActionRedirect(QObject *parent)
    : SieveAction(QLatin1String("redirect"), i18n("Redirect"), parent)
{
    mHasCopySupport = SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QLatin1String("copy"));
    mHasListSupport = SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QLatin1String("extlists"));
}

SieveAction *SieveActionRedirect::newAction()
{
    return new SieveActionRedirect;
}

QWidget *SieveActionRedirect::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);
    if (mHasCopySupport) {
        QCheckBox *copy = new QCheckBox(i18n("Keep a copy"));
        copy->setObjectName(QLatin1String("copy"));
        lay->addWidget(copy);
    }
    if (mHasListSupport) {
        QCheckBox *list = new QCheckBox(i18n("Use list"));
        list->setObjectName(QLatin1String("list"));
        lay->addWidget(list);
    }
    AddressLineEdit *edit = new AddressLineEdit;
    edit->setObjectName(QLatin1String("RedirectEdit"));
    lay->addWidget(edit);
    return w;
}

bool SieveActionRedirect::setParamWidgetValue(const QDomElement &element, QWidget *w , QString &error)
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("str")) {
                KLineEdit *edit = w->findChild<AddressLineEdit*>( QLatin1String("RedirectEdit") );
                const QString tagValue = e.text();
                edit->setText(AutoCreateScriptUtil::quoteStr(tagValue));
            } else if (tagName == QLatin1String("tag")) {
                const QString tagValue = e.text();
                if (tagValue == QLatin1String("copy")) {
                    if (mHasCopySupport) {
                        QCheckBox *copy = w->findChild<QCheckBox*>( QLatin1String("copy") );
                        copy->setChecked(true);
                    } else {
                        serverDoesNotSupportFeatures(QLatin1String("copy"), error);
                    }
                } else if (tagValue == QLatin1String("list")) {
                    if (mHasListSupport) {
                        QCheckBox *list = w->findChild<QCheckBox*>( QLatin1String("list") );
                        list->setChecked(true);
                    } else {
                        serverDoesNotSupportFeatures(QLatin1String("list"), error);
                    }
                } else {
                    unknowTagValue(tagValue, error);
                    qDebug()<<" SieveActionRedirect::setParamWidgetValue tagValue unknown"<<tagValue;
                }
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qDebug()<<" SieveActionRedirect::setParamWidgetValue unknown tagName "<<tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString SieveActionRedirect::code(QWidget *w) const
{
    QString result = QLatin1String("redirect ");
    const KLineEdit *edit = w->findChild<AddressLineEdit*>( QLatin1String("RedirectEdit") );
    const QString text = edit->text();

    if (mHasCopySupport) {
        const QCheckBox *copy = w->findChild<QCheckBox*>( QLatin1String("copy") );
        if (copy->isChecked())
            result += QLatin1String(":copy ");
    }

    if (mHasListSupport) {
        const QCheckBox *list = w->findChild<QCheckBox*>( QLatin1String("list") );
        if (list->isChecked())
            result += QLatin1String(":list ");
    }

    return result + QString::fromLatin1("\"%1\";").arg(text);
}

QStringList SieveActionRedirect::needRequires(QWidget *parent) const
{
    QStringList lst;
    if (mHasCopySupport) {
        const QCheckBox *copy = parent->findChild<QCheckBox*>( QLatin1String("copy") );
        if (copy->isChecked())
            lst <<QLatin1String("copy");
    }
    if (mHasListSupport) {
        const QCheckBox *list = parent->findChild<QCheckBox*>( QLatin1String("list") );
        if (list->isChecked())
            lst <<QLatin1String("extlists");
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

QString SieveActionRedirect::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}

