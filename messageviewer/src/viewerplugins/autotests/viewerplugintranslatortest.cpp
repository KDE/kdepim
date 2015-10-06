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

#include "viewerplugintranslatortest.h"
#include "../translatorplugin/viewerplugintranslator.h"
#include <QTest>
#include <KActionCollection>
#include <QHBoxLayout>
#include <viewerplugins/viewerplugininterface.h>

ViewerPluginTranslatorTest::ViewerPluginTranslatorTest(QObject *parent)
    : QObject(parent)
{

}

ViewerPluginTranslatorTest::~ViewerPluginTranslatorTest()
{

}

void ViewerPluginTranslatorTest::shouldHaveDefaultValue()
{
    MessageViewer::ViewerPluginTranslator *translator = new MessageViewer::ViewerPluginTranslator(this);
    QVERIFY(!translator->viewerPluginName().isEmpty());
    QWidget *parent = new QWidget(0);
    parent->setLayout(new QHBoxLayout);

    QVERIFY(translator->createView(parent, new KActionCollection(this)));

}

void ViewerPluginTranslatorTest::shouldCreateAction()
{
    MessageViewer::ViewerPluginTranslator *translator = new MessageViewer::ViewerPluginTranslator(this);
    QVERIFY(!translator->viewerPluginName().isEmpty());
    QWidget *parent = new QWidget(0);
    parent->setLayout(new QHBoxLayout);

    MessageViewer::ViewerPluginInterface *interface = translator->createView(parent, new KActionCollection(this));
    QVERIFY(interface->action());
}

QTEST_MAIN(ViewerPluginTranslatorTest)
