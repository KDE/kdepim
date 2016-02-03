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


#ifndef HANDLELISTHTML_H
#define HANDLELISTHTML_H
#include <QTextBlock>
class QTextEdit;
class HandleListHtml
{
public:
    explicit HandleListHtml(QTextEdit *te);
    ~HandleListHtml();
    void handleOnBulletType(int styleIndex);
private:
    void reformatBoundingItemSpacing(QTextBlock block);
    void reformatBoundingItemSpacing();
    void reformatList(QTextBlock block);
    void reformatList();
    void processList(QTextList *list);
    QTextEdit *mTextEdit;
    int listBottomMargin;
    int listTopMargin;
    int listNoMargin;
    QTextCursor bottomOfSelection();
    QTextCursor topOfSelection();
};

#endif // HANDLELISTHTML_H
