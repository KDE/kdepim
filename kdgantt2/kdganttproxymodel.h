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
#ifndef KDGANTTPROXYMODEL_H
#define KDGANTTPROXYMODEL_H

#include "kdganttforwardingproxymodel.h"

namespace KDGantt
{
class KDGANTT_EXPORT ProxyModel : public ForwardingProxyModel
{
    Q_OBJECT
    Q_DISABLE_COPY(ProxyModel)
    KDGANTT_DECLARE_PRIVATE_BASE_POLYMORPHIC(ProxyModel)
public:
    explicit ProxyModel(QObject *parent = Q_NULLPTR);
    virtual ~ProxyModel();

    void setColumn(int ganttrole, int col);
    void setRole(int ganttrole, int role);

    int column(int ganttrole) const;
    int role(int ganttrole) const;

#if 0
    void setCalendarMode(bool enable);
    bool calendarMode() const;
#endif

    QModelIndex mapFromSource(const QModelIndex &idx) const Q_DECL_OVERRIDE;
    QModelIndex mapToSource(const QModelIndex &proxyIdx) const Q_DECL_OVERRIDE;

    int rowCount(const QModelIndex &idx) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &idx) const Q_DECL_OVERRIDE;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &idx, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
};
}

#endif /* KDGANTTPROXYMODEL_H */

