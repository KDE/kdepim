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

using namespace KSieveUi;

SieveEditorMenuBar::SieveEditorMenuBar(QWidget *parent)
    : QMenuBar(parent)
{

}

SieveEditorMenuBar::~SieveEditorMenuBar()
{

}

void SieveEditorMenuBar::initActions()
{
#if 0
    mGoToLine = ac->addAction(QLatin1String("gotoline"), mMainWidget->sieveEditorMainWidget(), SLOT(slotGoToLine()));
    mGoToLine->setText(i18n("Go to Line"));
    mGoToLine->setIcon(KIcon(QLatin1String("go-jump")));
    mGoToLine->setShortcut(QKeySequence( Qt::CTRL + Qt::Key_G ));
    mGoToLine->setEnabled(false);

    mFindAction = KStandardAction::find(this, SIGNAL(find()), this);
    mReplaceAction = KStandardAction::replace(this, SIGNAL(replace()), this);
    mUndoAction = KStandardAction::undo(this, SIGNAL(undo()), this);
    mRedoAction = KStandardAction::redo(this, SIGNAL(redo()), this);
    mCopyAction = KStandardAction::copy(this, SIGNAL(copy()), this);
    mPasteAction = KStandardAction::paste(this, SIGNAL(paste()), this);
    mCutAction = KStandardAction::cut(this, SIGNAL(cut()), this);
    mSelectAllAction = KStandardAction::selectAll(this, SIGNAL(selectAll()), this);
#endif
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

QAction *SieveEditorMenuBar::goToLine() const
{
    return mGoToLine;
}

