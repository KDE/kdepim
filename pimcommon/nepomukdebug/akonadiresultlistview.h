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

#ifndef AKONADIRESULTLISTVIEW_H
#define AKONADIRESULTLISTVIEW_H

#include "pimcommon/pimcommon_export.h"
#include <QListView>
#include <QStyledItemDelegate>

namespace PimCommon {
class PIMCOMMON_EXPORT AkonadiResultListView : public QListView
{
    Q_OBJECT
public:
    explicit AkonadiResultListView(QWidget *parent=0);
    ~AkonadiResultListView();

protected:
    void contextMenuEvent( QContextMenuEvent *event );
};

class AkonadiResultListDelegate : public QStyledItemDelegate
{
public:
    AkonadiResultListDelegate(QObject *parent = 0);
    ~AkonadiResultListDelegate();
    QWidget *createEditor( QWidget *, const QStyleOptionViewItem &, const QModelIndex & ) const;
};
}

#endif // AKONADIRESULTLISTVIEW_H
