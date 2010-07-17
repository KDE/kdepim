/*
    Copyright (c) 2010 Volker Krause

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

#ifndef AGENTCONFIGDIALOG_H
#define AGENTCONFIGDIALOG_H

#include "ui_agentconfigdialog.h"
#include <Akonadi/AgentInstance>
#include <kdialog.h>

class AgentConfigModel;

class AgentConfigDialog : public KDialog
{
  Q_OBJECT
  public:
    AgentConfigDialog( QWidget *parent = 0 );
    void setAgentInstance( const Akonadi::AgentInstance &instance );

  private slots:
    void reconfigure();

  private:
    Ui::AgentConfigDialog ui;
    AgentConfigModel *m_model;
    Akonadi::AgentInstance m_instance;
};

#endif
