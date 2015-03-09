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
#include <KAction>
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

void SieveEditorMenuBar::initActions()
{
    //KF5 add i18n
    mGoToLine = new KAction(/*i18n*/QLatin1String("Go to Line"),this);
    mGoToLine->setIcon(KIcon(QLatin1String("go-jump")));
    //mGoToLine->setShortcut(QKeySequence( Qt::CTRL + Qt::Key_G ));
    connect(mGoToLine, SIGNAL(triggered(bool)), SIGNAL(gotoLine()));

    //KF5 add i18n
    mCommentCodeAction = new KAction(QLatin1String("Comment"), this);
    connect(mCommentCodeAction, SIGNAL(triggered(bool)), SIGNAL(comment()));

    mUncommentCodeAction = new KAction(QLatin1String("Uncomment"), this);
    connect(mUncommentCodeAction, SIGNAL(triggered(bool)), SIGNAL(uncomment()));

    mFindAction = KStandardAction::find(this, SIGNAL(find()), this);
    mReplaceAction = KStandardAction::replace(this, SIGNAL(replace()), this);
    mUndoAction = KStandardAction::undo(this, SIGNAL(undo()), this);
    mRedoAction = KStandardAction::redo(this, SIGNAL(redo()), this);
    mCopyAction = KStandardAction::copy(this, SIGNAL(copy()), this);
    mPasteAction = KStandardAction::paste(this, SIGNAL(paste()), this);
    mCutAction = KStandardAction::cut(this, SIGNAL(cut()), this);
    mSelectAllAction = KStandardAction::selectAll(this, SIGNAL(selectAll()), this);
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
    //Add i18n
    mFileMenu = addMenu(QLatin1String("File"));
    mEditorMenu = addMenu(QLatin1String("Edit"));
    mToolsMenu = addMenu(QLatin1String("Tools"));
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

    mToolsMenu->addAction(mCommentCodeAction);
    mToolsMenu->addAction(mUncommentCodeAction);
}
KAction *SieveEditorMenuBar::uncommentCodeAction() const
{
    return mUncommentCodeAction;
}

KAction *SieveEditorMenuBar::commentCodeAction() const
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

KAction *SieveEditorMenuBar::selectAllAction() const
{
    return mSelectAllAction;
}

KAction *SieveEditorMenuBar::cutAction() const
{
    return mCutAction;
}

KAction *SieveEditorMenuBar::pasteAction() const
{
    return mPasteAction;
}

KAction *SieveEditorMenuBar::copyAction() const
{
    return mCopyAction;
}

KAction *SieveEditorMenuBar::redoAction() const
{
    return mRedoAction;
}

KAction *SieveEditorMenuBar::undoAction() const
{
    return mUndoAction;
}

KAction *SieveEditorMenuBar::replaceAction() const
{
    return mReplaceAction;
}

KAction *SieveEditorMenuBar::findAction() const
{
    return mFindAction;
}

KAction *SieveEditorMenuBar::goToLineAction() const
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

