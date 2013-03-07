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
#include "globalsettings.h"

#include <KConfig>
#include <KConfigGroup>

#include <KLocale>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRadioButton>

namespace MessageViewer {

CustomHeaderSettingWidget::CustomHeaderSettingWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *topLayout = new QVBoxLayout;
    QGridLayout *grid = new QGridLayout;
    mCbHeaderToShow = new QRadioButton(i18n("Header to show:"));
    grid->addWidget(mCbHeaderToShow, 0, 0);

    PimCommon::SimpleStringListEditor::ButtonCode buttonCode =
      static_cast<PimCommon::SimpleStringListEditor::ButtonCode>( PimCommon::SimpleStringListEditor::Add | PimCommon::SimpleStringListEditor::Remove );

    mHeaderToShow = new PimCommon::SimpleStringListEditor( this, PimCommon::SimpleStringListEditor::All,
                                                           i18n("A&dd..."), i18n("Remo&ve"),
                                                           i18n("&Modify..."), i18n("Header to show:") );
    connect(mHeaderToShow, SIGNAL(changed()), this, SIGNAL(changed()));
    grid->addWidget(mHeaderToShow, 1, 0);

    mCbHeaderToHide = new QRadioButton(i18n("Header to hide:"));
    grid->addWidget(mCbHeaderToHide, 0, 1);
    mHeaderToHide = new PimCommon::SimpleStringListEditor( this, buttonCode,
                                                           i18n("A&dd..."), i18n("Remo&ve"),
                                                           i18n("&Modify..."), i18n("Header to hide:") );
    connect(mHeaderToHide, SIGNAL(changed()), this, SIGNAL(changed()));
    grid->addWidget(mHeaderToHide, 1, 1);
    topLayout->addLayout(grid);
    setLayout(topLayout);
}

CustomHeaderSettingWidget::~CustomHeaderSettingWidget()
{
}

void CustomHeaderSettingWidget::readConfig()
{
    mHeadersToDisplay = MessageViewer::GlobalSettings::self()->headersToDisplay();
    QStringList::iterator end( mHeadersToDisplay.end() );
    for ( QStringList::iterator it = mHeadersToDisplay.begin() ; it != end ; ++ it )
        *it = (*it).toLower();

    mHeaderToShow->setStringList(mHeadersToDisplay);

    mHeadersToHide = MessageViewer::GlobalSettings::self()->headersToHide();

    end = mHeadersToHide.end();
    for ( QStringList::iterator it = mHeadersToHide.begin() ; it != end; ++ it )
        *it = (*it).toLower();
    mHeaderToHide->setStringList(mHeadersToHide);
    //TODO display
}

void CustomHeaderSettingWidget::writeConfig()
{
    MessageViewer::GlobalSettings::self()->setHeadersToDisplay(mHeaderToShow->stringList());
    MessageViewer::GlobalSettings::self()->setHeadersToHide(mHeaderToHide->stringList());

    //Update customheaderstyle
}

void CustomHeaderSettingWidget::resetToDefault()
{
    MessageViewer::GlobalSettings::self()->setHeadersToDisplay(mHeaderToShow->stringList());
}

}
