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

#ifndef CALENDARJANITOR_H
#define CALENDARJANITOR_H

#include "options.h"

#include <KCalCore/Incidence>

#include <Akonadi/Calendar/IncidenceChanger>
#include <Akonadi/Calendar/FetchJobCalendar>
#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <QObject>
#include <QString>
#include <QMultiMap>

class CollectionLoader;

class KJob;

class CalendarJanitor : public QObject
{
    Q_OBJECT
public:
    explicit CalendarJanitor(const Options &options, QObject *parent = 0);

    void start();

Q_SIGNALS:
    void finished(bool success);

private Q_SLOTS:
    void onCollectionsFetched(bool success);
    void onItemsFetched(bool success, const QString &errorMessage);
    void onModifyFinished(int changeId, const Akonadi::Item &item,
                          Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorMessage);
    void onDeleteFinished(int changeId, const QVector<Akonadi::Item::Id> &,
                          Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorMessage);

    void processNextCollection();

    // For each collection we process, we run a bunch of tests on it.
    void runNextTest();

    void sanityCheck1();
    void sanityCheck2();
    void sanityCheck3();
    void sanityCheck4();
    void sanityCheck5();
    void sanityCheck6();
    void sanityCheck7();
    void sanityCheck8();
    void sanityCheck9();

    void stripOldAlarms();

    void printFound(const Akonadi::Item &item, const QString &explanation = QString());

    void beginTest(const QString &message);
    void endTest(bool print = true, const QString fixExplanation = QString(),
                 const QString &fixExplanation2 = QString());

    void deleteIncidence(const Akonadi::Item &item);

private:
    CollectionLoader *m_collectionLoader;
    Akonadi::Collection::List m_collectionsToProcess;
    Akonadi::Item::List m_itemsToProcess;
    Options m_options;
    Akonadi::IncidenceChanger *m_changer;
    Akonadi::Collection m_currentCollection;
    Options::SanityCheck m_currentSanityCheck;
    int m_pendingModifications;
    int m_pendingDeletions;
    bool m_strippingOldAlarms;

    QList<Akonadi::Item::Id> m_test1Results;
    QStringList m_test2Results;

    int m_numDamaged;
    bool m_fixingEnabled;

    QString m_summary; // to print at the end.
    QMultiMap<QString, KCalCore::Incidence::Ptr> m_incidenceMap;
    QMap<KCalCore::Incidence::Ptr, Akonadi::Item> m_incidenceToItem;

    Akonadi::FetchJobCalendar::Ptr m_calendar;
};

#endif // CALENDARJANITOR_H
