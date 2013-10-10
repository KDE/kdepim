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


#include "printingsettings.h"
#include "ui_printingsettings.h"
#include "settings/globalsettings.h"

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
    MessageViewer::GlobalSettings::self()->setPrintSelectedText(mPrintingUi->mPrintEmptySelectedText->isChecked());
    MessageViewer::GlobalSettings::self()->setRespectExpandCollapseSettings(mPrintingUi->respectExpandCollapseSettings->isChecked());
    MessageViewer::GlobalSettings::self()->setPrintBackgroundColorImages(mPrintingUi->printBackgroundColorAndImages->isChecked());
}

void PrintingSettings::doLoadFromGlobalSettings()
{
    mPrintingUi->mPrintEmptySelectedText->setChecked(MessageViewer::GlobalSettings::self()->printSelectedText());
    mPrintingUi->respectExpandCollapseSettings->setChecked(MessageViewer::GlobalSettings::self()->respectExpandCollapseSettings());
    mPrintingUi->printBackgroundColorAndImages->setChecked(MessageViewer::GlobalSettings::self()->printBackgroundColorImages());
}

void PrintingSettings::doResetToDefaultsOther()
{
    const bool bUseDefaults = GlobalSettings::self()->useDefaults( true );
    mPrintingUi->mPrintEmptySelectedText->setChecked(MessageViewer::GlobalSettings::self()->printSelectedText());
    mPrintingUi->respectExpandCollapseSettings->setChecked(MessageViewer::GlobalSettings::self()->respectExpandCollapseSettings());
    mPrintingUi->printBackgroundColorAndImages->setChecked(MessageViewer::GlobalSettings::self()->printBackgroundColorImages());
    GlobalSettings::self()->useDefaults( bUseDefaults );
}

#include "printingsettings.moc"
