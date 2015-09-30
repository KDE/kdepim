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

#include "viewerplugincreatenotetest.h"
#include "../createnoteplugin/viewerplugincreatenote.h"
#include <KActionCollection>
#include <QHBoxLayout>
#include <QTest>
#include <viewerplugins/viewerplugininterface.h>

ViewerPluginCreateNoteTest::ViewerPluginCreateNoteTest(QObject *parent)
    : QObject(parent)
{

}

ViewerPluginCreateNoteTest::~ViewerPluginCreateNoteTest()
{

}

void ViewerPluginCreateNoteTest::shouldHaveDefaultValue()
{
    MessageViewer::ViewerPluginCreatenote *note = new MessageViewer::ViewerPluginCreatenote(this);
    QVERIFY(!note->viewerPluginName().isEmpty());
    QWidget *parent = new QWidget(0);
    parent->setLayout(new QHBoxLayout);

    QVERIFY(note->createView(parent, new KActionCollection(this)));
}

void ViewerPluginCreateNoteTest::shouldCreateAction()
{
    MessageViewer::ViewerPluginCreatenote *event = new MessageViewer::ViewerPluginCreatenote(this);
    QWidget *parent = new QWidget(0);
    parent->setLayout(new QHBoxLayout);
    MessageViewer::ViewerPluginInterface *interface = event->createView(parent, new KActionCollection(this));
    QVERIFY(interface->action());
}


QTEST_MAIN(ViewerPluginCreateNoteTest)
