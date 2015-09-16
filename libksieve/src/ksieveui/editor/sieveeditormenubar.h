/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef SIEVEEDITORMENUBAR_H
#define SIEVEEDITORMENUBAR_H

#include <QMenuBar>

class QAction;
class QMenu;
namespace KSieveUi
{
class SieveEditorMenuBar : public QMenuBar
{
    Q_OBJECT
public:
    explicit SieveEditorMenuBar(QWidget *parent = Q_NULLPTR);
    ~SieveEditorMenuBar();

    QAction *goToLineAction() const;
    QAction *findAction() const;
    QAction *replaceAction() const;
    QAction *undoAction() const;
    QAction *redoAction() const;
    QAction *copyAction() const;
    QAction *pasteAction() const;
    QAction *cutAction() const;
    QAction *selectAllAction() const;

    QMenu *editorMenu() const;

    QMenu *toolsMenu() const;

    QMenu *fileMenu() const;

    QAction *commentCodeAction() const;

    QAction *uncommentCodeAction() const;

    QAction *zoomInAction() const;
    QAction *zoomOutAction() const;
    QAction *debugSieveScriptAction() const;

    QAction *zoomResetAction() const;
    QAction *wordWrapAction() const;
public Q_SLOTS:
    void setEditorMode(bool editorMode);
    void slotUndoAvailable(bool b);
    void slotRedoAvailable(bool b);
    void slotCopyAvailable(bool b);
Q_SIGNALS:
    void gotoLine();
    void find();
    void replace();
    void undo();
    void redo();
    void copy();
    void paste();
    void cut();
    void selectAll();
    void comment();
    void uncomment();
    void zoomIn();
    void zoomOut();
    void debugSieveScript();
    void zoomReset();
    void wordWrap(bool state);

private:
    void initActions();
    void initMenus();
    QAction *mGoToLine;
    QAction *mFindAction;
    QAction *mReplaceAction;
    QAction *mUndoAction;
    QAction *mRedoAction;
    QAction *mCopyAction;
    QAction *mPasteAction;
    QAction *mCutAction;
    QAction *mSelectAllAction;
    QAction *mCommentCodeAction;
    QAction *mUncommentCodeAction;
    QAction *mZoomInAction;
    QAction *mZoomOutAction;
    QAction *mZoomResetAction;
    QAction *mDebugSieveAction;
    QAction *mWordWrapAction;
    QMenu *mEditorMenu;
    QMenu *mToolsMenu;
    QMenu *mFileMenu;
};
}

#endif // SIEVEEDITORMENUBAR_H
