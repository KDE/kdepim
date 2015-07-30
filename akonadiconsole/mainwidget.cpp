/*
    This file is part of Akonadi.

    Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "mainwidget.h"

#include "agentwidget.h"
#include "dbbrowser.h"
#include "dbconsole.h"
#include "debugwidget.h"
#include "rawsocketconsole.h"
#include "searchdialog.h"
#include "searchwidget.h"
#include "jobtrackerwidget.h"
#include "notificationmonitor.h"
#include "monitorswidget.h"
#include "querydebugger.h"

#include <AkonadiWidgets/agentinstancewidget.h>
#include <AkonadiCore/agentfilterproxymodel.h>
#include <AkonadiWidgets/controlgui.h>
#include <AkonadiCore/searchcreatejob.h>
#include <AkonadiCore/servermanager.h>

#include <QIcon>
#include <QAction>
#include <KActionCollection>
#include <QTabWidget>
#include <KXmlGuiWindow>

#include <QVBoxLayout>

MainWidget::MainWidget(KXmlGuiWindow *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QTabWidget *tabWidget = new QTabWidget(this);
    tabWidget->setObjectName(QStringLiteral("mainTab"));
    layout->addWidget(tabWidget);

    tabWidget->addTab(new AgentWidget(tabWidget), QStringLiteral("Agents"));
    mBrowser = new BrowserWidget(parent, tabWidget);
    tabWidget->addTab(mBrowser, QStringLiteral("Browser"));
    tabWidget->addTab(new DebugWidget(tabWidget), QStringLiteral("Debugger"));
    tabWidget->addTab(new RawSocketConsole(tabWidget), QStringLiteral("Raw Socket"));
    tabWidget->addTab(new DbBrowser(tabWidget), QStringLiteral("DB Browser"));
    tabWidget->addTab(new DbConsole(tabWidget), QStringLiteral("DB Console"));
    tabWidget->addTab(new QueryDebugger(tabWidget), QStringLiteral("Query Debugger"));
    tabWidget->addTab(new JobTrackerWidget("jobtracker", tabWidget, QStringLiteral("Enable job tracker")), QStringLiteral("Job Tracker"));
    tabWidget->addTab(new JobTrackerWidget("resourcesJobtracker", tabWidget, QStringLiteral("Enable tracking of Resource Schedulers")), QStringLiteral("Resources Schedulers"));
    tabWidget->addTab(new NotificationMonitor(tabWidget), QStringLiteral("Notification Monitor"));
    tabWidget->addTab(new SearchWidget(tabWidget), QStringLiteral("Item Search"));
    tabWidget->addTab(new MonitorsWidget(tabWidget), QStringLiteral("Monitors"));

    QAction *action = parent->actionCollection()->addAction(QStringLiteral("akonadiconsole_search"));
    action->setText(QStringLiteral("Create Search..."));
    connect(action, &QAction::triggered, this, &MainWidget::createSearch);

    action = parent->actionCollection()->addAction(QStringLiteral("akonadiconsole_akonadi2xml"));
    action->setText(QStringLiteral("Dump to XML..."));
    connect(action, &QAction::triggered, mBrowser, &BrowserWidget::dumpToXml);

    action = parent->actionCollection()->addAction(QStringLiteral("akonadiconsole_clearcache"));
    action->setText(QStringLiteral("Clear Akonadi Cache"));
    connect(action, &QAction::triggered, mBrowser, &BrowserWidget::clearCache);

    action = parent->actionCollection()->addAction(QStringLiteral("akonadiserver_start"));
    action->setText(QStringLiteral("Start Server"));
    connect(action, &QAction::triggered, this, &MainWidget::startServer);

    action = parent->actionCollection()->addAction(QStringLiteral("akonadiserver_stop"));
    action->setText(QStringLiteral("Stop Server"));
    connect(action, &QAction::triggered, this, &MainWidget::stopServer);

    action = parent->actionCollection()->addAction(QStringLiteral("akonadiserver_restart"));
    action->setText(QStringLiteral("Restart Server"));
    connect(action, &QAction::triggered, this, &MainWidget::restartServer);
}

MainWidget::~MainWidget()
{
    delete mBrowser;
}

void MainWidget::createSearch()
{
//QT5 TODO need to reimplement it
#pragma message("port to QT5")
#if 0
    SearchDialog dlg;
    if (!dlg.exec()) {
        return;
    }

    const QString query = dlg.searchQuery();
    if (query.isEmpty()) {
        return;
    }

    QString name = dlg.searchName();
    if (name.isEmpty()) {
        name = "My Search";
    }

    new Akonadi::SearchCreateJob(name, query);
#endif
}

void MainWidget::startServer()
{
    Akonadi::ServerManager::start();
}

void MainWidget::stopServer()
{
    Akonadi::ServerManager::stop();
}

void MainWidget::restartServer()
{
    Akonadi::ControlGui::restart(this);
}

