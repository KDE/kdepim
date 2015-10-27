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

#include "sieveeditormenubar.h"
#include "sieveeditortextmodewidget.h"
#include <KStandardAction>
#include <KLocalizedString>
#include <QAction>
#include <QIcon>
using namespace KSieveUi;

SieveEditorMenuBar::SieveEditorMenuBar(QWidget *parent)
    : QMenuBar(parent)
{
    initActions();
    initMenus();
}

SieveEditorMenuBar::~SieveEditorMenuBar()
{

}

void SieveEditorMenuBar::setEditorMode(bool editorMode)
{
    mGoToLine->setEnabled(editorMode);
    mFindAction->setEnabled(editorMode);
    mReplaceAction->setEnabled(editorMode);
    mUndoAction->setEnabled(editorMode);
    mRedoAction->setEnabled(editorMode);
    mCopyAction->setEnabled(editorMode);
    mPasteAction->setEnabled(editorMode);
    mCutAction->setEnabled(editorMode);
    mSelectAllAction->setEnabled(editorMode);
    mCommentCodeAction->setEnabled(editorMode);
    mUncommentCodeAction->setEnabled(editorMode);
    mZoomInAction->setEnabled(editorMode);
    mZoomOutAction->setEnabled(editorMode);
    mZoomResetAction->setEnabled(editorMode);
    mDebugSieveAction->setEnabled(editorMode);
    mWordWrapAction->setEnabled(editorMode);
}

void SieveEditorMenuBar::initActions()
{
    mGoToLine = new QAction(i18n("Go to Line"), this);
    mGoToLine->setIcon(QIcon::fromTheme(QStringLiteral("go-jump")));
    mGoToLine->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
    connect(mGoToLine, &QAction::triggered, this, &SieveEditorMenuBar::gotoLine);

    mCommentCodeAction = new QAction(i18n("Comment"), this);
    mCommentCodeAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
    connect(mCommentCodeAction, &QAction::triggered, this, &SieveEditorMenuBar::comment);

    mUncommentCodeAction = new QAction(i18n("Uncomment"), this);
    mUncommentCodeAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_D));
    connect(mUncommentCodeAction, &QAction::triggered, this, &SieveEditorMenuBar::uncomment);

    mDebugSieveAction = new QAction(i18n("Debug Sieve Script..."), this);
    mDebugSieveAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_D));
    connect(mDebugSieveAction, &QAction::triggered, this, &SieveEditorMenuBar::debugSieveScript);

    mFindAction = KStandardAction::find(this, SIGNAL(find()), this);
    mReplaceAction = KStandardAction::replace(this, SIGNAL(replace()), this);
    mUndoAction = KStandardAction::undo(this, SIGNAL(undo()), this);
    mRedoAction = KStandardAction::redo(this, SIGNAL(redo()), this);
    mCopyAction = KStandardAction::copy(this, SIGNAL(copy()), this);
    mPasteAction = KStandardAction::paste(this, SIGNAL(paste()), this);
    mCutAction = KStandardAction::cut(this, SIGNAL(cut()), this);
    mSelectAllAction = KStandardAction::selectAll(this, SIGNAL(selectAll()), this);
    mZoomInAction = KStandardAction::zoomIn(this, SIGNAL(zoomIn()), this);
    mZoomOutAction = KStandardAction::zoomOut(this, SIGNAL(zoomOut()), this);

    mZoomResetAction = new QAction(i18nc("Reset the zoom", "Reset"), this);
    mZoomResetAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_0));
    connect(mZoomResetAction, &QAction::triggered, this, &SieveEditorMenuBar::zoomReset);

    mWordWrapAction = new QAction(i18n("Wordwrap"), this);
    mWordWrapAction->setCheckable(true);
    connect(mWordWrapAction, &QAction::triggered, this, &SieveEditorMenuBar::wordWrap);

    mUndoAction->setEnabled(false);
    mRedoAction->setEnabled(false);
    mCopyAction->setEnabled(false);
    mCutAction->setEnabled(false);
}

QMenu *SieveEditorMenuBar::editorMenu() const
{
    return mEditorMenu;
}

void SieveEditorMenuBar::initMenus()
{
    mFileMenu = addMenu(i18nc("@title:menu", "File"));
    mEditorMenu = addMenu(i18nc("@title:menu", "Edit"));
    mToolsMenu = addMenu(i18nc("@title:menu", "Tools"));
    mEditorMenu->addAction(mUndoAction);
    mEditorMenu->addAction(mRedoAction);
    mEditorMenu->addSeparator();
    mEditorMenu->addAction(mCutAction);
    mEditorMenu->addAction(mCopyAction);
    mEditorMenu->addAction(mPasteAction);
    mEditorMenu->addSeparator();
    mEditorMenu->addAction(mSelectAllAction);
    mEditorMenu->addSeparator();
    mEditorMenu->addAction(mFindAction);
    mEditorMenu->addAction(mReplaceAction);
    mEditorMenu->addSeparator();
    mEditorMenu->addAction(mGoToLine);
    mEditorMenu->addSeparator();
    mEditorMenu->addAction(mZoomInAction);
    mEditorMenu->addAction(mZoomOutAction);
    mEditorMenu->addAction(mZoomResetAction);
    mEditorMenu->addSeparator();
    mEditorMenu->addAction(mWordWrapAction);

    mToolsMenu->addAction(mCommentCodeAction);
    mToolsMenu->addAction(mUncommentCodeAction);
    mToolsMenu->addSeparator();
    mToolsMenu->addAction(mDebugSieveAction);
}

QAction *SieveEditorMenuBar::uncommentCodeAction() const
{
    return mUncommentCodeAction;
}

QAction *SieveEditorMenuBar::zoomResetAction() const
{
    return mZoomResetAction;
}

QAction *SieveEditorMenuBar::wordWrapAction() const
{
    return mWordWrapAction;
}

QAction *SieveEditorMenuBar::zoomInAction() const
{
    return mZoomInAction;
}

QAction *SieveEditorMenuBar::zoomOutAction() const
{
    return mZoomOutAction;
}

QAction *SieveEditorMenuBar::debugSieveScriptAction() const
{
    return mDebugSieveAction;
}

QAction *SieveEditorMenuBar::commentCodeAction() const
{
    return mCommentCodeAction;
}

QMenu *SieveEditorMenuBar::fileMenu() const
{
    return mFileMenu;
}

QMenu *SieveEditorMenuBar::toolsMenu() const
{
    return mToolsMenu;
}
QAction *SieveEditorMenuBar::selectAllAction() const
{
    return mSelectAllAction;
}

QAction *SieveEditorMenuBar::cutAction() const
{
    return mCutAction;
}

QAction *SieveEditorMenuBar::pasteAction() const
{
    return mPasteAction;
}

QAction *SieveEditorMenuBar::copyAction() const
{
    return mCopyAction;
}

QAction *SieveEditorMenuBar::redoAction() const
{
    return mRedoAction;
}

QAction *SieveEditorMenuBar::undoAction() const
{
    return mUndoAction;
}

QAction *SieveEditorMenuBar::replaceAction() const
{
    return mReplaceAction;
}

QAction *SieveEditorMenuBar::findAction() const
{
    return mFindAction;
}

QAction *SieveEditorMenuBar::goToLineAction() const
{
    return mGoToLine;
}

void SieveEditorMenuBar::slotUndoAvailable(bool b)
{
    mUndoAction->setEnabled(b);
}

void SieveEditorMenuBar::slotRedoAvailable(bool b)
{
    mRedoAction->setEnabled(b);
}

void SieveEditorMenuBar::slotCopyAvailable(bool b)
{
    mCutAction->setEnabled(b);
    mCopyAction->setEnabled(b);
}

