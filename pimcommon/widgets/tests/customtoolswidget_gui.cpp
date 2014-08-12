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


#include "customtoolswidget_gui.h"
#include "pimcommon/widgets/customtoolswidget.h"

#include <qdebug.h>


#include <KLocalizedString>

#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QApplication>
#include <KAboutData>
#include <QCommandLineParser>

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
    KAboutData aboutData( QLatin1String("customtoolswidget_gui"), i18n("CustomToolWidgetsTest_Gui"), QLatin1String("1.0"));
    aboutData.setShortDescription(i18n("Test for customtoolswidget"));
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    //PORTING SCRIPT: adapt aboutdata variable if necessary
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);


    CustomToolWidgetTest *w = new CustomToolWidgetTest();
    w->resize(800, 200);
    w->show();
    app.exec();
    delete w;
    return 0;
}

