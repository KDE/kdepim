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

#include "sieveactionaddheader.h"
#include "widgets/selectaddheaderpositioncombobox.h"

#include <KLocale>
#include <KLineEdit>

#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>

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
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    SelectAddHeaderPositionCombobox *combo = new SelectAddHeaderPositionCombobox;
    combo->setObjectName(QLatin1String("selectposition"));
    lay->addWidget(combo);

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

void SieveActionAddHeader::setParamWidgetValue(const QDomDocument &doc, QWidget *w ) const
{
    const SelectAddHeaderPositionCombobox *combo = w->findChild<SelectAddHeaderPositionCombobox*>(QLatin1String("selectposition"));
    //combo->setCode();

    const KLineEdit *edit = w->findChild<KLineEdit*>( QLatin1String("headeredit") );
    //edit->setText();

    const KLineEdit *value = w->findChild<KLineEdit*>( QLatin1String("valueedit") );
    //value->setText();
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

#include "sieveactionaddheader.moc"
