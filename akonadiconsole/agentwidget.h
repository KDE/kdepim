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

#ifndef AKONADICONSOLE_AGENTWIDGET_H
#define AKONADICONSOLE_AGENTWIDGET_H

#include "ui_agentwidget.h"

#include <akonadi/agentinstance.h>

class KJob;
class QMenu;
class QPoint;
class QResizeEvent;

class AgentWidget : public QWidget
{
  Q_OBJECT

  public:
    explicit AgentWidget( QWidget *parent = 0 );
    Akonadi::AgentInstanceWidget *widget() const { return ui.instanceWidget; }

protected:
    void resizeEvent( QResizeEvent *event );

  private Q_SLOTS:
    void addAgent();
    void removeAgent();
    void configureAgent();
    void configureAgentRemote();
    void synchronizeAgent();
    void synchronizeTree();
    void toggleOnline();
    void showChangeNotifications();
    void showTaskList();
    void abortAgent();
    void restartAgent();
    void cloneAgent();
    void cloneAgent( KJob *job );

    void currentChanged();
    void showContextMenu( const QPoint &pos );

    void selectionChanged();
    void slotDataChanged( const QModelIndex&, const QModelIndex& );

  private:
    Ui::AgentWidget ui;
    QMenu *mSyncMenu, *mConfigMenu;
    Akonadi::AgentInstance mCloneSource;
};

#endif
