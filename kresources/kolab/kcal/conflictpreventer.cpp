/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2012 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Sérgio Martins <sergio.martins@kdab.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "conflictpreventer.h"
#include <libkcal/incidence.h>
#include <libkcal/comparisonvisitor.h>
#include <qmap.h>
#include <qptrlist.h>
#include <kdebug.h>

typedef QString IncidenceUid;
typedef QString ResourceString;

class ConflictPreventer::Private
{
public:
  Private()
  {
  }

  QMap<QPair<ResourceString,IncidenceUid>, KCal::Incidence*> m_payloadsByUid;
  QMap<QPair<QString,Q_INT32>, bool> m_falsePositives;
};

ConflictPreventer::ConflictPreventer() : d( new Private() )
{
}

ConflictPreventer::~ConflictPreventer()
{
  delete d;
}

void ConflictPreventer::registerOldPayload( KCal::Incidence *incidence, const QString &subresource )
{
  Q_ASSERT( incidence );
  KCal::Incidence *clone = incidence->clone();
  kdDebug() << "ConflictPreventer::registerOldPayload() registering " << clone->summary()
            << "; dtStart = " << incidence->dtStart()
            << "; subresource = " << subresource
            << endl;
  const QPair<ResourceString,IncidenceUid> key( subresource, clone->uid() );
  if ( d->m_payloadsByUid.contains( key ) ) {
    delete d->m_payloadsByUid[key];
  }
  d->m_payloadsByUid.insert( key, clone );
}

bool ConflictPreventer::processNewPayload( KCal::Incidence *incidence,
                                           const QString &resource,
                                           Q_INT32 sernum )
{
  Q_ASSERT( incidence );
  const QPair<ResourceString,IncidenceUid> key( resource, incidence->uid() );
  if ( !d->m_payloadsByUid.contains( key ) )
    return false;

  KCal::Incidence *inc = d->m_payloadsByUid[key];
  KCal::ComparisonVisitor v;
  if ( v.compare( inc, incidence ) ) {
    kdDebug() << "ConflictPreventer::isOldPayload() found false positive: "
              << incidence->summary() << endl;
    d->m_falsePositives.insert( QPair<QString,Q_INT32>( resource, sernum ), true );
    return true;
  }
  return false;
}

bool ConflictPreventer::isFalsePositive( const QString &resource, Q_INT32 sernum ) const
{
  const bool result = d->m_falsePositives.contains( QPair<QString,Q_INT32>( resource, sernum ) );
  if ( result )
    kdDebug() << "ConflictPreventer::isFalsePositive() It's a false positive" << endl;
  return result;
}


bool ConflictPreventer::isRegistered( KCal::Incidence *incidence, const QString &subresource ) const
{
  KCal::ComparisonVisitor v;
  const QPair<ResourceString,IncidenceUid> key( subresource, incidence->uid() );
  return d->m_payloadsByUid.contains( key ) &&
         v.compare( d->m_payloadsByUid[key], incidence );
}

void ConflictPreventer::cleanup( const QString &uid, const QString &resource, Q_INT32 sernum )
{
  const QPair<ResourceString,IncidenceUid> key( resource, uid );
  d->m_payloadsByUid.remove( key );
  if ( !resource.isEmpty() )
    d->m_falsePositives.remove( QPair<QString,Q_INT32>( resource, sernum ) );
}
