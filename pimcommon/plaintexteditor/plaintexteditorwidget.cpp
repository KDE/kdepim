/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "plaintexteditorwidget.h"
#include "plaintexteditor.h"
#include "plaintexteditfindbar.h"

#include <QVBoxLayout>
#include <QShortcut>
#include <QTextCursor>

using namespace PimCommon;

PlainTextEditorWidget::PlainTextEditorWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    lay->setMargin(0);
    mEditor = new PlainTextEditor;
    lay->addWidget(mEditor);

    mFindBar = new PimCommon::PlainTextEditFindBar( mEditor, this );
    lay->addWidget(mFindBar);

    QShortcut *shortcut = new QShortcut( this );
    shortcut->setKey( Qt::Key_F+Qt::CTRL );
    connect( shortcut, SIGNAL(activated()), SLOT(slotFind()) );
    connect( mEditor, SIGNAL(findText()), SLOT(slotFind()) );

    shortcut = new QShortcut( this );
    shortcut->setKey( Qt::Key_R+Qt::CTRL );
    connect( shortcut, SIGNAL(activated()), SLOT(slotReplace()) );
    connect( mEditor, SIGNAL(replaceText()), SLOT(slotReplace()) );

    setLayout(lay);
}

PlainTextEditorWidget::~PlainTextEditorWidget()
{

}

void PlainTextEditorWidget::slotReplace()
{
    mFindBar->showReplace();
    mFindBar->focusAndSetCursor();
}

void PlainTextEditorWidget::slotFind()
{
    if ( mEditor->textCursor().hasSelection() )
        mFindBar->setText( mEditor->textCursor().selectedText() );
    mEditor->moveCursor(QTextCursor::Start);
    mFindBar->showFind();
    mFindBar->focusAndSetCursor();
}


#include "plaintexteditorwidget.moc"
