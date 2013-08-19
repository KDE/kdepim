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

#ifndef COMPOSERTABLEACTIONMENU_H
#define COMPOSERTABLEACTIONMENU_H

#include <KActionMenu>
class QWebElement;

namespace ComposerEditorNG
{
class ComposerTableActionMenuPrivate;
class ComposerTableActionMenu : public KActionMenu
{
    Q_OBJECT
public:
    explicit ComposerTableActionMenu(const QWebElement &element, QObject *parent, QWidget *view);
    ~ComposerTableActionMenu();

Q_SIGNALS:
    /**
     * @brief insertNewTable, send signal to insert new table
     */
    void insertNewTable();

private:
    friend class ComposerTableActionMenuPrivate;
    ComposerTableActionMenuPrivate * const d;
    Q_PRIVATE_SLOT( d, void _k_slotInsertRowBelow() )
    Q_PRIVATE_SLOT( d, void _k_slotInsertRowAbove() )
    Q_PRIVATE_SLOT( d, void _k_slotTableFormat() )
    Q_PRIVATE_SLOT( d, void _k_slotTableCellFormat() )
    Q_PRIVATE_SLOT( d, void _k_slotRemoveCellContents() )
    Q_PRIVATE_SLOT( d, void _k_slotRemoveCell() )
    Q_PRIVATE_SLOT( d, void _k_slotInsertCellBefore() )
    Q_PRIVATE_SLOT( d, void _k_slotInsertCellAfter() )
    Q_PRIVATE_SLOT( d, void _k_slotRemoveTable() )
    Q_PRIVATE_SLOT( d, void _k_slotRemoveRow() )
    Q_PRIVATE_SLOT( d, void _k_slotRemoveColumn() )
    Q_PRIVATE_SLOT( d, void _k_slotInsertColumnBefore() )
    Q_PRIVATE_SLOT( d, void _k_slotInsertColumnAfter() )
    Q_PRIVATE_SLOT( d, void _k_slotMergeCellToTheRight() )
    Q_PRIVATE_SLOT( d, void _k_slotSplitCell() )
};
}

#endif // COMPOSERTABLEACTIONMENU_H
