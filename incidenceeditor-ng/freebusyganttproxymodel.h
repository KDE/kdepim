/*
  Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
  Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#ifndef INCIDENCEEDITOR_FREEBUSYGANTTPROXYMODEL_H
#define INCIDENCEEDITOR_FREEBUSYGANTTPROXYMODEL_H

#include "incidenceeditors_ng_export.h"

#include <KDateTime>

#include <QSortFilterProxyModel>

namespace KCalCore {
  class FreeBusyPeriod;
}

namespace IncidenceEditorNG {

/**
 * This is a private proxy model, that wraps the free busy data exposed
 * by the FreeBusyItemModel for use by KDGantt2.
 *
 * This model exposes the FreeBusyPeriods, which are the child level nodes
 * in FreeBusyItemModel, as a list.
 *
 * @see FreeBusyItemMode
 * @see FreeBusyItem
 */
class INCIDENCEEDITORS_NG_EXPORT FreeBusyGanttProxyModel : public QSortFilterProxyModel
{
  public:
    explicit FreeBusyGanttProxyModel( QObject *parent = 0 );
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    QString tooltipify( const KCalCore::FreeBusyPeriod &period,
                        const KDateTime::Spec &timeSpec ) const;
};

}

#endif
