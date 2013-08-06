/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include <KLocale>
#include <KLineEdit>

#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QDomNode>
#include <QDebug>

using namespace KSieveUi;

SieveActionDeleteHeader::SieveActionDeleteHeader(QObject *parent)
    : SieveActionAbstractEditHeader(QLatin1String("deleteheader"), i18n("Delete header"), parent)
{
}

SieveAction* SieveActionDeleteHeader::newAction()
{
    return new SieveActionDeleteHeader;
}

QWidget *SieveActionDeleteHeader::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    SelectMatchTypeComboBox *matchType = new SelectMatchTypeComboBox;
    matchType->setObjectName(QLatin1String("matchtype"));
    lay->addWidget(matchType);

    QLabel *lab = new QLabel(i18n("header:"));
    lay->addWidget(lab);

    KLineEdit *headerEdit = new KLineEdit;
    headerEdit->setObjectName(QLatin1String("headeredit"));
    lay->addWidget(headerEdit);

    lab = new QLabel(i18n("value:"));
    lay->addWidget(lab);

    KLineEdit *valueEdit = new KLineEdit;
    valueEdit->setObjectName(QLatin1String("valueedit"));
    lay->addWidget(valueEdit);
    return w;
}

bool SieveActionDeleteHeader::setParamWidgetValue(const QDomElement &element, QWidget *w, QString &error )
{
    int index = 0;
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("test")) {
                QDomNode testNode = e.toElement();
                //TODO return error here
                setParamWidgetValue(testNode.toElement(), w, error );
                return true;
            } else if (tagName == QLatin1String("tag")) {
                SelectMatchTypeComboBox *combo = w->findChild<SelectMatchTypeComboBox*>( QLatin1String("matchtype") );
                combo->setCode(AutoCreateScriptUtil::tagValue(e.text()));
            } else if (tagName == QLatin1String("str")) {
                if (index == 0) {
                    KLineEdit *edit = w->findChild<KLineEdit*>( QLatin1String("headeredit") );
                    edit->setText(e.text());
                } else if (index == 1) {
                    KLineEdit *value = w->findChild<KLineEdit*>( QLatin1String("valueedit") );
                    value->setText(e.text());
                } else {
                    qDebug()<<" SieveActionAddHeader::setParamWidgetValue too many argument :"<<index;
                }
                ++index;
            } else {
                unknownTag(tagName, error);
                qDebug()<<"SieveActionAddHeader::setParamWidgetValue unknown tag "<<tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString SieveActionDeleteHeader::code(QWidget *w) const
{
    const SelectMatchTypeComboBox *combo = w->findChild<SelectMatchTypeComboBox*>( QLatin1String("matchtype") );
    bool isNegative = false;
    const QString matchTypeStr = combo->code(isNegative);

    const KLineEdit *edit = w->findChild<KLineEdit*>( QLatin1String("headeredit") );
    const QString headerStr = edit->text();

    const KLineEdit *value = w->findChild<KLineEdit*>( QLatin1String("valueedit") );
    const QString valueStr = value->text();

    return QString::fromLatin1("deleteheader %1 \"%2\" \"%3\";").arg((isNegative ? QLatin1String("not ") + matchTypeStr : matchTypeStr )).arg(headerStr).arg(valueStr);
}

QString SieveActionDeleteHeader::help() const
{
    return i18n("By default, the deleteheader action deletes all occurrences of the named header field.");
}

#include "sieveactiondeleteheader.moc"
