/*
  Copyright (C) 2016 eyeOS S.L.U., a Telefonica company, sales@eyeos.com

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


#include "handlelisthtml.h"

#include <QTextCursor>
#include <QTextList>
#include <QTextEdit>
#include <QDebug>

HandleListHtml::HandleListHtml(QTextEdit *te)
    : mTextEdit(te)
{
    listBottomMargin = 12;
    listTopMargin = 12;
    listNoMargin = 0;
}

HandleListHtml::~HandleListHtml()
{

}

void HandleListHtml::handleOnBulletType(int styleIndex)
{
    qDebug()<<" styleIndex"<<styleIndex;
    QTextCursor cursor = mTextEdit->textCursor();
    if (styleIndex != 0) {
        QTextListFormat::Style style = (QTextListFormat::Style)styleIndex;
        QTextList *currentList = cursor.currentList();
        QTextListFormat listFmt;

        cursor.beginEditBlock();

        if (currentList) {
            listFmt = currentList->format();
            listFmt.setStyle(style);
            currentList->setFormat(listFmt);
        } else {
            listFmt.setStyle(style);
            cursor.createList(listFmt);
        }

        cursor.endEditBlock();
    } else {
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.setBlockFormat(bfmt);
        reformatBoundingItemSpacing();
    }

    reformatBoundingItemSpacing();
    reformatList();
    mTextEdit->setTextCursor(cursor);
}

void HandleListHtml::reformatBoundingItemSpacing(QTextBlock block)
{
    // This is a workaround for qt bug 201228. Spacing between items is not kept
    // consistent.
    // Fixed scheduled for qt4.5
    // -- Stephen Kelly, 8th June 2008

    int nextBlockTopMargin = listNoMargin;
    int previousBlockBottomMargin = listNoMargin;
    int thisBlockBottomMargin = listBottomMargin;
    int thisBlockTopMargin = listTopMargin;
    bool prevBlockValid = block.previous().isValid();
    bool nextBlockValid = block.next().isValid();

    if (block.textList()) {
        if (prevBlockValid && block.previous().textList()) {
            thisBlockTopMargin = listNoMargin;
        }

        if (nextBlockValid && block.next().textList()) {
            thisBlockBottomMargin = listNoMargin;
        }
    } else {
        if (prevBlockValid && !block.previous().textList()) {
            thisBlockTopMargin = listNoMargin;
        }
        if (nextBlockValid && !block.next().textList()) {
            thisBlockBottomMargin = listNoMargin;
        }

    }
    QTextBlockFormat fmt;
    QTextCursor cursor;

    fmt = block.blockFormat();
    fmt.setBottomMargin(thisBlockBottomMargin);
    fmt.setTopMargin(thisBlockTopMargin);
    cursor = QTextCursor(block);
    cursor.setBlockFormat(fmt);

    if (nextBlockValid) {
        block = block.next();
        fmt = block.blockFormat();
        fmt.setTopMargin(nextBlockTopMargin);
        cursor = QTextCursor(block);
        cursor.setBlockFormat(fmt);

        block = block.previous();
    }
    if (prevBlockValid) {
        block = block.previous();
        fmt = block.blockFormat();
        fmt.setBottomMargin(previousBlockBottomMargin);
        cursor = QTextCursor(block);
        cursor.setBlockFormat(fmt);
    }
}

void HandleListHtml::reformatBoundingItemSpacing()
{
    reformatBoundingItemSpacing(topOfSelection().block());
    reformatBoundingItemSpacing(bottomOfSelection().block());
}


void HandleListHtml::reformatList(QTextBlock block)
{
    if (block.textList()) {
        int minimumIndent =  block.textList()->format().indent();

        // Start at the top of the list
        while (block.previous().textList() != 0) {
            if (block.previous().textList()->format().indent() < minimumIndent) {
                break;
            }
            block = block.previous();
        }

        processList(block.textList());

    }
}

void HandleListHtml::reformatList()
{
    QTextCursor cursor = mTextEdit->textCursor();
    reformatList(cursor.block());
}

void HandleListHtml::processList(QTextList *list)
{
    QTextBlock block = list->item(0);
    int thisListIndent = list->format().indent();

    QTextCursor cursor = QTextCursor(block);
    list = cursor.createList(list->format());
    bool processingSubList  = false;
    while (block.next().textList() != 0) {
        block = block.next();

        QTextList *nextList = block.textList();
        int nextItemIndent = nextList->format().indent();
        if (nextItemIndent < thisListIndent) {
            return;
        } else if (nextItemIndent > thisListIndent) {
            if (processingSubList) {
                continue;
            }
            processingSubList = true;
            processList(nextList);
        } else {
            processingSubList = false;
            list->add(block);
        }
    }
//     delete nextList;
//     nextList = 0;
}

QTextCursor HandleListHtml::topOfSelection()
{
    QTextCursor cursor = mTextEdit->textCursor();

    if (cursor.hasSelection()) {
        cursor.setPosition(qMin(cursor.position(), cursor.anchor()));
    }
    return cursor;
}

QTextCursor HandleListHtml::bottomOfSelection()
{
    QTextCursor cursor = mTextEdit->textCursor();

    if (cursor.hasSelection()) {
        cursor.setPosition(qMax(cursor.position(), cursor.anchor()));
    }
    return cursor;
}
