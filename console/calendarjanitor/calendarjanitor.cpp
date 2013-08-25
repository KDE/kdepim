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

#include "calendarjanitor.h"
#include "collectionloader.h"

#include <calendarsupport/utils.h>

#include <KCalCore/Attachment>
#include <KCalCore/Alarm>
#include <KCalCore/Event>
#include <KCalCore/Todo>
#include <KCalCore/Journal>

#include <KLocale>
#include <KDateTime>

#include <QList>
#include <QString>
#include <QTextStream>
#include <QCoreApplication>

#define TEXT_WIDTH 70

static void print(const QString &message, bool newline = true)
{
    QTextStream out(stdout);
    out << message;
    if (newline)
        out << "\n";
}

static void bailOut()
{
    print(i18n("Bailing out. Fix your akonadi setup first. These kind of errors should not happen."));
    qApp->exit(-1);
}

static bool collectionIsReadOnly(const Akonadi::Collection &collection)
{
    return !(collection.rights() & Akonadi::Collection::CanChangeItem) ||
           !(collection.rights() & Akonadi::Collection::CanDeleteItem);
}

static bool incidenceIsOld(const KCalCore::Incidence::Ptr &incidence)
{
    if (incidence->recurs() || incidence->type() == KCalCore::Incidence::TypeJournal)
        return false;

    KDateTime datetime = incidence->dtStart();
    if (!datetime.isValid() && incidence->type() == KCalCore::Incidence::TypeTodo) {
        datetime = incidence->dateTime(KCalCore::Incidence::RoleEnd);
    }

    return datetime.isValid() && datetime.daysTo(KDateTime::currentDateTime(KDateTime::LocalZone)) > 365;
}

CalendarJanitor::CalendarJanitor(const Options &options, QObject *parent) : QObject(parent)
                                                                          , m_collectionLoader(new CollectionLoader(this))
                                                                          , m_options(options)
                                                                          , m_currentSanityCheck(Options::CheckNone)
                                                                          , m_pendingModifications(0)
                                                                          , m_pendingDeletions(0)
                                                                          , m_strippingOldAlarms(false)
{
    m_changer = new Akonadi::IncidenceChanger(this);
    m_changer->setShowDialogsOnError(false);
    connect(m_changer, SIGNAL(modifyFinished(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)),
            SLOT(onModifyFinished(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)));
    connect(m_changer, SIGNAL(deleteFinished(int,QVector<Akonadi::Item::Id>,Akonadi::IncidenceChanger::ResultCode,QString)),
            SLOT(onDeleteFinished(int,QVector<Akonadi::Item::Id>,Akonadi::IncidenceChanger::ResultCode,QString)));
    connect(m_collectionLoader, SIGNAL(loaded(bool)), SLOT(onCollectionsFetched(bool)));
}

void CalendarJanitor::start()
{
    m_collectionLoader->load();
}

void CalendarJanitor::onCollectionsFetched(bool success)
{
    if (!success) {
        print(i18n("Error while fetching collections"));
        emit finished(false);
        qApp->exit(-1);
        return;
    }

    foreach (const Akonadi::Collection &collection, m_collectionLoader->collections()) {
        if (m_options.testCollection(collection.id()))
            m_collectionsToProcess << collection;
    }

    if (m_collectionsToProcess.isEmpty()) {
        print(i18n("There are no collections to process!"));
        qApp->exit((-1));
        return;
    }

    // Load all items:
    m_calendar = Akonadi::FetchJobCalendar::Ptr(new Akonadi::FetchJobCalendar());
    connect(m_calendar.data(), SIGNAL(loadFinished(bool,QString)), SLOT(onItemsFetched(bool,QString)));
}

void CalendarJanitor::onItemsFetched(bool success, const QString &errorMessage)
{
    if (!success) {
        print(errorMessage);
        emit finished(false);
        qApp->exit(-1);
        return;
    }

    // Start processing collections
    processNextCollection();
}

void CalendarJanitor::onModifyFinished(int changeId, const Akonadi::Item &item,
                                       Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorMessage)
{
    Q_UNUSED(changeId);
    if (resultCode != Akonadi::IncidenceChanger::ResultCodeSuccess) {
        print(i18n("Error while modifying incidence: %1!", errorMessage));
        bailOut();
        return;
    }
    if (!m_options.stripOldAlarms())
        print(i18n("Fixed item %1", item.id()));

    m_pendingModifications--;
    if (m_pendingModifications == 0) {
        runNextTest();
    }
}

void CalendarJanitor::onDeleteFinished(int changeId, const QVector<Akonadi::Entity::Id> &items,
                                       Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorMessage)
{
    Q_UNUSED(changeId);
    if (resultCode != Akonadi::IncidenceChanger::ResultCodeSuccess) {
        print(i18n("Error while deleting incidence: %1!", errorMessage));
        bailOut();
        return;
    }
    print(i18n("Deleted item %1", items.first()));
    m_pendingDeletions--;
    if (m_pendingDeletions == 0) {
        runNextTest();
    }
}

void CalendarJanitor::processNextCollection()
{
    m_itemsToProcess.clear();
    m_currentSanityCheck = Options::CheckNone;
    m_strippingOldAlarms = false;

    if (m_collectionsToProcess.isEmpty()) {
        print(QLatin1Char('\n') + QString().leftJustified(TEXT_WIDTH, QLatin1Char('*')));
        emit finished(true);
        qApp->exit(0);
        return;
    }

    m_currentCollection = m_collectionsToProcess.takeFirst();
    print(QLatin1Char('\n') + QString().leftJustified(TEXT_WIDTH, QLatin1Char('*')));
    print(i18n("Processing collection %1 (id=%2) ...", m_currentCollection.displayName(), m_currentCollection.id()));

    if (collectionIsReadOnly(m_currentCollection)) {
        if (m_options.action() == Options::ActionScanAndFix) {
            print(i18n("Collection is read only, disabling fix mode."));
        } else if (m_options.stripOldAlarms()) {
            print(i18n("Collection is read only, skipping it."));
            processNextCollection();
            return;
        }
    }

    m_itemsToProcess = m_calendar->items(m_currentCollection.id());
    if (m_itemsToProcess.isEmpty()) {
        print(i18n("Collection is empty, ignoring it."));
        processNextCollection();
    } else {
        m_incidenceMap.clear();
        foreach (const Akonadi::Item &item, m_itemsToProcess) {
            KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
            Q_ASSERT(incidence);
            m_incidenceMap.insert(incidence->instanceIdentifier(), incidence);
            m_incidenceToItem.insert(incidence, item);
        }
        runNextTest();
    }
}

void CalendarJanitor::runNextTest()
{
    if (m_options.stripOldAlarms()) {
        if (!m_strippingOldAlarms) {
            m_strippingOldAlarms = true;
            stripOldAlarms();
        } else {
            processNextCollection();
        }

        return;
    }

    int currentType = static_cast<int>(m_currentSanityCheck);
    m_currentSanityCheck = static_cast<Options::SanityCheck>(currentType+1);

    switch(m_currentSanityCheck) {
    case Options::CheckEmptySummary:
        sanityCheck1();
        break;
    case Options::CheckEmptyUid:
        sanityCheck2();
        break;
    case Options::CheckEventDates:
        sanityCheck3();
        break;
    case Options::CheckTodoDates:
        sanityCheck4();
        break;
    case Options::CheckJournalDates:
        sanityCheck5();
        break;
    case Options::CheckOrphans:
        //sanityCheck6(); // Disabled for now
        runNextTest();
        break;
    case Options::CheckDuplicateUIDs:
        sanityCheck7();
        break;
    case Options::CheckStats:
        sanityCheck8();
        break;
    case Options::CheckCount:
        processNextCollection();
        break;
    default:
        Q_ASSERT(false);
    }
}

void CalendarJanitor::sanityCheck1()
{
    beginTest(i18n("Checking for incidences with empty summary and description..."));

    foreach (const Akonadi::Item &item, m_itemsToProcess) {
        KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        if (incidence->summary().isEmpty() && incidence->description().isEmpty()
            && incidence->attachments().isEmpty()) {
            printFound(item);
            deleteIncidence(item);
        }
    }

    endTest();
}

void CalendarJanitor::sanityCheck2()
{
    beginTest(i18n("Checking for incidences with empty UID..."));

    foreach (const Akonadi::Item &item, m_itemsToProcess) {
        KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        if (incidence->uid().isEmpty()) {
            printFound(item);
            if (m_fixingEnabled) {
                incidence->recreate();
                m_pendingModifications++;
                m_changer->modifyIncidence(item);
            }
        }
    }

    endTest();
}

void CalendarJanitor::sanityCheck3()
{
    beginTest(i18n("Checking for events with invalid DTSTART..."));
    foreach (const Akonadi::Item &item, m_itemsToProcess) {
        KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        KCalCore::Event::Ptr event = incidence.dynamicCast<KCalCore::Event>();
        if (!event)
            continue;

        KDateTime start = event->dtStart();
        KDateTime end   = event->dtEnd();

        bool modify = false;
        QString message;
        if (!start.isValid() && end.isValid()) {
            modify = true;
            printFound(item);
            event->setDtStart(end);
        } else if (!start.isValid() && !end.isValid()) {
            modify = true;
            printFound(item);
            event->setDtStart(KDateTime::currentLocalDateTime());
            event->setDtEnd(event->dtStart().addSecs(3600));
        }

        if (modify) {
            if (m_fixingEnabled) {
                m_changer->modifyIncidence(item);
                m_pendingModifications++;
            }
        }
    }

    endTest();
}

void CalendarJanitor::sanityCheck4()
{
    beginTest(i18n("Checking for recurring to-dos with invalid DTSTART..."));
    foreach (const Akonadi::Item &item, m_itemsToProcess) {
        KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        KCalCore::Todo::Ptr todo = incidence.dynamicCast<KCalCore::Todo>();
        if (!todo)
            continue;

        KDateTime start = todo->dtStart();
        KDateTime due   = todo->dtDue();
        bool modify = false;
        if (todo->recurs() && !start.isValid() && due.isValid()) {
            modify = true;
            printFound(item);
            todo->setDtStart(due);
        }

        if (todo->recurs() && !start.isValid() && !due.isValid()) {
            modify = true;
            printFound(item);
            todo->setDtStart(KDateTime::currentLocalDateTime());
        }

        if (modify) {
            if (m_fixingEnabled) {
                m_changer->modifyIncidence(item);
                m_pendingModifications++;
            }
        }
    }

    endTest();
}

void CalendarJanitor::sanityCheck5()
{
    beginTest(i18n("Checking for journals with invalid DTSTART..."));
    foreach (const Akonadi::Item &item, m_itemsToProcess) {
        KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        if (incidence->type() != KCalCore::Incidence::TypeJournal)
            continue;

        if (!incidence->dtStart().isValid()) {
            printFound(item);
            incidence->setDtStart(KDateTime::currentLocalDateTime());
            if (m_fixingEnabled) {
                m_changer->modifyIncidence(item);
                m_pendingModifications++;
            }
        }
    }
    endTest();
}

void CalendarJanitor::sanityCheck6()
{
    beginTest(i18n("Checking for orphans...")); // Incidences without a parent

    foreach (const Akonadi::Item &item, m_itemsToProcess) {
        KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        const QString parentUid = incidence->relatedTo();
        if (!parentUid.isEmpty() && !m_incidenceMap.contains(parentUid)) {
            printFound(item);
            if (m_fixingEnabled) {
                incidence->setRelatedTo(QString());
                m_changer->modifyIncidence(item);
                m_pendingModifications++;
            }
        }
    }

    endTest();
}

void CalendarJanitor::sanityCheck7()
{
    beginTest(i18n("Checking for duplicate UIDs..."));

    foreach (const Akonadi::Item &item, m_itemsToProcess) {
        KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        QList<KCalCore::Incidence::Ptr> existingIncidences = m_incidenceMap.values(incidence->instanceIdentifier());

        if (existingIncidences.count() == 1)
            continue;

        foreach (const KCalCore::Incidence::Ptr &existingIncidence, existingIncidences) {
            if (existingIncidence != incidence && *incidence == *existingIncidence) {
                printFound(item);
                deleteIncidence(item);
                break;
            }
        }
    }

    foreach (const Akonadi::Item &item, m_itemsToProcess) {
        KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        QList<KCalCore::Incidence::Ptr> existingIncidences = m_incidenceMap.values(incidence->instanceIdentifier());

        if (existingIncidences.count() == 1)
            continue;

        for (int i=1; i<existingIncidences.count(); ++i) {
            printFound(item);
            if (m_fixingEnabled) {
                KCalCore::Incidence::Ptr existingIncidence = existingIncidences.at(i);
                Akonadi::Item item = m_incidenceToItem.value(existingIncidence);
                Q_ASSERT(item.isValid());
                if (item.isValid()) {
                    existingIncidence->recreate();
                    m_changer->modifyIncidence(item);
                    m_pendingModifications++;
                    m_incidenceMap.remove(incidence->instanceIdentifier(), existingIncidence);
                }
            }
        }
    }

    endTest();
}

static void printStat(const QString &message, int arg)
{
    if (arg > 0) {
        print(message.leftJustified(50), false);
        const QString s = QLatin1String(": %1");
        print(s.arg(arg));
    }
}

void CalendarJanitor::sanityCheck8()
{
    beginTest(i18n("Gathering statistics..."));
    print(QLatin1String("\n"));

    int numOldAlarms = 0;
    int numAttachments = 0;
    int totalAttachmentSize = 0;
    int numOldIncidences = 0;
    QHash<KCalCore::Incidence::IncidenceType, int> m_counts;

    foreach (const Akonadi::Item &item, m_itemsToProcess) {
        KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        if (!incidence->attachments().isEmpty()) {
            foreach (const KCalCore::Attachment::Ptr &attachment, incidence->attachments()) {
                if (!attachment->isUri()) {
                    numAttachments++;
                    totalAttachmentSize += attachment->size();
                }
            }
        }

        m_counts[incidence->type()]++;

        if (incidenceIsOld(incidence)) {
            if (!incidence->alarms().isEmpty())
                numOldAlarms++;
            numOldIncidences++;
        }

        numAttachments += incidence->attachments().count();
    }

    printStat(i18n("Events"), m_counts[KCalCore::Incidence::TypeEvent]);
    printStat(i18n("Todos"), m_counts[KCalCore::Incidence::TypeTodo]);
    printStat(i18n("Journals"), m_counts[KCalCore::Incidence::TypeJournal]);
    printStat(i18n("Passed events and to-dos (>365 days)"), numOldIncidences);
    printStat(i18n("Old incidences with alarms"), numOldAlarms);
    printStat(i18n("Inline attachments"), numAttachments);

    if (totalAttachmentSize < 1024) {
        printStat(i18n("Total size of inline attachments (bytes)"), totalAttachmentSize);
    } else {
        printStat(i18n("Total size of inline attachments (KB)"), totalAttachmentSize / 1024);
    }

    endTest(/**print=*/false);
}

void CalendarJanitor::stripOldAlarms()
{
    beginTest(i18n("Deleting alarms older than 365 days..."));

    foreach (const Akonadi::Item &item, m_itemsToProcess) {
        KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        if (!incidence->alarms().isEmpty() && incidenceIsOld(incidence)) {
            incidence->clearAlarms();
            m_pendingModifications++;
            m_changer->modifyIncidence(item);
        }
    }

    endTest();
}

static QString dateString(const KCalCore::Incidence::Ptr &incidence)
{
    KDateTime start = incidence->dtStart();
    KDateTime end = incidence->dateTime(KCalCore::Incidence::RoleEnd);
    QString str = QLatin1String("DTSTART=") + (start.isValid() ? start.toString() : i18n("invalid")) + QLatin1String("; ");

    if (incidence->type() == KCalCore::Incidence::TypeJournal) {
        return str;
    }

    str += QLatin1String("\n        ");

    if (incidence->type() == KCalCore::Incidence::TypeTodo)
        str += QLatin1String("DTDUE=");
    else if (incidence->type() == KCalCore::Incidence::TypeEvent)
        str += QLatin1String("DTEND=");

    str+= (start.isValid() ? end.toString() : i18n("invalid")) + QLatin1String("; ");

    if (incidence->recurs())
        str += i18n("recurrent");

    return str;
}

void CalendarJanitor::printFound(const Akonadi::Item &item)
{
    KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
    m_numDamaged++;
    if (m_numDamaged == 1)
        print(QLatin1String(" [!!]"));
    print(QLatin1String("    * ") + i18n("Found buggy item:"));
    print(QLatin1String("        ") + i18n("id=%1; summary=\"%2\"", item.id(), incidence->summary()));
    print(QLatin1String("        ") + dateString(incidence));
}

void CalendarJanitor::beginTest(const QString &message)
{
    m_numDamaged = 0;
    m_fixingEnabled = m_options.action() == Options::ActionScanAndFix && !collectionIsReadOnly(m_currentCollection);
    print(message.leftJustified(TEXT_WIDTH), false);
}

void CalendarJanitor::endTest(bool printEnabled)
{
    if (m_numDamaged == 0 && printEnabled) {
        print(QLatin1String(" [OK]"));
    }

    if (m_pendingDeletions == 0 && m_pendingModifications == 0) {
        runNextTest();
    }
}

void CalendarJanitor::deleteIncidence(const Akonadi::Item &item)
{
    if (m_fixingEnabled && !collectionIsReadOnly(m_currentCollection)) {
        m_pendingDeletions++;
        m_changer->deleteIncidence(item);
        KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        m_incidenceMap.remove(incidence->instanceIdentifier(), incidence);
        m_incidenceToItem.remove(incidence);
    }
}
