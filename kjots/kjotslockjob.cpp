/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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

#include "kjotslockjob.h"
#include <Akonadi/CollectionModifyJob>
#include <AkonadiCore/ItemModifyJob>
#include "noteshared/attributes/notelockattribute.h"

KJotsLockJob::KJotsLockJob( const Akonadi::Collection::List& collections, const Akonadi::Item::List& items, KJotsLockJob::Type type, QObject* parent )
  : Job( parent ), m_collections( collections ), m_items( items ), m_type( type )
{
}

KJotsLockJob::KJotsLockJob( const Akonadi::Collection::List& collections, const Akonadi::Item::List& items, QObject* parent )
  : Job( parent ), m_collections( collections ), m_items( items ), m_type( LockJob )
{
}

KJotsLockJob::~KJotsLockJob()
{

}

void KJotsLockJob::doStart()
{
  foreach ( const Akonadi::Collection &_col, m_collections )
  {
    Akonadi::Collection col = _col;
    if ( m_type == LockJob )
      col.addAttribute( new NoteShared::NoteLockAttribute() );
    else
      col.removeAttribute<NoteShared::NoteLockAttribute>();
    new Akonadi::CollectionModifyJob( col, this );
  }
  foreach ( const Akonadi::Item &_item, m_items )
  {
    Akonadi::Item item = _item;
    if ( m_type == LockJob )
      item.addAttribute( new NoteShared::NoteLockAttribute() );
    else
      item.removeAttribute<NoteShared::NoteLockAttribute>();

    new Akonadi::ItemModifyJob( item, this );
  }
}

void KJotsLockJob::slotResult(KJob* job)
{
  Akonadi::Job::slotResult( job );
  if ( !hasSubjobs() )
    emitResult();
}

