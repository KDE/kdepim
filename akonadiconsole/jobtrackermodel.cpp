/*
 This file is part of Akonadi.

 Copyright (c) 2009 Till Adam <adam@kde.org>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 USA.
 */

#include "jobtrackermodel.h"
#include "jobtracker.h"

#include <QtCore/QStringList>
#include <QtCore/QModelIndex>
#include <QtCore/QDebug>
#include <QtCore/QDateTime>

#include <cassert>

class JobTrackerModel::Private
{
public:
  Private( JobTrackerModel* _q )
  :q(_q)
  {

  }
  JobTracker tracker;
private:
  JobTrackerModel* const q;
};

JobTrackerModel::JobTrackerModel( QObject* parent )
:QAbstractItemModel( parent ), d( new Private(this ) )
{
  connect( &d->tracker, SIGNAL( updated() ),
           this, SIGNAL( modelReset() ) );
}

JobTrackerModel::~JobTrackerModel()
{
  delete d;
}

QModelIndex JobTrackerModel::index(int row, int column, const QModelIndex & parent) const
{
  if ( !parent.isValid() ) // session, at top level
  {
    if ( row < 0 || row >= d->tracker.sessions().size() )
      return QModelIndex();
    return createIndex( row, column, d->tracker.idForSession( d->tracker.sessions().at(row) ) );
  }
  // non-toplevel job
  const QList<JobInfo> jobs = d->tracker.jobs( parent.internalId() );
  if ( row >= jobs.size() ) return QModelIndex();
  return createIndex( row, column, d->tracker.idForJob( jobs.at( row ).id ) );
}

QModelIndex JobTrackerModel::parent(const QModelIndex & idx) const
{
  if ( !idx.isValid() ) return QModelIndex();

  const int parentid = d->tracker.parentId( idx.internalId() );
  if ( parentid == -1 ) return QModelIndex(); // top level session

  const int grandparentid = d->tracker.parentId( parentid );
  int row = -1;
  if ( grandparentid == -1 )
  {
    row = d->tracker.sessions().indexOf( d->tracker.sessionForId( parentid ) );
  }
  else
  {
    // offset of the parent in the list of children of the grandparent
    row = d->tracker.jobs( grandparentid ).indexOf( d->tracker.info( parentid ) );
  }
  assert( row != -1 );
  return createIndex( row, 0, parentid );
}

int JobTrackerModel::rowCount(const QModelIndex & parent) const
{
  if ( !parent.isValid() )
  {
    return d->tracker.sessions().size();
  }
  else
  {
    return d->tracker.jobs( parent.internalId() ).size();
  }
}

int JobTrackerModel::columnCount(const QModelIndex & parent) const
{
  return 4;
}

QVariant JobTrackerModel::data(const QModelIndex & idx, int role) const
{
  // top level items are sessions
  if ( !idx.parent().isValid() )
  {
    if ( role == Qt::DisplayRole )
    {
      if( idx.column() == 0 )
      {
        assert(d->tracker.sessions().size() > idx.row());
        return d->tracker.sessions().at(idx.row());
      }
    }
  }
  else // not top level, so a job or subjob
  {
    const int id = idx.internalId();
    const JobInfo info = d->tracker.info( id );
    if ( role == Qt::DisplayRole )
    {
      if ( idx.column() == 0 )
        return info.id;
      if ( idx.column() == 1 )
        return info.timestamp;
      if ( idx.column() == 2 )
        return info.type;
      if ( idx.column() == 3 )
        return info.stateAsString();
    }
  }
  return QVariant();
}

QVariant JobTrackerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ( role == Qt::DisplayRole )
  {
    if ( orientation == Qt::Horizontal )
    {
      if ( section == 0 )
        return QLatin1String("Job ID");
      if ( section == 1 )
        return QLatin1String("Timestamp");
      if ( section == 2 )
        return QLatin1String("Job Type");
      if ( section == 3 )
        return QLatin1String("State");

    }
  }
  return QVariant();
}

Qt::ItemFlags JobTrackerModel::flags(const QModelIndex & index) const
{
  Qt::ItemFlags f = QAbstractItemModel::flags( index );
  if ( index.isValid() && index.parent().isValid() )
  {
    const JobInfo info = d->tracker.info( index.internalId() );
    if ( info.state == JobInfo::Running )
      f ^= Qt::ItemIsEnabled;
  }
  return f;
}

#include "jobtrackermodel.moc"
