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

#include "customheadersettingwidget.h"
#include "pimcommon/widgets/simplestringlisteditor.h"
#include "settings/globalsettings.h"

#include <KConfig>
#include <KConfigGroup>

#include <KLocale>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QButtonGroup>

namespace MessageViewer {

CustomHeaderSettingWidget::CustomHeaderSettingWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *topLayout = new QVBoxLayout;
    topLayout->setContentsMargins( 0, 0, 0, 0 );
    mCbHeaderToShow = new QRadioButton(i18n("Only show the headers listed below"));
    topLayout->addWidget(mCbHeaderToShow);

    mCbHeaderToHide = new QRadioButton(i18n("Show all but hide the headers listed below"));
    topLayout->addWidget(mCbHeaderToHide);
    mHeaders = new PimCommon::SimpleStringListEditor( this, PimCommon::SimpleStringListEditor::All,
                                                           i18n("A&dd..."), i18n("Remo&ve"),
                                                           i18n("&Modify..."), i18n("Header:") );
    connect(mHeaders, SIGNAL(changed()), this, SIGNAL(changed()));
    topLayout->addWidget(mHeaders);

    mHeaderGroup = new QButtonGroup(this);
    mHeaderGroup->addButton(mCbHeaderToHide, MessageViewer::GlobalSettings::EnumCustomHeadersDefaultPolicy::Hide);
    mHeaderGroup->addButton(mCbHeaderToShow, MessageViewer::GlobalSettings::EnumCustomHeadersDefaultPolicy::Display);
    connect( mHeaderGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotHeaderClicked(int)) );

    setLayout(topLayout);
}

CustomHeaderSettingWidget::~CustomHeaderSettingWidget()
{
}

void CustomHeaderSettingWidget::slotHeaderClicked(int button)
{
    switch(button) {
    case MessageViewer::GlobalSettings::EnumCustomHeadersDefaultPolicy::Hide:
        mHeadersToDisplay = mHeaders->stringList();
        mHeaders->setStringList(mHeadersToHide);
        break;
    case MessageViewer::GlobalSettings::EnumCustomHeadersDefaultPolicy::Display:
        mHeadersToHide = mHeaders->stringList();
        mHeaders->setStringList(mHeadersToDisplay);
        break;
    }
    Q_EMIT changed();
}

void CustomHeaderSettingWidget::readConfig()
{
    mHeadersToDisplay = MessageViewer::GlobalSettings::self()->headersToDisplay();
    mHeadersToHide = MessageViewer::GlobalSettings::self()->headersToHide();

    switch(MessageViewer::GlobalSettings::self()->customHeadersDefaultPolicy()) {
    case MessageViewer::GlobalSettings::EnumCustomHeadersDefaultPolicy::Hide:
        mHeaders->setStringList(mHeadersToHide);
        mCbHeaderToHide->setChecked(true);
        break;
    case MessageViewer::GlobalSettings::EnumCustomHeadersDefaultPolicy::Display:
        mHeaders->setStringList(mHeadersToDisplay);
        mCbHeaderToShow->setChecked(true);
        break;
    }
}

void CustomHeaderSettingWidget::writeConfig()
{
    switch(mHeaderGroup->checkedId()) {
    case MessageViewer::GlobalSettings::EnumCustomHeadersDefaultPolicy::Hide:
        MessageViewer::GlobalSettings::self()->setHeadersToDisplay(mHeadersToDisplay);
        MessageViewer::GlobalSettings::self()->setHeadersToHide(mHeaders->stringList());
        break;
    case MessageViewer::GlobalSettings::EnumCustomHeadersDefaultPolicy::Display:
        MessageViewer::GlobalSettings::self()->setHeadersToDisplay(mHeaders->stringList());
        MessageViewer::GlobalSettings::self()->setHeadersToHide(mHeadersToHide);
        break;
    }

    MessageViewer::GlobalSettings::self()->setCustomHeadersDefaultPolicy(mHeaderGroup->checkedId());
}

void CustomHeaderSettingWidget::resetToDefault()
{
    const bool bUseDefaults = MessageViewer::GlobalSettings::self()->useDefaults( true );
    readConfig();

    MessageViewer::GlobalSettings::self()->useDefaults( bUseDefaults );
}

}

#include "customheadersettingwidget.moc"
