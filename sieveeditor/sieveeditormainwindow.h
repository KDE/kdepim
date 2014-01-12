/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#ifndef SIEVEEDITORMAINWINDOW_H
#define SIEVEEDITORMAINWINDOW_H

#include <KXmlGuiWindow>

class KAction;
class SieveEditorMainWidget;
class SieveEditorMainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    explicit SieveEditorMainWindow();
    ~SieveEditorMainWindow();

private slots:
    void slotQuitApp();
    void slotConfigure();
    void slotAddServerSieve();
    void slotCreateNewScript();
    void slotDeleteScript();
    void slotUpdateButtons(bool newScriptAction, bool editScriptAction, bool deleteScriptAction, bool desactivateScriptAction);    
    void slotEditScript();
    void slotDesactivateScript();
    void slotRefreshList();
    void slotSaveScript();

private:
    void readConfig();
    void setupActions();
    SieveEditorMainWidget *mMainWidget;
    KAction *mDeleteScript;
    KAction *mNewScript;
    KAction *mEditScript;
    KAction *mDesactivateScript;
    KAction *mRefreshList;
    KAction *mSaveScript;
};

#endif // SIEVEEDITORMAINWINDOW_H
