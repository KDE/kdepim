/*
    Copyright (c) 2011 Tobias Koenig <tokoe@kde.org>

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

#ifndef MAILFILTERAGENT_H
#define MAILFILTERAGENT_H

#include <akonadi/agentbase.h>

#include "mailcommon/searchpattern.h"

namespace Akonadi {
class Monitor;
}

class FilterLogDialog;
class FilterManager;
class KJob;

class MailFilterAgent : public Akonadi::AgentBase, public Akonadi::AgentBase::ObserverV2
{
  Q_OBJECT

  public:
    explicit MailFilterAgent( const QString &id );
    ~MailFilterAgent();

    void itemAdded( const Akonadi::Item &item, const Akonadi::Collection &collection );

    QString createUniqueName( const QString &nameTemplate );
    void filterItems( const QVector<qlonglong> &items, int filterSet );

    void filterItem( qlonglong item, int filterSet, const QString &resourceId );
    void filter( qlonglong item, const QString &filterIdentifier, int requires );
    void applySpecificFilters( const QVector<qlonglong> &itemIds, int requires, const QStringList& listFilters );

    void reload();

    void showFilterLogDialog(qlonglong windowId = 0);

  private Q_SLOTS:
    void initializeCollections();
    void initialCollectionFetchingDone( KJob* );
    void mailCollectionAdded( const Akonadi::Collection &collection, const Akonadi::Collection &parent );
    void mailCollectionChanged( const Akonadi::Collection &collection );
    void mailCollectionRemoved( const Akonadi::Collection& collection );
    void emitProgress(int percent);
    void emitProgressMessage(const QString &message);

  private:
    Akonadi::Monitor *m_collectionMonitor;
    FilterManager *m_filterManager;

    FilterLogDialog *m_filterLogDialog;
    MailCommon::SearchRule::RequiredPart mRequestedPart;
};

#endif
