/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "filteractionremoveheader.h"

#include "PimCommon/MinimumComboBox"

#include <KLocalizedString>
#include <QLineEdit>

using namespace MailCommon;

FilterAction *FilterActionRemoveHeader::newAction()
{
    return new FilterActionRemoveHeader;
}

FilterActionRemoveHeader::FilterActionRemoveHeader(QObject *parent)
    : FilterActionWithStringList(QStringLiteral("remove header"), i18n("Remove Header"), parent)
{
    mParameterList << QStringLiteral("")
                   << QStringLiteral("Reply-To")
                   << QStringLiteral("Delivered-To")
                   << QStringLiteral("X-KDE-PR-Message")
                   << QStringLiteral("X-KDE-PR-Package")
                   << QStringLiteral("X-KDE-PR-Keywords");

    mParameter = mParameterList.at(0);
}

QWidget *FilterActionRemoveHeader::createParamWidget(QWidget *parent) const
{
    PimCommon::MinimumComboBox *comboBox = new PimCommon::MinimumComboBox(parent);
    comboBox->setEditable(true);
    comboBox->setInsertPolicy(QComboBox::InsertAtBottom);
    setParamWidgetValue(comboBox);

    connect(comboBox, static_cast<void (PimCommon::MinimumComboBox::*)(int)>(&PimCommon::MinimumComboBox::currentIndexChanged), this, &FilterActionRemoveHeader::filterActionModified);
    connect(comboBox->lineEdit(), &QLineEdit::textChanged,
            this, &FilterAction::filterActionModified);

    return comboBox;
}

FilterAction::ReturnCode FilterActionRemoveHeader::process(ItemContext &context , bool) const
{
    if (isEmpty()) {
        return ErrorButGoOn;
    }

    KMime::Message::Ptr msg = context.item().payload<KMime::Message::Ptr>();
    const QByteArray param(mParameter.toLatin1());
    bool headerRemove = false;
    while (msg->headerByType(param)) {
        headerRemove = true;
        msg->removeHeader(param);
    }
    if (headerRemove) {
        msg->assemble();
        context.setNeedsPayloadStore();
    }

    return GoOn;
}

SearchRule::RequiredPart FilterActionRemoveHeader::requiredPart() const
{
    return SearchRule::CompleteMessage;
}

void FilterActionRemoveHeader::setParamWidgetValue(QWidget *paramWidget) const
{
    PimCommon::MinimumComboBox *comboBox = dynamic_cast<PimCommon::MinimumComboBox *>(paramWidget);
    Q_ASSERT(comboBox);

    const int index = mParameterList.indexOf(mParameter);
    comboBox->clear();
    comboBox->addItems(mParameterList);
    if (index < 0) {
        comboBox->addItem(mParameter);
        comboBox->setCurrentIndex(comboBox->count() - 1);
    } else {
        comboBox->setCurrentIndex(index);
    }
}

QStringList FilterActionRemoveHeader::sieveRequires() const
{
    return QStringList() << QStringLiteral("editheader");
}

QString FilterActionRemoveHeader::sieveCode() const
{
    return QStringLiteral("deleteheader \"%1\";").arg(mParameter);
}

QString FilterActionRemoveHeader::informationAboutNotValidAction() const
{
    return i18n("Header name undefined.");
}

