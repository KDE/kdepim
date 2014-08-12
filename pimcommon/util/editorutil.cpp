/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "editorutil.h"

void PimCommon::EditorUtil::upperCase( QTextCursor &cursor )
{
    if (cursor.hasSelection()) {
        const QString newText = cursor.selectedText().toUpper();
        cursor.insertText(newText);
    }
}

void PimCommon::EditorUtil::lowerCase( QTextCursor &cursor )
{
    if (cursor.hasSelection()) {
        const QString newText = cursor.selectedText().toLower();
        cursor.insertText(newText);
    }
}

void PimCommon::EditorUtil::sentenceCase( QTextCursor &cursor )
{
    if (cursor.hasSelection()) {
        QString newText = cursor.selectedText();
        const int nbChar(newText.count());
        for (int i = 0; i <nbChar; ++i) {
            if (i==0 && newText.at(0).isLetter()) {
                newText.replace(0, 1,newText.at(0).toUpper());
            } else if (newText.at(i) == QChar::ParagraphSeparator || newText.at(i) == QChar::LineSeparator) {
                ++i;
                if (i <nbChar) {
                    if (newText.at(i).isLetter()) {
                        newText.replace(i, 1,newText.at(i).toUpper());
                    }
                }
            }
        }
        cursor.insertText(newText);
    }
}
