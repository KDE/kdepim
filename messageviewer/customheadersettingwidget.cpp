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
#include "pimcommon/simplestringlisteditor.h"

#include <KConfig>
#include <KConfigGroup>

#include <KLocale>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

namespace MessageViewer {

CustomHeaderSettingWidget::CustomHeaderSettingWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *topLayout = new QVBoxLayout;
    QGridLayout *grid = new QGridLayout;
    QLabel *lab = new QLabel(i18n("Header to show:"));
    grid->addWidget(lab, 0, 0);

    PimCommon::SimpleStringListEditor::ButtonCode buttonCode =
      static_cast<PimCommon::SimpleStringListEditor::ButtonCode>( PimCommon::SimpleStringListEditor::Add | PimCommon::SimpleStringListEditor::Remove );

    mHeaderToShow = new PimCommon::SimpleStringListEditor( this, PimCommon::SimpleStringListEditor::All,
                                                           i18n("A&dd..."), i18n("Remo&ve"),
                                                           i18n("&Modify..."), i18n("Header to show:") );
    grid->addWidget(mHeaderToShow, 1, 0);

    lab = new QLabel(i18n("Header to hide:"));
    grid->addWidget(mHeaderToShow, 0, 1);
    mHeaderToHide = new PimCommon::SimpleStringListEditor( this, buttonCode,
                                                           i18n("A&dd..."), i18n("Remo&ve"),
                                                           i18n("&Modify..."), i18n("Header to hide:") );
    grid->addWidget(mHeaderToHide, 1, 1);
    topLayout->addLayout(grid);
    setLayout(topLayout);
}

CustomHeaderSettingWidget::~CustomHeaderSettingWidget()
{
}

void CustomHeaderSettingWidget::loadConfig()
{

}

void CustomHeaderSettingWidget::writeConfig()
{

}

void CustomHeaderSettingWidget::resetToDefault()
{

}

}
