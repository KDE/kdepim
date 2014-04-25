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
#include <AkonadiCore/control.h>
#include <AkonadiCore/searchcreatejob.h>
#include <AkonadiCore/servermanager.h>

#include <KIcon>
#include <QAction>
#include <KActionCollection>
#include <KCMultiDialog>
#include <KTabWidget>
#include <KXmlGuiWindow>

#include <QVBoxLayout>

MainWidget::MainWidget( KXmlGuiWindow *parent )
  : QWidget( parent )
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  KTabWidget *tabWidget = new KTabWidget( this );
  tabWidget->setObjectName( "mainTab" );
  layout->addWidget( tabWidget );

  tabWidget->addTab( new AgentWidget( tabWidget ), "Agents" );
  mBrowser = new BrowserWidget( parent, tabWidget );
  tabWidget->addTab( mBrowser, "Browser" );
  tabWidget->addTab( new DebugWidget( tabWidget ), "Debugger" );
  tabWidget->addTab( new RawSocketConsole( tabWidget ), "Raw Socket" );
  tabWidget->addTab( new DbBrowser( tabWidget ), "DB Browser" );
  tabWidget->addTab( new DbConsole( tabWidget ), "DB Console" );
  tabWidget->addTab( new QueryDebugger( tabWidget ), "Query Debugger" );
  tabWidget->addTab( new JobTrackerWidget( "jobtracker", tabWidget, "Enable job tracker" ), "Job Tracker" );
  tabWidget->addTab( new JobTrackerWidget( "resourcesJobtracker", tabWidget, "Enable tracking of Resource Schedulers" ), "Resources Schedulers" );
  tabWidget->addTab( new NotificationMonitor( tabWidget ), "Notification Monitor" );
  tabWidget->addTab( new SearchWidget( tabWidget ), "Item Search" );
  tabWidget->addTab( new MonitorsWidget( tabWidget ), "Monitors" );

  QAction *action = parent->actionCollection()->addAction( "akonadiconsole_search" );
  action->setText( "Create Search..." );
  connect( action, SIGNAL(triggered()), this, SLOT(createSearch()) );

  action = parent->actionCollection()->addAction( "akonadiconsole_akonadi2xml" );
  action->setText( "Dump to XML..." );
  connect( action, SIGNAL(triggered()), mBrowser, SLOT(dumpToXml()) );

  action = parent->actionCollection()->addAction( "akonadiconsole_clearcache" );
  action->setText( "Clear Akonadi Cache" );
  connect( action, SIGNAL(triggered()), mBrowser, SLOT(clearCache()) );

  action = parent->actionCollection()->addAction( "akonadiserver_start" );
  action->setText( "Start Server" );
  connect( action, SIGNAL(triggered()), SLOT(startServer()) );

  action = parent->actionCollection()->addAction( "akonadiserver_stop" );
  action->setText( "Stop Server" );
  connect( action, SIGNAL(triggered()), SLOT(stopServer()) );

  action = parent->actionCollection()->addAction( "akonadiserver_restart" );
  action->setText( "Restart Server" );
  connect( action, SIGNAL(triggered()), SLOT(restartServer()) );

  action = parent->actionCollection()->addAction( "akonadiserver_configure" );
  action->setText( "Configure Server..." );
  action->setIcon( KIcon("configure") );
  connect( action, SIGNAL(triggered()), SLOT(configureServer()) );
}

MainWidget::~MainWidget()
{
  delete mBrowser;
}

void MainWidget::createSearch()
{
  SearchDialog dlg;
  if ( !dlg.exec() )
    return;

  const QString query = dlg.searchQuery();
  if ( query.isEmpty() )
    return;

  QString name = dlg.searchName();
  if ( name.isEmpty() )
    name = "My Search";

  new Akonadi::SearchCreateJob( name, query );
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
  Akonadi::Control::restart( this );
}

void MainWidget::configureServer()
{
  KCMultiDialog dlg;
  dlg.addModule( "kcm_akonadi_server" );
  dlg.exec();
}


