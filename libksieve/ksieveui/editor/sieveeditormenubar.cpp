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
    mFindAction = KStandardAction::find(this, SIGNAL(find()), this);
    mReplaceAction = KStandardAction::replace(this, SIGNAL(replace()), this);
    mUndoAction = KStandardAction::undo(this, SIGNAL(undo()), this);
    mRedoAction = KStandardAction::redo(this, SIGNAL(redo()), this);
    mCopyAction = KStandardAction::copy(this, SIGNAL(copy()), this);
    mPasteAction = KStandardAction::paste(this, SIGNAL(paste()), this);
    mCutAction = KStandardAction::cut(this, SIGNAL(cut()), this);
    mSelectAllAction = KStandardAction::selectAll(this, SIGNAL(selectAll()), this);
}
QMenu *SieveEditorMenuBar::editorMenu() const
{
    return mEditorMenu;
}


void SieveEditorMenuBar::initMenus()
{
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
    mEditorMenu->addSeparator();
    mEditorMenu->addAction(mGoToLine);
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

