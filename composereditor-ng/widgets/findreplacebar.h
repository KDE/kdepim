/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#ifndef FINDREPLACEBAR_H
#define FINDREPLACEBAR_H

#include <KWebView>

namespace ComposerEditorNG
{
class FindReplaceBarPrivate;

class FindReplaceBar : public QWidget
{
    Q_OBJECT
public:
    explicit FindReplaceBar(KWebView *parent);
    ~FindReplaceBar();

public Q_SLOTS:
    void showAndFocus();

protected:
    bool event(QEvent* e);

private:
    friend class FindReplaceBarPrivate;
    FindReplaceBarPrivate * const d;
    Q_PRIVATE_SLOT( d, void _k_closeBar() )
    Q_PRIVATE_SLOT( d, void _k_slotHighlightAllChanged(bool) )
    Q_PRIVATE_SLOT( d, void _k_slotCaseSensitivityChanged(bool) )
    Q_PRIVATE_SLOT( d, void _k_slotClearSearch() )
    Q_PRIVATE_SLOT( d, void _k_slotAutoSearch(const QString&) )
    Q_PRIVATE_SLOT( d, void _k_slotSearchText() )
    Q_PRIVATE_SLOT( d, void _k_slotFindNext() )
    Q_PRIVATE_SLOT( d, void _k_slotFindPrevious() )
};
}

#endif // FINDREPLACEBAR_H
