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

#include "freebusyganttproxymodel.h"

#include "freebusyitemmodel.h"

#include <kdgantt2/kdganttgraphicsview.h>

#include <kcalcore/attendee.h>

#include <KDebug>
#include <KSystemTimeZones>
#include <KLocale>
#include <KGlobal>

using namespace IncidenceEditorsNG;

FreeBusyGanttProxyModel::FreeBusyGanttProxyModel( QObject* parent ) : QAbstractProxyModel( parent )
{
}

QVariant FreeBusyGanttProxyModel::data( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();
    QModelIndex source_index = mapToSource( index );
    if ( !source_index.isValid() )
        return QVariant();


    KDateTime::Spec timeSpec = KSystemTimeZones::local();
    KCalCore::FreeBusyPeriod period  = sourceModel()->data( source_index, FreeBusyItemModel::FreeBusyPeriodRole ).value<KCalCore::Period>();
    switch ( role ) {
    case KDGantt::ItemTypeRole:
        return KDGantt::TypeTask;
    case KDGantt::StartTimeRole:
        return period.start().toTimeSpec( timeSpec ).dateTime();
    case KDGantt::EndTimeRole:
        return period.end().toTimeSpec( timeSpec ).dateTime();
    case Qt::BackgroundRole:
        return Qt::red;
    case Qt::ToolTipRole:
        return tooltipify( period, timeSpec );
    case Qt::DisplayRole:
        kDebug() << "OMG";
        kDebug() << sourceModel()->data( source_index.parent(), Qt::DisplayRole ).toString();
        return sourceModel()->data( source_index.parent(), Qt::DisplayRole );
    default:
        return QVariant();
    }
}

int FreeBusyGanttProxyModel::columnCount( const QModelIndex& parent ) const
{
    return 1;
}


int FreeBusyGanttProxyModel::rowCount( const QModelIndex& parent ) const
{
    int count = 0;
    for ( int i = 0; i < sourceModel()->rowCount(); ++i ) {
        QModelIndex parent = sourceModel()->index( i, 1 );
        count += sourceModel()->rowCount( parent );
    }
    return count;
}

QModelIndex FreeBusyGanttProxyModel::mapFromSource( const QModelIndex& sourceIndex ) const
{
    if ( !sourceIndex.isValid() )
        return QModelIndex();

    if ( !sourceIndex.parent().isValid() )
        return QModelIndex();

    int count = 0;
    for ( int i = 0; i < sourceIndex.parent().row(); ++i ) {
        QModelIndex parent = sourceModel()->index( i, 1 );
        count += sourceModel()->rowCount( parent );
    }
    count += sourceIndex.row();

    return index( count, 1 );
}

QModelIndex FreeBusyGanttProxyModel::mapToSource( const QModelIndex& proxyIndex ) const
{
    int proxy_row = proxyIndex.row();
    int count = 0;
    QModelIndex parent;
    bool found = false;
    for ( int i = 0; i < sourceModel()->rowCount(); ++i ) {
        parent = sourceModel()->index( i, 1 );
        count += sourceModel()->rowCount( parent );
        if ( count >= proxy_row ) {
            found = true;
            break;
        }
    }
    if ( !found ) {
        kDebug() << "source model parent not found";
        return QModelIndex();
    }
//         kDebug() << "count - proxy_row" << count - proxy_row;
    return sourceModel()->index( count - proxy_row, 1, parent );
}

QModelIndex FreeBusyGanttProxyModel::index( int row, int column, const QModelIndex& parent ) const
{
    if ( !hasIndex( row, column, parent ) )
        return QModelIndex();

    return createIndex( row, 1 );
}

QModelIndex FreeBusyGanttProxyModel::parent( const QModelIndex& child ) const
{
    return QModelIndex();
}

QString FreeBusyGanttProxyModel::tooltipify( const KCalCore::FreeBusyPeriod& period, const KDateTime::Spec& timeSpec ) const
{
    QString toolTip = "<qt>";
    toolTip += "<b>" + i18nc( "@info:tooltip", "Free/Busy Period" ) + "</b>";
    toolTip += "<hr>";
    if ( !period.summary().isEmpty() ) {
        toolTip += "<i>" + i18nc( "@info:tooltip", "Summary:" ) + "</i>" + "&nbsp;";
        toolTip += period.summary();
        toolTip += "<br>";
    }
    if ( !period.location().isEmpty() ) {
        toolTip += "<i>" + i18nc( "@info:tooltip", "Location:" ) + "</i>" + "&nbsp;";
        toolTip += period.location();
        toolTip += "<br>";
    }
    toolTip += "<i>" + i18nc( "@info:tooltip period start time", "Start:" ) + "</i>" + "&nbsp;";
    toolTip += KGlobal::locale()->formatDateTime( period.start().toTimeSpec( timeSpec ).dateTime() );
    toolTip += "<br>";
    toolTip += "<i>" + i18nc( "@info:tooltip period end time", "End:" ) + "</i>" + "&nbsp;";
    toolTip += KGlobal::locale()->formatDateTime( period.end().toTimeSpec( timeSpec ).dateTime() );
    toolTip += "<br>";
    toolTip += "</qt>";
    return toolTip;
}

