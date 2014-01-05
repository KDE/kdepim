/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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


#include "customtoolswidget_gui.h"
#include "pimcommon/widgets/customtoolswidget.h"

#include <kdebug.h>
#include <kapplication.h>
#include <KCmdLineArgs>
#include <KLocalizedString>

#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>

CustomToolWidgetTest::CustomToolWidgetTest(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;

    QHBoxLayout *hbox = new QHBoxLayout;
    lay->addLayout(hbox);
    QLabel *lab = new QLabel(i18n("Switch component:"));
    hbox->addWidget(lab);

    QComboBox *style = new QComboBox;
    hbox->addWidget(style);
    style->addItems(QStringList()<< i18n("Translate") << i18n("Short url"));
    connect(style, SIGNAL(activated(int)), this, SLOT(slotSwitchComponent(int)));

    mCustomTools = new PimCommon::CustomToolsWidget;
    lay->addWidget(mCustomTools);
    setLayout(lay);
}

CustomToolWidgetTest::~CustomToolWidgetTest()
{

}

void CustomToolWidgetTest::slotSwitchComponent(int index)
{
    mCustomTools->switchToTool(static_cast<PimCommon::CustomToolsWidget::ToolType>(index));
}

int main (int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "customtoolswidget_gui", 0, ki18n("CustomToolWidgetsTest_Gui"),
                       "1.0", ki18n("Test for customtoolswidget"));

    KApplication app;

    CustomToolWidgetTest *w = new CustomToolWidgetTest();
    w->resize(800, 200);
    w->show();
    app.exec();
    delete w;
    return 0;
}

