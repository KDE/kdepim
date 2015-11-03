/****************************************************************************
 ** Copyright (C) 2001-2006 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Gantt library.
 **
 ** This file may be distributed and/or modified under the terms of the
 ** GNU General Public License version 2 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL included in the
 ** packaging of this file.
 **
 ** Licensees holding valid commercial KD Gantt licenses may use this file in
 ** accordance with the KD Gantt Commercial License Agreement provided with
 ** the Software.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** See http://www.kdab.net/kdgantt for
 **   information about KD Gantt Commercial License Agreements.
 **
 ** Contact info@kdab.net if any conditions of this
 ** licensing are not clear to you.
 **
 **********************************************************************/
#ifndef KDGANTTTREEVIEWROWCONTROLLER_H
#define KDGANTTTREEVIEWROWCONTROLLER_H

#include "kdganttabstractrowcontroller.h"

class QAbstractProxyModel;
class QTreeView;

namespace KDGantt
{
class KDGANTT_EXPORT TreeViewRowController :  public AbstractRowController
{
    KDGANTT_DECLARE_PRIVATE_BASE_POLYMORPHIC(TreeViewRowController)
public:
    TreeViewRowController(QTreeView *tv, QAbstractProxyModel *proxy);
    virtual ~TreeViewRowController();

    int headerHeight() const Q_DECL_OVERRIDE;
    int maximumItemHeight() const Q_DECL_OVERRIDE;
    int totalHeight() const Q_DECL_OVERRIDE;
    bool isRowVisible(const QModelIndex &idx) const Q_DECL_OVERRIDE;
    bool isRowExpanded(const QModelIndex &idx) const Q_DECL_OVERRIDE;
    Span rowGeometry(const QModelIndex &idx) const Q_DECL_OVERRIDE;
    QModelIndex indexAt(int height) const Q_DECL_OVERRIDE;
    QModelIndex indexAbove(const QModelIndex &idx) const Q_DECL_OVERRIDE;
    QModelIndex indexBelow(const QModelIndex &idx) const Q_DECL_OVERRIDE;
};
}

#endif /* KDGANTTTREEVIEWROWCONTROLLER_H */

