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

#include "sieveactionaddheader.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "widgets/selectaddheaderpositioncombobox.h"

#include <KLocalizedString>
#include <KLineEdit>

#include <QWidget>
#include <QLabel>
#include <QDomNode>
#include <QDebug>
#include <QGridLayout>

using namespace KSieveUi;

SieveActionAddHeader::SieveActionAddHeader(QObject *parent)
    : SieveActionAbstractEditHeader(QLatin1String("addheader"), i18n("Add header"), parent)
{
}

SieveAction* SieveActionAddHeader::newAction()
{
    return new SieveActionAddHeader;
}

QWidget *SieveActionAddHeader::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);
    w->setLayout(grid);

    SelectAddHeaderPositionCombobox *combo = new SelectAddHeaderPositionCombobox;
    combo->setObjectName(QLatin1String("selectposition"));
    connect(combo, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()));
    grid->addWidget(combo, 0, 0);

    QLabel *lab = new QLabel(i18n("header:"));
    grid->addWidget(lab, 0, 1);

    KLineEdit *headerEdit = new KLineEdit;
    connect(headerEdit, SIGNAL(textChanged(QString)), this, SIGNAL(valueChanged()));
    headerEdit->setObjectName(QLatin1String("headeredit"));
    grid->addWidget(headerEdit, 0, 2);

    lab = new QLabel(i18n("value:"));
    grid->addWidget(lab, 1, 1);

    KLineEdit *valueEdit = new KLineEdit;
    connect(valueEdit, SIGNAL(textChanged(QString)), this, SIGNAL(valueChanged()));
    valueEdit->setObjectName(QLatin1String("valueedit"));
    grid->addWidget(valueEdit, 1, 2);

    return w;
}

bool SieveActionAddHeader::setParamWidgetValue(const QDomElement &element, QWidget *w , QString &error)
{
    int index = 0;
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("tag")) {
                SelectAddHeaderPositionCombobox *combo = w->findChild<SelectAddHeaderPositionCombobox*>(QLatin1String("selectposition"));
                combo->setCode(AutoCreateScriptUtil::tagValue(e.text()), name(), error);
            } else if (tagName == QLatin1String("str")) {
                if (index == 0) {
                    KLineEdit *edit = w->findChild<KLineEdit*>( QLatin1String("headeredit") );
                    edit->setText(e.text());
                } else if (index == 1) {
                    KLineEdit *value = w->findChild<KLineEdit*>( QLatin1String("valueedit") );
                    value->setText(AutoCreateScriptUtil::quoteStr(e.text()));
                } else {
                    tooManyArgument(tagName, index, 2, error);
                    qDebug()<<" SieveActionAddHeader::setParamWidgetValue too many argument :"<<index;
                }
                ++index;
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qDebug()<<"SieveActionAddHeader::setParamWidgetValue unknown tag "<<tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString SieveActionAddHeader::code(QWidget *w) const
{
    const SelectAddHeaderPositionCombobox *combo = w->findChild<SelectAddHeaderPositionCombobox*>(QLatin1String("selectposition"));
    const QString position = combo->code();

    const KLineEdit *edit = w->findChild<KLineEdit*>( QLatin1String("headeredit") );
    const QString headerStr = edit->text();

    const KLineEdit *value = w->findChild<KLineEdit*>( QLatin1String("valueedit") );
    const QString valueStr = value->text();

    return QString::fromLatin1("addheader %1 \"%2\" \"%3\";").arg(position).arg(headerStr).arg(valueStr);
}

QString SieveActionAddHeader::help() const
{
    return i18n("The addheader action adds a header field to the existing message header.");
}

QString SieveActionAddHeader::href() const
{
    return QLatin1String("http://tools.ietf.org/html/rfc5293");
}

#include "moc_sieveactionaddheader.cpp"
