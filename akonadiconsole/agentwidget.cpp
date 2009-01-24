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

#include "agentwidget.h"

#include <akonadi/agenttypedialog.h>
#include <akonadi/agentinstancewidget.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/control.h>

#include <KLocale>
#include <KMessageBox>
#include <KStandardGuiItem>

#include <QtGui/QGridLayout>
#include <QtGui/QMenu>
#include <QtGui/QPushButton>

using namespace Akonadi;

AgentWidget::AgentWidget( QWidget *parent )
  : QWidget( parent )
{
  ui.setupUi( this );

  connect( ui.instanceWidget, SIGNAL(doubleClicked(Akonadi::AgentInstance)), SLOT(configureAgent()) );
  connect( ui.instanceWidget, SIGNAL(currentChanged(Akonadi::AgentInstance,Akonadi::AgentInstance)),
           SLOT(currentChanged(Akonadi::AgentInstance)) );
  connect( ui.instanceWidget, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)) );
  currentChanged( ui.instanceWidget->currentAgentInstance() );

  ui.addButton->setGuiItem( KStandardGuiItem::add() );
  connect( ui.addButton, SIGNAL( clicked() ), this, SLOT( addAgent() ) );

  ui.removeButton->setGuiItem( KStandardGuiItem::remove() );
  connect( ui.removeButton, SIGNAL( clicked() ), this, SLOT( removeAgent() ) );

  ui.configButton->setGuiItem( KStandardGuiItem::configure() );
  connect( ui.configButton, SIGNAL( clicked() ), this, SLOT( configureAgent() ) );

  mSyncMenu = new QMenu( i18n("Synchronize"), this );
  mSyncMenu->addAction( i18n("Synchronize All"), this, SLOT(synchronizeAgent()) );
  mSyncMenu->addAction( i18n("Synchronize Collection Tree"), this, SLOT(synchronizeTree()) );
  mSyncMenu->setIcon( KIcon("view-refresh" ) );
  ui.syncButton->setMenu( mSyncMenu );
  ui.syncButton->setIcon( KIcon( "view-refresh" ) );
  connect( ui.syncButton, SIGNAL( clicked() ), this, SLOT( synchronizeAgent() ) );

  ui.restartButton->setIcon( KIcon( "system-restart" ) );
  connect( ui.restartButton, SIGNAL(clicked()), SLOT(restartAgent()) );

  Control::widgetNeedsAkonadi( this );
}

void AgentWidget::addAgent()
{
  Akonadi::AgentTypeDialog dlg( this );
  if ( dlg.exec() ) {
    const AgentType agentType = dlg.agentType();

    if ( agentType.isValid() ) {
      AgentInstanceCreateJob *job = new AgentInstanceCreateJob( agentType, this );
      job->configure( this );
      job->start(); // TODO: check result
    }
  }
}

void AgentWidget::removeAgent()
{
  const AgentInstance agent = ui.instanceWidget->currentAgentInstance();
  if ( agent.isValid() ) {
    if ( KMessageBox::questionYesNo( this,
                                     i18n( "Do you really want to delete agent instance %1?", agent.name() ),
                                     i18n( "Agent Deletion" ),
                                     KStandardGuiItem::del(),
                                     KStandardGuiItem::cancel(),
                                     QString(),
                                     KMessageBox::Dangerous )
      == KMessageBox::Yes )
    {
      AgentManager::self()->removeInstance( agent );
    }
  }
}

void AgentWidget::configureAgent()
{
  AgentInstance agent = ui.instanceWidget->currentAgentInstance();
  if ( agent.isValid() )
    agent.configure( this );
}

void AgentWidget::synchronizeAgent()
{
  AgentInstance agent = ui.instanceWidget->currentAgentInstance();
  if ( agent.isValid() )
    agent.synchronize();
}

void AgentWidget::toggleOnline()
{
  AgentInstance agent = ui.instanceWidget->currentAgentInstance();
  if ( agent.isValid() )
    agent.setIsOnline( !agent.isOnline() );
}

void AgentWidget::synchronizeTree()
{
  AgentInstance agent = ui.instanceWidget->currentAgentInstance();
  if ( agent.isValid() )
    agent.synchronizeCollectionTree();
}

void AgentWidget::restartAgent()
{
  AgentInstance agent = ui.instanceWidget->currentAgentInstance();
  if ( agent.isValid() )
    agent.restart();
}

void AgentWidget::currentChanged(const Akonadi::AgentInstance& instance)
{
  ui.removeButton->setEnabled( instance.isValid() );
  ui.configButton->setEnabled( instance.isValid() );
  ui.syncButton->setEnabled( instance.isValid() );
  ui.restartButton->setEnabled( instance.isValid() );

  if ( instance.isValid() ) {
    ui.identifierLabel->setText( instance.identifier() );
    ui.typeLabel->setText( instance.type().name() );
    ui.statusLabel->setText( instance.isOnline() ? i18n( "Online" ) : i18n( "Offline" ) );
    ui.capabilitiesLabel->setText( instance.type().capabilities().join( ", " ) );
    ui.mimeTypeLabel->setText( instance.type().mimeTypes().join( ", " ) );
  } else {
    ui.identifierLabel->setText( QString() );
    ui.typeLabel->setText( QString() );
    ui.statusLabel->setText( QString() );
    ui.capabilitiesLabel->setText( QString() );
    ui.mimeTypeLabel->setText( QString() );
  }
}

void AgentWidget::showContextMenu(const QPoint& pos)
{
  QMenu menu( this );
  menu.addAction( KIcon("list-add"), i18n("Add Agent..."), this, SLOT(addAgent()) );
  menu.addSeparator();
  menu.addMenu( mSyncMenu );
  menu.addAction( KIcon("system-restart"), i18n("Restart Agent"), this, SLOT(restartAgent()) );
  menu.addAction( KIcon("network-disconnect"), i18n("Toggle Online/Offline"), this, SLOT(toggleOnline()) );
  menu.addAction( KIcon("configure"), i18n("Configure..."), this, SLOT(configureAgent()) );
  menu.addAction( KIcon("list-remove"), i18n("Remove Agent"), this, SLOT(removeAgent()) );
  menu.exec( mapToGlobal( pos ) );
}


#include "agentwidget.moc"
