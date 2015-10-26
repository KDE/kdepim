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

#include "customheadersettingwidget.h"
#include "PimCommon/SimpleStringlistEditor"
#include "settings/messageviewersettings.h"

#include <KConfig>
#include <KConfigGroup>

#include <KLocalizedString>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QButtonGroup>

namespace MessageViewer
{

CustomHeaderSettingWidget::CustomHeaderSettingWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *topLayout = new QVBoxLayout;
    topLayout->setContentsMargins(0, 0, 0, 0);
    mCbHeaderToShow = new QRadioButton(i18n("Only show the headers listed below"));
    topLayout->addWidget(mCbHeaderToShow);

    mCbHeaderToHide = new QRadioButton(i18n("Show all but hide the headers listed below"));
    topLayout->addWidget(mCbHeaderToHide);
    mHeaders = new PimCommon::SimpleStringListEditor(this, PimCommon::SimpleStringListEditor::All,
            i18n("A&dd..."), i18n("Remo&ve"),
            i18n("&Modify..."), i18n("Header:"));
    mHeaders->setUpDownAutoRepeat(true);

    connect(mHeaders, &PimCommon::SimpleStringListEditor::changed, this, &CustomHeaderSettingWidget::changed);
    topLayout->addWidget(mHeaders);

    mHeaderGroup = new QButtonGroup(this);
    mHeaderGroup->addButton(mCbHeaderToHide, MessageViewer::MessageViewerSettings::EnumCustomHeadersDefaultPolicy::Hide);
    mHeaderGroup->addButton(mCbHeaderToShow, MessageViewer::MessageViewerSettings::EnumCustomHeadersDefaultPolicy::Display);
    connect(mHeaderGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &CustomHeaderSettingWidget::slotHeaderClicked);

    setLayout(topLayout);
}

CustomHeaderSettingWidget::~CustomHeaderSettingWidget()
{
}

void CustomHeaderSettingWidget::slotHeaderClicked(int button)
{
    switch (button) {
    case MessageViewer::MessageViewerSettings::EnumCustomHeadersDefaultPolicy::Hide:
        mHeadersToDisplay = mHeaders->stringList();
        mHeaders->setStringList(mHeadersToHide);
        break;
    case MessageViewer::MessageViewerSettings::EnumCustomHeadersDefaultPolicy::Display:
        mHeadersToHide = mHeaders->stringList();
        mHeaders->setStringList(mHeadersToDisplay);
        break;
    }
    Q_EMIT changed();
}

void CustomHeaderSettingWidget::readConfig()
{
    mHeadersToDisplay = MessageViewer::MessageViewerSettings::self()->headersToDisplay();
    mHeadersToHide = MessageViewer::MessageViewerSettings::self()->headersToHide();

    switch (MessageViewer::MessageViewerSettings::self()->customHeadersDefaultPolicy()) {
    case MessageViewer::MessageViewerSettings::EnumCustomHeadersDefaultPolicy::Hide:
        mHeaders->setStringList(mHeadersToHide);
        mCbHeaderToHide->setChecked(true);
        break;
    case MessageViewer::MessageViewerSettings::EnumCustomHeadersDefaultPolicy::Display:
        mHeaders->setStringList(mHeadersToDisplay);
        mCbHeaderToShow->setChecked(true);
        break;
    }
}

void CustomHeaderSettingWidget::writeConfig()
{
    switch (mHeaderGroup->checkedId()) {
    case MessageViewer::MessageViewerSettings::EnumCustomHeadersDefaultPolicy::Hide:
        MessageViewer::MessageViewerSettings::self()->setHeadersToDisplay(mHeadersToDisplay);
        MessageViewer::MessageViewerSettings::self()->setHeadersToHide(mHeaders->stringList());
        break;
    case MessageViewer::MessageViewerSettings::EnumCustomHeadersDefaultPolicy::Display:
        MessageViewer::MessageViewerSettings::self()->setHeadersToDisplay(mHeaders->stringList());
        MessageViewer::MessageViewerSettings::self()->setHeadersToHide(mHeadersToHide);
        break;
    }

    MessageViewer::MessageViewerSettings::self()->setCustomHeadersDefaultPolicy(mHeaderGroup->checkedId());
}

void CustomHeaderSettingWidget::resetToDefault()
{
    const bool bUseDefaults = MessageViewer::MessageViewerSettings::self()->useDefaults(true);
    readConfig();

    MessageViewer::MessageViewerSettings::self()->useDefaults(bUseDefaults);
}

}

