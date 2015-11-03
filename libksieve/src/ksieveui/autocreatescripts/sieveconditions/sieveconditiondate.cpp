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

#include "sieveconditiondate.h"
#include "autocreatescripts/commonwidgets/selectmatchtypecombobox.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "widgets/selectdatewidget.h"
#include "editor/sieveeditorutil.h"

#include <KLocalizedString>
#include <QLineEdit>

#include <QHBoxLayout>
#include <QLabel>
#include "libksieve_debug.h"

using namespace KSieveUi;

SieveConditionDate::SieveConditionDate(QObject *parent)
    : SieveCondition(QStringLiteral("date"), i18n("Date"), parent)
{
}

SieveCondition *SieveConditionDate::newAction()
{
    return new SieveConditionDate;
}

QWidget *SieveConditionDate::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    SelectMatchTypeComboBox *matchTypeCombo = new SelectMatchTypeComboBox;
    matchTypeCombo->setObjectName(QStringLiteral("matchtype"));
    connect(matchTypeCombo, &SelectMatchTypeComboBox::valueChanged, this, &SieveConditionDate::valueChanged);
    lay->addWidget(matchTypeCombo);

    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);
    lay->addLayout(grid);
    QLabel *lab = new QLabel(i18n("header:"));
    grid->addWidget(lab, 0, 0);

    QLineEdit *header = new QLineEdit;
    connect(header, &QLineEdit::textChanged, this, &SieveConditionDate::valueChanged);
    header->setObjectName(QStringLiteral("header"));
    grid->addWidget(header, 0, 1);

    SelectDateWidget *dateWidget = new SelectDateWidget;
    connect(dateWidget, &SelectDateWidget::valueChanged, this, &SieveConditionDate::valueChanged);
    dateWidget->setObjectName(QStringLiteral("datewidget"));
    grid->addWidget(dateWidget, 1, 0, 1, 2);

    return w;
}

QString SieveConditionDate::code(QWidget *w) const
{
    const SelectMatchTypeComboBox *selectMatchCombobox = w->findChild<SelectMatchTypeComboBox *>(QStringLiteral("matchtype"));
    bool isNegative = false;
    const QString matchTypeStr = selectMatchCombobox->code(isNegative);

    const QLineEdit *header = w->findChild<QLineEdit *>(QStringLiteral("header"));
    const QString headerStr = header->text();

    const SelectDateWidget *dateWidget = w->findChild<SelectDateWidget *>(QStringLiteral("datewidget"));
    const QString dateWidgetStr = dateWidget->code();

    return AutoCreateScriptUtil::negativeString(isNegative) + QStringLiteral("date %1 \"%2\" %3").arg(matchTypeStr).arg(headerStr).arg(dateWidgetStr);
}

bool SieveConditionDate::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionDate::serverNeedsCapability() const
{
    return QStringLiteral("date");
}

QStringList SieveConditionDate::needRequires(QWidget *) const
{
    return QStringList() << QStringLiteral("date");
}

QString SieveConditionDate::help() const
{
    return i18n("The date test matches date/time information derived from headers containing date-time values.");
}

bool SieveConditionDate::setParamWidgetValue(const QDomElement &element, QWidget *w, bool notCondition, QString &error)
{
    int index = 0;
    QString type;
    QString value;
    QString headerStr;
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("str")) {
                if (index == 0) {
                    headerStr   = e.text();
                } else if (index == 1) {
                    type = e.text();
                } else if (index == 2) {
                    value = e.text();
                } else {
                    tooManyArgument(tagName, index, 3, error);
                    qCDebug(LIBKSIEVE_LOG) << " SieveConditionDate::setParamWidgetValue too many argument :" << index;
                }
                ++index;
            } else if (tagName == QLatin1String("tag")) {
                SelectMatchTypeComboBox *selectMatchCombobox = w->findChild<SelectMatchTypeComboBox *>(QStringLiteral("matchtype"));
                selectMatchCombobox->setCode(AutoCreateScriptUtil::tagValueWithCondition(e.text(), notCondition), name(), error);
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qCDebug(LIBKSIEVE_LOG) << "SieveConditionDate::setParamWidgetValue unknown tag " << tagName;
            }
        }
        node = node.nextSibling();
    }
    SelectDateWidget *dateWidget = w->findChild<SelectDateWidget *>(QStringLiteral("datewidget"));
    dateWidget->setCode(type, value);
    QLineEdit *header = w->findChild<QLineEdit *>(QStringLiteral("header"));
    header->setText(headerStr);
    return true;
}

QUrl SieveConditionDate::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}

