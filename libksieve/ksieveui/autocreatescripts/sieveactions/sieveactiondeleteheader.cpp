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

#include "sieveactiondeleteheader.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "autocreatescripts/commonwidgets/selectmatchtypecombobox.h"
#include "editor/sieveeditorutil.h"

#include <KLocalizedString>
#include <QLineEdit>

#include <QWidget>
#include <QLabel>
#include <QDomNode>
#include "libksieve_debug.h"
#include <QGridLayout>

using namespace KSieveUi;

SieveActionDeleteHeader::SieveActionDeleteHeader(QObject *parent)
    : SieveActionAbstractEditHeader(QLatin1String("deleteheader"), i18n("Delete header"), parent)
{
}

SieveAction *SieveActionDeleteHeader::newAction()
{
    return new SieveActionDeleteHeader;
}

QWidget *SieveActionDeleteHeader::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);
    w->setLayout(grid);

    SelectMatchTypeComboBox *matchType = new SelectMatchTypeComboBox;
    matchType->setObjectName(QLatin1String("matchtype"));
    connect(matchType, &SelectMatchTypeComboBox::valueChanged, this, &SieveActionDeleteHeader::valueChanged);
    grid->addWidget(matchType, 0, 0);

    QLabel *lab = new QLabel(i18n("header:"));
    grid->addWidget(lab, 0, 1);

    QLineEdit *headerEdit = new QLineEdit;
    headerEdit->setObjectName(QLatin1String("headeredit"));
    connect(headerEdit, &QLineEdit::textChanged, this, &SieveActionDeleteHeader::valueChanged);
    grid->addWidget(headerEdit, 0, 2);

    lab = new QLabel(i18n("value:"));
    grid->addWidget(lab, 1, 1);

    QLineEdit *valueEdit = new QLineEdit;
    valueEdit->setObjectName(QLatin1String("valueedit"));
    connect(valueEdit, &QLineEdit::textChanged, this, &SieveActionDeleteHeader::valueChanged);
    grid->addWidget(valueEdit, 1, 2);
    return w;
}

bool SieveActionDeleteHeader::setParamWidgetValue(const QDomElement &element, QWidget *w, QString &error)
{
    int index = 0;
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("test")) {
                QDomNode testNode = e.toElement();
                return setParamWidgetValue(testNode.toElement(), w, error);
            } else if (tagName == QLatin1String("tag")) {
                SelectMatchTypeComboBox *combo = w->findChild<SelectMatchTypeComboBox *>(QLatin1String("matchtype"));
                combo->setCode(AutoCreateScriptUtil::tagValue(e.text()), name(), error);
            } else if (tagName == QLatin1String("str")) {
                if (index == 0) {
                    QLineEdit *edit = w->findChild<QLineEdit *>(QLatin1String("headeredit"));
                    edit->setText(e.text());
                } else if (index == 1) {
                    QLineEdit *value = w->findChild<QLineEdit *>(QLatin1String("valueedit"));
                    value->setText(e.text());
                } else {
                    tooManyArgument(tagName, index, 2, error);
                    qCDebug(LIBKSIEVE_LOG) << " SieveActionAddHeader::setParamWidgetValue too many argument :" << index;
                }
                ++index;
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qCDebug(LIBKSIEVE_LOG) << "SieveActionAddHeader::setParamWidgetValue unknown tag " << tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString SieveActionDeleteHeader::code(QWidget *w) const
{
    const SelectMatchTypeComboBox *combo = w->findChild<SelectMatchTypeComboBox *>(QLatin1String("matchtype"));
    bool isNegative = false;
    const QString matchTypeStr = combo->code(isNegative);

    const QLineEdit *edit = w->findChild<QLineEdit *>(QLatin1String("headeredit"));
    const QString headerStr = edit->text();

    const QLineEdit *value = w->findChild<QLineEdit *>(QLatin1String("valueedit"));
    const QString valueStr = value->text();

    return QStringLiteral("deleteheader %1 \"%2\" \"%3\";").arg((isNegative ? QLatin1String("not ") + matchTypeStr : matchTypeStr)).arg(headerStr).arg(valueStr);
}

QString SieveActionDeleteHeader::help() const
{
    return i18n("By default, the deleteheader action deletes all occurrences of the named header field.");
}

QString SieveActionDeleteHeader::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}

