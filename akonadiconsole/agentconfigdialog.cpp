/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#include "agentconfigdialog.h"
#include "agentconfigmodel.h"
#include <KLocalizedString>
#include <KIcon>

AgentConfigDialog::AgentConfigDialog(QWidget* parent) :
  KDialog(parent),
  m_model( new AgentConfigModel( this ) )
{
  ui.setupUi( mainWidget() );
  ui.propertyView->setModel( m_model );

  setButtons( User1 | User2 | Apply | Close );
  setButtonGuiItem( User1, KGuiItem( i18n( "Save Configuration" ), KIcon( "document-save" ) ) );
  setButtonGuiItem( User2, KGuiItem( i18n( "Refresh" ), KIcon( "view-refresh" ) ) );
  setButtonText( Apply, i18n( "Apply Configuration" ) );

  setCaption( i18n( "Agent Configuration" ) );

  connect( this, SIGNAL(applyClicked()), SLOT(reconfigure()) );
  connect( this, SIGNAL(user1Clicked()), m_model, SLOT(writeConfig()) );
  connect( this, SIGNAL(user2Clicked()), m_model, SLOT(reload()) );
}

void AgentConfigDialog::setAgentInstance(const Akonadi::AgentInstance& instance)
{
  m_instance = instance;
  m_model->setAgentInstance( instance );
}

void AgentConfigDialog::reconfigure()
{
  m_instance.reconfigure();
}

