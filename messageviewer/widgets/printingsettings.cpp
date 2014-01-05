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


#include "printingsettings.h"
#include "ui_printingsettings.h"
#include "settings/globalsettings.h"
#include "pimcommon/widgets/configureimmutablewidgetutils.h"
using namespace PimCommon::ConfigureImmutableWidgetUtils;


using namespace MessageViewer;

PrintingSettings::PrintingSettings(QWidget *parent)
    : QWidget( parent ), mPrintingUi( new Ui_PrintingSettings )
{
    mPrintingUi->setupUi( this );
    connect(mPrintingUi->mPrintEmptySelectedText, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(mPrintingUi->respectExpandCollapseSettings, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(mPrintingUi->printBackgroundColorAndImages, SIGNAL(toggled(bool)), SIGNAL(changed()));
}

PrintingSettings::~PrintingSettings()
{
    delete mPrintingUi;
    mPrintingUi = 0;
}

void PrintingSettings::save()
{
    saveCheckBox(mPrintingUi->mPrintEmptySelectedText, MessageViewer::GlobalSettings::self()->printSelectedTextItem());
    saveCheckBox(mPrintingUi->respectExpandCollapseSettings, MessageViewer::GlobalSettings::self()->respectExpandCollapseSettingsItem());
    saveCheckBox(mPrintingUi->printBackgroundColorAndImages, MessageViewer::GlobalSettings::self()->printBackgroundColorImagesItem());
}

void PrintingSettings::doLoadFromGlobalSettings()
{
    loadWidget(mPrintingUi->mPrintEmptySelectedText, MessageViewer::GlobalSettings::self()->printSelectedTextItem());
    loadWidget(mPrintingUi->respectExpandCollapseSettings, MessageViewer::GlobalSettings::self()->respectExpandCollapseSettingsItem());
    loadWidget(mPrintingUi->printBackgroundColorAndImages, MessageViewer::GlobalSettings::self()->printBackgroundColorImagesItem());
}

void PrintingSettings::doResetToDefaultsOther()
{
    const bool bUseDefaults = GlobalSettings::self()->useDefaults( true );
    loadWidget(mPrintingUi->mPrintEmptySelectedText, MessageViewer::GlobalSettings::self()->printSelectedTextItem());
    loadWidget(mPrintingUi->respectExpandCollapseSettings, MessageViewer::GlobalSettings::self()->respectExpandCollapseSettingsItem());
    loadWidget(mPrintingUi->printBackgroundColorAndImages, MessageViewer::GlobalSettings::self()->printBackgroundColorImagesItem());
    GlobalSettings::self()->useDefaults( bUseDefaults );
}

