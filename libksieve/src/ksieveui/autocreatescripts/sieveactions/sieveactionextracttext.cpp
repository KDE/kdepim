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

#include "sieveactionextracttext.h"
#include "editor/sieveeditorutil.h"

#include <KLocalizedString>
#include <QLineEdit>

#include <QLabel>
#include <QSpinBox>
#include <QDomNode>
#include "libksieve_debug.h"
#include <QGridLayout>

using namespace KSieveUi;
SieveActionExtractText::SieveActionExtractText(QObject *parent)
    : SieveAction(QStringLiteral("extracttext"), i18n("Extract Text"), parent)
{
}

SieveAction *SieveActionExtractText::newAction()
{
    return new SieveActionExtractText;
}

QWidget *SieveActionExtractText::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);
    w->setLayout(grid);

    QLabel *lab = new QLabel(i18n("Number of characters:"));
    grid->addWidget(lab, 0, 0);

    QSpinBox *nbCharacters = new QSpinBox;
    nbCharacters->setMinimum(1);
    nbCharacters->setMaximum(99999);
    nbCharacters->setObjectName(QStringLiteral("numberOfCharacters"));
    grid->addWidget(nbCharacters, 0, 1);
    connect(nbCharacters, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &SieveActionExtractText::valueChanged);

    lab = new QLabel(i18n("Stored in variable name:"));
    grid->addWidget(lab, 1, 0);

    QLineEdit *variableName = new QLineEdit;
    connect(variableName, &QLineEdit::textChanged, this, &SieveActionExtractText::valueChanged);
    variableName->setObjectName(QStringLiteral("variablename"));
    grid->addWidget(variableName, 1, 1);

    return w;
}

bool SieveActionExtractText::setParamWidgetValue(const QDomElement &element, QWidget *w, QString &error)
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("tag")) {
                //TODO ?
            } else if (tagName == QLatin1String("num")) {
                QSpinBox *numberOfCharacters = w->findChild<QSpinBox *>(QStringLiteral("numberOfCharacters"));
                numberOfCharacters->setValue(e.text().toInt());
            } else if (tagName == QLatin1String("str")) {
                QLineEdit *variableName = w->findChild<QLineEdit *>(QStringLiteral("variablename"));
                variableName->setText(e.text());
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qCDebug(LIBKSIEVE_LOG) << " SieveActionExtractText::setParamWidgetValue unknown tagName " << tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString SieveActionExtractText::code(QWidget *w) const
{
    const QSpinBox *numberOfCharacters = w->findChild<QSpinBox *>(QStringLiteral("numberOfCharacters"));
    const QString numberOfCharactersStr = QString::number(numberOfCharacters->value());

    const QLineEdit *variableName = w->findChild<QLineEdit *>(QStringLiteral("variablename"));
    const QString variableNameStr = variableName->text();

    const QString result = QStringLiteral("extracttext :first %1 \"%2\";").arg(numberOfCharactersStr, variableNameStr);
    return result;
}

QStringList SieveActionExtractText::needRequires(QWidget * /*parent*/) const
{
    return QStringList() << QStringLiteral("extracttext");
}

bool SieveActionExtractText::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionExtractText::serverNeedsCapability() const
{
    return QStringLiteral("extracttext");
}

QString SieveActionExtractText::help() const
{
    return i18n("The \"extracttext\" action may be used within the context of a \"foreverypart\" loop and is used to store text into a variable");
}

QUrl SieveActionExtractText::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}

