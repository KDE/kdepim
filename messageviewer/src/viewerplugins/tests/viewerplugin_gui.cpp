/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "viewerplugin_gui.h"
#include "viewerplugins/viewerplugintoolmanager.h"
#include <QStandardPaths>
#include <KActionCollection>
#include <KLocalizedString>

#include <QApplication>
#include <QTextEdit>
#include <KAboutData>
#include <QCommandLineParser>
#include <QVBoxLayout>
#include <QMenuBar>

#include <viewerplugins/viewerplugininterface.h>

ViewerPluginTest::ViewerPluginTest(QWidget *parent)
    : QWidget(parent)
{
    QMenuBar *menuBar = new QMenuBar(this);
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout(vbox);
    vbox->addWidget(menuBar);
    QTextEdit *textEdit = new QTextEdit;
    vbox->addWidget(textEdit);

    QWidget *toolManagerWidget = new QWidget;
    vbox->addWidget(toolManagerWidget);
    vbox = new QVBoxLayout;
    vbox->setMargin(0);
    vbox->setSpacing(0);
    toolManagerWidget->setLayout(vbox);
    MessageViewer::ViewerPluginToolManager *toolManager = new MessageViewer::ViewerPluginToolManager(toolManagerWidget, this);
    connect(toolManager, &MessageViewer::ViewerPluginToolManager::activatePlugin, this, &ViewerPluginTest::slotActivatePlugin);

    toolManager->setActionCollection(new KActionCollection(this));
    toolManager->createView();
    QMenu *menu = new QMenu(this);
    menu->setTitle(QStringLiteral("tools"));
    menu->addActions(toolManager->viewerPluginActionList(MessageViewer::ViewerPluginInterface::All));
    menuBar->addMenu(menu);

    menu = new QMenu(this);
    menu->setTitle(QStringLiteral("selected tools"));
    menu->addActions(toolManager->viewerPluginActionList(MessageViewer::ViewerPluginInterface::NeedSelection));
    menuBar->addMenu(menu);

    menu = new QMenu(this);
    menu->setTitle(QStringLiteral("message tools"));
    menu->addActions(toolManager->viewerPluginActionList(MessageViewer::ViewerPluginInterface::NeedMessage));
    menuBar->addMenu(menu);

    menu = new QMenu(this);
    menu->setTitle(QStringLiteral("message tools and selected tools"));
    MessageViewer::ViewerPluginInterface::SpecificFeatureTypes featureTypes;
    featureTypes |= MessageViewer::ViewerPluginInterface::NeedMessage;
    featureTypes |= MessageViewer::ViewerPluginInterface::NeedSelection;
    menu->addActions(toolManager->viewerPluginActionList(featureTypes));
    menuBar->addMenu(menu);
}

ViewerPluginTest::~ViewerPluginTest()
{

}

void ViewerPluginTest::slotActivatePlugin(MessageViewer::ViewerPluginInterface *interface)
{
    interface->showWidget();
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QStandardPaths::setTestModeEnabled(true);
    KAboutData aboutData(QStringLiteral("viewerplugin_gui"), i18n("viewerplugin_Gui"), QStringLiteral("1.0"));
    aboutData.setShortDescription(i18n("Test for viewerplugin"));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    ViewerPluginTest *w = new ViewerPluginTest();
    w->resize(800, 200);
    w->show();
    app.exec();
    delete w;
    return 0;
}

