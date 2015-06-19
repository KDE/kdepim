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

#include "filteractionaddtag.h"
#include "filtermanager.h"
#include "dialog/filteractionmissingargumentdialog.h"
#include "pimcommon/widgets/minimumcombobox.h"

#include <QTextDocument>
#include <QPointer>
#include <Tag>

using namespace MailCommon;

FilterAction *FilterActionAddTag::newAction()
{
    return new FilterActionAddTag;
}

FilterActionAddTag::FilterActionAddTag(QObject *parent)
    : FilterAction(QLatin1String("add tag"), i18n("Add Tag"), parent),
      mComboBox(0)
{
    mList = FilterManager::instance()->tagList();
    connect(FilterManager::instance(), SIGNAL(tagListingFinished()), SLOT(slotTagListingFinished()));
}

QWidget *FilterActionAddTag::createParamWidget(QWidget *parent) const
{
    mComboBox = new PimCommon::MinimumComboBox(parent);
    mComboBox->setEditable(false);
    QMapIterator<QUrl, QString> i(mList);
    while (i.hasNext()) {
        i.next();
        mComboBox->addItem(i.value(), i.key());
    }

    setParamWidgetValue(mComboBox);

    connect(mComboBox, static_cast<void (PimCommon::MinimumComboBox::*)(int)>(&PimCommon::MinimumComboBox::currentIndexChanged), this, &FilterActionAddTag::filterActionModified);

    return mComboBox;
}

void FilterActionAddTag::applyParamWidgetValue(QWidget *paramWidget)
{
    PimCommon::MinimumComboBox *combo = static_cast<PimCommon::MinimumComboBox *>(paramWidget);
    mParameter = combo->itemData(combo->currentIndex()).toString();
}

void FilterActionAddTag::setParamWidgetValue(QWidget *paramWidget) const
{
    const int index = static_cast<PimCommon::MinimumComboBox *>(paramWidget)->findData(mParameter);

    static_cast<PimCommon::MinimumComboBox *>(paramWidget)->setCurrentIndex(index < 0 ? 0 : index);
}

void FilterActionAddTag::clearParamWidget(QWidget *paramWidget) const
{
    static_cast<PimCommon::MinimumComboBox *>(paramWidget)->setCurrentIndex(0);
}

bool FilterActionAddTag::isEmpty() const
{
    return (mParameter.isEmpty());
}

void FilterActionAddTag::slotTagListingFinished()
{
    if (mComboBox) {
        mList = FilterManager::instance()->tagList();
        mComboBox->clear();
        fillComboBox();
    }
}

void FilterActionAddTag::fillComboBox()
{
    QMapIterator<QUrl, QString> i(mList);
    while (i.hasNext()) {
        i.next();
        mComboBox->addItem(i.value(), i.key());
    }
}

bool FilterActionAddTag::argsFromStringInteractive(const QString &argsStr, const QString &filterName)
{
    bool needUpdate = false;
    argsFromString(argsStr);
    if (mList.isEmpty()) {
        return needUpdate;
    }
    const bool index = mList.contains(mParameter);
    if (!index) {
        QPointer<FilterActionMissingTagDialog> dlg = new FilterActionMissingTagDialog(mList, filterName, argsStr);
        if (dlg->exec()) {
            mParameter = dlg->selectedTag();
            needUpdate = true;
        }
        delete dlg;
    }
    return needUpdate;
}

FilterAction::ReturnCode FilterActionAddTag::process(ItemContext &context , bool) const
{
    if (!mList.contains(mParameter)) {
        return ErrorButGoOn;
    }
    context.item().setTag(Akonadi::Tag::fromUrl(mParameter));
    context.setNeedsFlagStore();

    return GoOn;
}

SearchRule::RequiredPart FilterActionAddTag::requiredPart() const
{
    return SearchRule::Envelope;
}

void FilterActionAddTag::argsFromString(const QString &argsStr)
{
    if (mList.isEmpty()) {
        mParameter = argsStr;
        return;
    }
    if (mList.contains(argsStr)) {
        mParameter = argsStr;
        return;
    }
    if (!mList.isEmpty()) {
        mParameter = mList.cbegin().value();
    }
}

QString FilterActionAddTag::argsAsString() const
{
    return mParameter;
}

QString FilterActionAddTag::displayString() const
{
    return label() + QLatin1String(" \"") + argsAsString().toHtmlEscaped() + QLatin1String("\"");
}

QString FilterActionAddTag::informationAboutNotValidAction() const
{
    const QString info = name() + QLatin1Char('\n') + i18n("No tag selected.");
    return info;
}

