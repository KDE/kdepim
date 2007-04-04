/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <libqopensync/engine.h>
#include <kdemacros.h>

#include <qwidget.h>

class KAboutData;
class KAction;
class KXMLGUIClient;
class GroupView;
class SyncProcess;
class SyncProcessManager;

namespace QSync {
class Environment;
}

class KDE_EXPORT MainWidget : public QWidget
{
    Q_OBJECT
  public:
    MainWidget( KXMLGUIClient *guiClient, QWidget *widget = 0, const char *name = 0 );
    ~MainWidget();

    virtual KXMLGUIClient *guiClient() const;
    static KAboutData *aboutData();

  public slots:
    void addGroup();
    void deleteGroup();
    void editGroup();
    void editGroup( SyncProcess *syncProcess );

    void sync();
    void sync( SyncProcess *syncProcess );
    void abortSync( SyncProcess *syncProcess );

  private:
    void initGUI();
    void initActions();
    void enableActions();

    KXMLGUIClient *mGUIClient;
    KAction *mActionSynchronize;
    KAction *mActionAddGroup;
    KAction *mActionDeleteGroup;
    KAction *mActionEditGroup;

    GroupView *mGroupView;
};

#endif
