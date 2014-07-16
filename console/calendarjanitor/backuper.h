/*
  Copyright (c) 2013 Sérgio Martins <iamsergio@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef BACKUPER_H
#define BACKUPER_H

#include "options.h"

#include <KCalCore/MemoryCalendar>
#include <Akonadi/Collection>

#include <QObject>
#include <QList>

class KJob;

class Backuper : public QObject
{
    Q_OBJECT
public:
    explicit Backuper(QObject *parent = 0);
    void backup(const QString &filename, const QList<Akonadi::Collection::Id> &collectionIds);

Q_SIGNALS:
    void finished(bool success, const QString &errorMessage);

private Q_SLOTS:
    void onCollectionsFetched(KJob *);
    void onCollectionLoaded(KJob *);

private:
    void loadCollection(const Akonadi::Collection &collection);
    void emitFinished(bool success, const QString &message);

    QList<Akonadi::Collection::Id> m_requestedCollectionIds;
    QList<Akonadi::Collection::Id> m_pendingCollections;

    Akonadi::Collection::List m_collections;
    QString m_filename;
    KCalCore::MemoryCalendar::Ptr m_calendar;

    bool m_backupInProgress;

};

#endif // BACKUPER_H
