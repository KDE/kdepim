/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef MULTISYNK_MAINWIDGET_H
#define MULTISYNK_MAINWIDGET_H

#include <qwidget.h>

#include <syncer.h>

class KAboutData;
class KAction;
class KActionCollection;
class KonnectorPairManager;
class KonnectorPairView;
class KXMLGUIClient;
class LogDialog;

namespace KSync {
class Engine;
}

class KDE_EXPORT MainWidget : public QWidget
{
  Q_OBJECT

  public:
    MainWidget( KXMLGUIClient *guiClient, QWidget *widget = 0, const char *name = 0 );
    ~MainWidget();

    virtual KActionCollection *actionCollection() const;
    static KAboutData *aboutData();

  private slots:
    void addPair();
    void editPair();
    void deletePair();
    void showLog();
    void startSync();
    void syncDone();
    void konnectorPairSelected( bool );
    void engineError( const QString& );

  private:
    void initGUI();

    KonnectorPairManager *mManager;
    KonnectorPairView *mView;
    KSync::Engine *mEngine;
    LogDialog *mLogDialog;

    KAction *mEditAction;
    KAction *mDeleteAction;
    KAction *mSyncAction;

    KXMLGUIClient *mGUIClient;
};

#endif
