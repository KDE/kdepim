/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "richtexteditfindbar.h"
#include "pimcommon/texteditor/commonwidget/textfindreplacewidget.h"

#include <KLocalizedString>
#include <kmessagebox.h>
#include <klineedit.h>
#include <KColorScheme>

#include <QTextEdit>

using namespace PimCommon;
class PimCommon::RichTextEditFindBarPrivate
{
public:
    RichTextEditFindBarPrivate()
        : mView(Q_NULLPTR)
    {

    }

    QTextEdit *mView;
};



RichTextEditFindBar::RichTextEditFindBar(QTextEdit *view, QWidget *parent)
    : TextEditFindBarBase(parent),
      d(new PimCommon::RichTextEditFindBarPrivate)
{
}

RichTextEditFindBar::~RichTextEditFindBar()
{
    delete d;
}

void RichTextEditFindBar::slotSearchText(bool backward, bool isAutoSearch)
{
    d->mView->moveCursor(QTextCursor::Start);
    searchText(backward, isAutoSearch);
}

bool RichTextEditFindBar::viewIsReadOnly() const
{
    return d->mView->isReadOnly();
}

bool RichTextEditFindBar::documentIsEmpty() const
{
    return d->mView->document()->isEmpty();
}

bool RichTextEditFindBar::searchInDocument(const QString &text, QTextDocument::FindFlags searchOptions)
{
    const bool found = d->mView->find(text, searchOptions);
    mFindWidget->setFoundMatch(found);
    return found;
}

void RichTextEditFindBar::autoSearchMoveCursor()
{
    QTextCursor cursor = d->mView->textCursor();
    cursor.setPosition(cursor.selectionStart());
    d->mView->setTextCursor(cursor);
}

void RichTextEditFindBar::slotReplaceText()
{
    //FIXME!
    if (d->mView->textCursor().hasSelection()) {
        if (d->mView->textCursor().selectedText() == mFindWidget->search()->text()) {
            d->mView->textCursor().insertText(mReplaceWidget->replace()->text());
            //search next after replace text.
            searchText(false, false);
        }
    } else {
        searchText(false, false);
    }
}

void RichTextEditFindBar::slotReplaceAllText()
{
    //FIXME richtext
    d->mView->setPlainText(d->mView->toPlainText().replace(mFindWidget->findRegExp(), mReplaceWidget->replace()->text()));
}

