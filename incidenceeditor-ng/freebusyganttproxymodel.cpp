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

#include <KCalCore/FreeBusyPeriod>

#include <KGlobal>
#include <KLocale>
#include <KLocalizedString>
#include <KSystemTimeZones>

using namespace IncidenceEditorNG;

FreeBusyGanttProxyModel::FreeBusyGanttProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
{
}

QVariant FreeBusyGanttProxyModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() ) {
    return QVariant();
  }
  QModelIndex source_index = mapToSource( index );

  // if the index is not valid, then its a toplevel item, which is an attendee
  if ( !source_index.parent().isValid() ) {
    switch ( role ) {
    case KDGantt::ItemTypeRole:
      return KDGantt::TypeMulti;
    case Qt::DisplayRole:
      return source_index.data( Qt::DisplayRole );
    default:
      return QVariant();
    }
  }

  // if the index is valid, then it corrsponds to a free busy period
  KDateTime::Spec timeSpec = KSystemTimeZones::local();
  KCalCore::FreeBusyPeriod period =
    sourceModel()->data( source_index, FreeBusyItemModel::FreeBusyPeriodRole ).
    value<KCalCore::FreeBusyPeriod>();

  switch ( role ) {
  case KDGantt::ItemTypeRole:
    return KDGantt::TypeTask;
  case KDGantt::StartTimeRole:
    return period.start().toTimeSpec( timeSpec ).dateTime();
  case KDGantt::EndTimeRole:
    return period.end().toTimeSpec( timeSpec ).dateTime();
  case Qt::BackgroundRole:
    return QColor(Qt::red);
  case Qt::ToolTipRole:
    return tooltipify( period, timeSpec );
  case Qt::DisplayRole:
    return sourceModel()->data( source_index.parent(), Qt::DisplayRole );
  default:
    return QVariant();
  }
}

QString FreeBusyGanttProxyModel::tooltipify( const KCalCore::FreeBusyPeriod &period,
                                             const KDateTime::Spec &timeSpec ) const
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
  toolTip += KLocale::global()->formatDateTime( period.start().toTimeSpec( timeSpec ).dateTime() );
  toolTip += "<br>";
  toolTip += "<i>" + i18nc( "@info:tooltip period end time", "End:" ) + "</i>" + "&nbsp;";
  toolTip += KLocale::global()->formatDateTime( period.end().toTimeSpec( timeSpec ).dateTime() );
  toolTip += "<br>";
  toolTip += "</qt>";
  return toolTip;
}

