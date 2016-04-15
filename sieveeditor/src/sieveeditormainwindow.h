/*
  Copyright (c) 2013-2016 Montel Laurent <montel@kde.org>

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

class QAction;
class SieveEditorCentralWidget;
class QLabel;
class QNetworkConfigurationManager;
class SieveEditorBookmarks;
class KActionMenu;
namespace PimCommon
{
class KActionMenuChangeCase;
}

class SieveEditorMainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    explicit SieveEditorMainWindow();
    ~SieveEditorMainWindow();

    QString currentHelpTitle() const;
    QUrl currentHelpUrl() const;

protected:
    void closeEvent(QCloseEvent *e) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotConfigure();
    void slotAddServerSieve();
    void slotCreateNewScript();
    void slotDeleteScript();
    void slotUpdateButtons(bool newScriptAction, bool editScriptAction, bool deleteScriptAction, bool desactivateScriptAction);
    void slotEditScript();
    void slotDesactivateScript();
    void slotRefreshList();
    void slotUploadScript();
    void slotUpdateActions();
    void slotSystemNetworkOnlineStateChanged(bool state);
    void slotUndoAvailable(bool);
    void slotRedoAvailable(bool);
    void slotCopyAvailable(bool b);
    void slotOpenBookmarkUrl(const QUrl &url);
private:
    void initStatusBar();
    void readConfig();
    void setupActions();
    SieveEditorCentralWidget *mMainWidget;
    QAction *mDeleteScript;
    QAction *mNewScript;
    QAction *mEditScript;
    QAction *mDesactivateScript;
    QAction *mRefreshList;
    QAction *mUploadScript;
    QAction *mGoToLine;
    QAction *mFindAction;
    QAction *mReplaceAction;
    QAction *mUndoAction;
    QAction *mRedoAction;
    QAction *mCopyAction;
    QAction *mPasteAction;
    QAction *mCutAction;
    QAction *mSelectAllAction;
    QAction *mSaveAsAction;
    QAction *mImportAction;
    QAction *mShareAction;
    QAction *mSpellCheckAction;
    QAction *mCheckSyntaxAction;
    QAction *mAutoGenerateScriptAction;
    QAction *mCommentAction;
    QAction *mUncommentAction;
    QAction *mZoomInAction;
    QAction *mZoomOutAction;
    QAction *mZoomResetAction;
    QAction *mDebugSieveScriptAction;
    QAction *mWrapTextAction;
    PimCommon::KActionMenuChangeCase *mMenuChangeCaseAction;

    QLabel *mStatusBarInfo;
    QNetworkConfigurationManager *mNetworkConfigurationManager;
    SieveEditorBookmarks *mSieveEditorBookmarks;
    KActionMenu *mBookmarkMenu;
    bool mNetworkIsDown;
};

#endif // SIEVEEDITORMAINWINDOW_H
