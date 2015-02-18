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
class KAction;
class QMenu;
namespace KSieveUi {
class SieveEditorMenuBar : public QMenuBar
{
    Q_OBJECT
public:
    explicit SieveEditorMenuBar(QWidget *parent = 0);
    ~SieveEditorMenuBar();

    KAction *goToLineAction() const;
    KAction *findAction() const;
    KAction *replaceAction() const;
    KAction *undoAction() const;
    KAction *redoAction() const;
    KAction *copyAction() const;
    KAction *pasteAction() const;
    KAction *cutAction() const;
    KAction *selectAllAction() const;

    QMenu *editorMenu() const;

    QMenu *toolsMenu() const;

public slots:
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

private:
    void initActions();
    void initMenus();
    KAction *mGoToLine;
    KAction *mFindAction;
    KAction *mReplaceAction;
    KAction *mUndoAction;
    KAction *mRedoAction;
    KAction *mCopyAction;
    KAction *mPasteAction;
    KAction *mCutAction;
    KAction *mSelectAllAction;
    QMenu *mEditorMenu;
    QMenu *mToolsMenu;
};
}

#endif // SIEVEEDITORMENUBAR_H
