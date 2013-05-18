/*
  Copyright (c) 2009, 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Author: Frank Osterfeld <osterfeld@kde.org>
    Author: Andras Mantia <andras@kdab.com>

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

#include "utils.h"
#include "kcalprefs.h"

#include <Akonadi/Collection>
#include <Akonadi/CollectionDialog>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/Item>
#include <Akonadi/Calendar/ETMCalendar>
#include <akonadi/calendar/publishdialog.h>
#include <akonadi/calendar/calendarsettings.h>

#include <KHolidays/Holidays>

#include <KCalCore/CalFilter>
#include <KCalCore/Event>
#include <KCalCore/FreeBusy>
#include <KCalCore/Incidence>
#include <KCalCore/Journal>
#include <KCalCore/MemoryCalendar>
#include <KCalCore/Todo>
#include <KCalCore/ICalFormat>
#include <KCalCore/FileStorage>

#include <KCalUtils/DndFactory>
#include <KCalUtils/ICalDrag>
#include <KCalUtils/VCalDrag>

#include <Mailtransport/TransportManager>

#include <KIconLoader>
#include <KUrl>

#include <QAbstractItemModel>
#include <QDrag>
#include <QMimeData>
#include <QModelIndex>
#include <QPixmap>
#include <QPointer>

#include <boost/bind.hpp>
#include <KMessageBox>
#include <KPIMIdentities/IdentityManager>
#include <KFileDialog>
#include <KIO/NetAccess>

using namespace CalendarSupport;
using namespace KHolidays;
using namespace KCalCore;

KCalCore::Incidence::Ptr CalendarSupport::incidence( const Akonadi::Item &item )
{
  //relying on exception for performance reasons
  try {
    return item.payload<KCalCore::Incidence::Ptr>();
  } catch( Akonadi::PayloadException ) {
    return KCalCore::Incidence::Ptr();
  }
}

KCalCore::Event::Ptr CalendarSupport::event( const Akonadi::Item &item )
{
  //relying on exception for performance reasons
  try {
    KCalCore::Incidence::Ptr incidence = item.payload<KCalCore::Incidence::Ptr>();
    if ( incidence && incidence->type() == KCalCore::Incidence::TypeEvent ) {
      return item.payload<KCalCore::Event::Ptr>();
    }
  } catch( Akonadi::PayloadException ) {
    return KCalCore::Event::Ptr();
  }
  return KCalCore::Event::Ptr();
}

KCalCore::Event::List CalendarSupport::eventsFromItems( const Akonadi::Item::List &items )
{
  KCalCore::Event::List events;
  Q_FOREACH ( const Akonadi::Item &item, items ) {
    if ( const KCalCore::Event::Ptr e = CalendarSupport::event( item ) ) {
      events.push_back( e );
    }
  }
  return events;
}

KCalCore::Incidence::List CalendarSupport::incidencesFromItems( const Akonadi::Item::List &items )
{
  KCalCore::Incidence::List incidences;
  Q_FOREACH ( const Akonadi::Item &item, items ) {
    if ( const KCalCore::Incidence::Ptr e = CalendarSupport::incidence( item ) ) {
      incidences.push_back( e );
    }
  }
  return incidences;
}

KCalCore::Todo::Ptr CalendarSupport::todo( const Akonadi::Item &item )
{
  try {
    KCalCore::Incidence::Ptr incidence = item.payload<KCalCore::Incidence::Ptr>();
    if ( incidence && incidence->type() == KCalCore::Incidence::TypeTodo ) {
      return item.payload<KCalCore::Todo::Ptr>();
    }
  } catch( Akonadi::PayloadException ) {
    return KCalCore::Todo::Ptr();
  }
  return KCalCore::Todo::Ptr();
}

KCalCore::Journal::Ptr CalendarSupport::journal( const Akonadi::Item &item )
{
  try {
    KCalCore::Incidence::Ptr incidence = item.payload<KCalCore::Incidence::Ptr>();
    if ( incidence && incidence->type() == KCalCore::Incidence::TypeJournal ) {
      return item.payload<KCalCore::Journal::Ptr>();
    }
  } catch( Akonadi::PayloadException ) {
    return KCalCore::Journal::Ptr();
  }
  return KCalCore::Journal::Ptr();
}

bool CalendarSupport::hasIncidence( const Akonadi::Item &item )
{
  return item.hasPayload<KCalCore::Incidence::Ptr>();
}

bool CalendarSupport::hasEvent( const Akonadi::Item &item )
{
  return item.hasPayload<KCalCore::Event::Ptr>();
}

bool CalendarSupport::hasTodo( const Akonadi::Item &item )
{
  return item.hasPayload<KCalCore::Todo::Ptr>();
}

bool CalendarSupport::hasJournal( const Akonadi::Item &item )
{
  return item.hasPayload<KCalCore::Journal::Ptr>();
}

QMimeData *CalendarSupport::createMimeData( const Akonadi::Item::List &items,
                                            const KDateTime::Spec &timeSpec )
{
  if ( items.isEmpty() ) {
    return 0;
  }

  KCalCore::MemoryCalendar::Ptr cal( new KCalCore::MemoryCalendar( timeSpec ) );

  QList<QUrl> urls;
  int incidencesFound = 0;
  Q_FOREACH ( const Akonadi::Item &item, items ) {
    const KCalCore::Incidence::Ptr incidence( CalendarSupport::incidence( item ) );
    if ( !incidence ) {
      continue;
    }
    ++incidencesFound;
    urls.push_back( item.url() );
    KCalCore::Incidence::Ptr i( incidence->clone() );
    cal->addIncidence( i );
  }

  if ( incidencesFound == 0 ) {
    return 0;
  }

  std::auto_ptr<QMimeData> mimeData( new QMimeData );

  mimeData->setUrls( urls );

  KCalUtils::ICalDrag::populateMimeData( mimeData.get(), cal );
  KCalUtils::VCalDrag::populateMimeData( mimeData.get(), cal );

  return mimeData.release();
}

QMimeData *CalendarSupport::createMimeData( const Akonadi::Item &item,
                                            const KDateTime::Spec &timeSpec )
{
  return createMimeData( Akonadi::Item::List() << item, timeSpec );
}

#ifndef QT_NO_DRAGANDDROP
QDrag *CalendarSupport::createDrag( const Akonadi::Item &item,
                                    const KDateTime::Spec &timeSpec, QWidget *parent )
{
  return createDrag( Akonadi::Item::List() << item, timeSpec, parent );
}
#endif

static QByteArray findMostCommonType( const Akonadi::Item::List &items )
{
  QByteArray prev;
  if ( items.isEmpty() ) {
    return "Incidence";
  }

  Q_FOREACH ( const Akonadi::Item &item, items ) {
    if ( !CalendarSupport::hasIncidence( item ) ) {
      continue;
    }
    const QByteArray type = CalendarSupport::incidence( item )->typeStr();
    if ( !prev.isEmpty() && type != prev ) {
      return "Incidence";
    }
    prev = type;
  }
  return prev;
}

#ifndef QT_NO_DRAGANDDROP
QDrag *CalendarSupport::createDrag( const Akonadi::Item::List &items,
                                    const KDateTime::Spec &timeSpec, QWidget *parent )
{
  std::auto_ptr<QDrag> drag( new QDrag( parent ) );
  drag->setMimeData( CalendarSupport::createMimeData( items, timeSpec ) );

  const QByteArray common = findMostCommonType( items );
  if ( common == "Event" ) {
    drag->setPixmap( BarIcon( QLatin1String( "view-calendar-day" ) ) );
  } else if ( common == "Todo" ) {
    drag->setPixmap( BarIcon( QLatin1String( "view-calendar-tasks" ) ) );
  }

  return drag.release();
}
#endif

static bool itemMatches( const Akonadi::Item &item, const KCalCore::CalFilter *filter )
{
  assert( filter );
  KCalCore::Incidence::Ptr inc = CalendarSupport::incidence( item );
  if ( !inc ) {
    return false;
  }
  return filter->filterIncidence( inc );
}

Akonadi::Item::List CalendarSupport::applyCalFilter( const Akonadi::Item::List &items_,
                                                     const KCalCore::CalFilter *filter )
{
  Q_ASSERT( filter );
  Akonadi::Item::List items( items_ );
  items.erase( std::remove_if( items.begin(), items.end(),
                               !bind( itemMatches, _1, filter ) ), items.end() );
  return items;
}

bool CalendarSupport::isValidIncidenceItemUrl( const KUrl &url,
                                               const QStringList &supportedMimeTypes )
{
  if ( !url.isValid() ) {
    return false;
  }

  if ( url.scheme() != QLatin1String( "akonadi" ) ) {
    return false;
  }

  return supportedMimeTypes.contains( url.queryItem( QLatin1String( "type" ) ) );
}

bool CalendarSupport::isValidIncidenceItemUrl( const KUrl &url )
{
  return isValidIncidenceItemUrl( url,
                                  QStringList() << KCalCore::Event::eventMimeType()
                                                << KCalCore::Todo::todoMimeType()
                                                << KCalCore::Journal::journalMimeType()
                                                << KCalCore::FreeBusy::freeBusyMimeType() );
}

static bool containsValidIncidenceItemUrl( const QList<QUrl>& urls )
{
  return
    std::find_if( urls.begin(), urls.end(),
                  bind( CalendarSupport::isValidIncidenceItemUrl, _1 ) ) != urls.constEnd();
}

bool CalendarSupport::isValidTodoItemUrl( const KUrl &url )
{
  if ( !url.isValid() || url.scheme() != QLatin1String( "akonadi" ) ) {
    return false;
  }

  return url.queryItem( QLatin1String( "type" ) ) == KCalCore::Todo::todoMimeType();
}

bool CalendarSupport::canDecode( const QMimeData *md )
{
  Q_ASSERT( md );
  return
    containsValidIncidenceItemUrl( md->urls() ) ||
    KCalUtils::ICalDrag::canDecode( md ) ||
    KCalUtils::VCalDrag::canDecode( md );
}

QList<KUrl> CalendarSupport::incidenceItemUrls( const QMimeData *mimeData )
{
  QList<KUrl> urls;
  Q_FOREACH ( const KUrl &i, mimeData->urls() ) {
    if ( isValidIncidenceItemUrl( i ) ) {
      urls.push_back( i );
    }
  }
  return urls;
}

QList<KUrl> CalendarSupport::todoItemUrls( const QMimeData *mimeData )
{
  QList<KUrl> urls;

  Q_FOREACH ( const KUrl &i, mimeData->urls() ) {
    if ( isValidIncidenceItemUrl( i, QStringList() << KCalCore::Todo::todoMimeType() ) ) {
      urls.push_back( i );
    }
  }
  return urls;
}

bool CalendarSupport::mimeDataHasTodo( const QMimeData *mimeData )
{
  return !todoItemUrls( mimeData ).isEmpty() || !todos( mimeData, KDateTime::Spec() ).isEmpty();
}

bool CalendarSupport::mimeDataHasIncidence( const QMimeData *mimeData )
{
  return !incidenceItemUrls( mimeData ).isEmpty() ||
         !incidences( mimeData, KDateTime::Spec() ).isEmpty();
}

KCalCore::Todo::List CalendarSupport::todos( const QMimeData *mimeData,
                                             const KDateTime::Spec &spec )
{
  KCalCore::Todo::List todos;

#ifndef QT_NO_DRAGANDDROP
  KCalCore::Calendar::Ptr cal( KCalUtils::DndFactory::createDropCalendar( mimeData, spec ) );
  if ( cal ) {
    Q_FOREACH ( const KCalCore::Todo::Ptr &i, cal->todos() ) {
      todos.push_back( KCalCore::Todo::Ptr( i->clone() ) );
    }
  }
#endif

  return todos;
}

KCalCore::Incidence::List CalendarSupport::incidences( const QMimeData *mimeData,
                                                       const KDateTime::Spec &spec )
{
  KCalCore::Incidence::List incidences;

#ifndef QT_NO_DRAGANDDROP
  KCalCore::Calendar::Ptr cal( KCalUtils::DndFactory::createDropCalendar( mimeData, spec ) );
  if ( cal ) {
    KCalCore::Incidence::List calIncidences = cal->incidences();
    Q_FOREACH ( const KCalCore::Incidence::Ptr &i, calIncidences ) {
      incidences.push_back( KCalCore::Incidence::Ptr( i->clone() ) );
    }
  }
#endif

  return incidences;
}

Akonadi::Collection CalendarSupport::selectCollection( QWidget *parent,
                                                       int &dialogCode,
                                                       const QStringList &mimeTypes,
                                                       const Akonadi::Collection &defCollection )
{
  QPointer<Akonadi::CollectionDialog> dlg( new Akonadi::CollectionDialog( parent ) );
  dlg->setCaption( i18n( "Select Calendar" ) );
  dlg->setDescription( i18n( "Select the calendar where this item will be stored." ) );
  dlg->changeCollectionDialogOptions( Akonadi::CollectionDialog::KeepTreeExpanded );
  kDebug() << "selecting collections with mimeType in " << mimeTypes;

  dlg->setMimeTypeFilter( mimeTypes );
  dlg->setAccessRightsFilter( Akonadi::Collection::CanCreateItem );
  if ( defCollection.isValid() ) {
    dlg->setDefaultCollection( defCollection );
  }
  Akonadi::Collection collection;

  // FIXME: don't use exec.
  dialogCode = dlg->exec();
  if ( dlg && dialogCode == QDialog::Accepted ) {
    collection = dlg->selectedCollection();

    if ( !collection.isValid() ) {
      kWarning() << "An invalid collection was selected!";
    }
  }
  delete dlg;
  return collection;
}

Akonadi::Item CalendarSupport::itemFromIndex( const QModelIndex &idx )
{
  Akonadi::Item item = idx.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  item.setParentCollection(
    idx.data( Akonadi::EntityTreeModel::ParentCollectionRole ).value<Akonadi::Collection>() );
  return item;
}

Akonadi::Collection::List CalendarSupport::collectionsFromModel( const QAbstractItemModel *model,
                                                                 const QModelIndex &parentIndex,
                                                                 int start, int end )
{
  const int endRow = end >= 0 ? end : model->rowCount( parentIndex ) - 1;
  Akonadi::Collection::List collections;
  int row = start;
  QModelIndex i = model->index( row, 0, parentIndex );
  while ( row <= endRow ) {
    const Akonadi::Collection collection = collectionFromIndex( i );
    if ( collection.isValid() ) {
      collections << collection;
      QModelIndex childIndex = i.child( 0, 0 );
      if ( childIndex.isValid() ) {
        collections << collectionsFromModel( model, i );
      }
    }
    ++row;
    i = i.sibling( row, 0 );
  }
  return collections;
}

Akonadi::Item::List CalendarSupport::itemsFromModel( const QAbstractItemModel * model,
                                                     const QModelIndex &parentIndex,
                                                     int start, int end )
{
  const int endRow = end >= 0 ? end : model->rowCount( parentIndex ) - 1;
  Akonadi::Item::List items;
  int row = start;
  QModelIndex i = model->index( row, 0, parentIndex );
  while ( row <= endRow ) {
    const Akonadi::Item item = itemFromIndex( i );
    if ( CalendarSupport::hasIncidence( item ) ) {
      items << item;
    } else {
      QModelIndex childIndex = i.child( 0, 0 );
      if ( childIndex.isValid() ) {
        items << itemsFromModel( model, i );
      }
    }
    ++row;
    i = i.sibling( row, 0 );
  }
  return items;
}

Akonadi::Collection CalendarSupport::collectionFromIndex( const QModelIndex &index )
{
  return index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
}

Akonadi::Collection::Id CalendarSupport::collectionIdFromIndex( const QModelIndex &index )
{
  return index.data( Akonadi::EntityTreeModel::CollectionIdRole ).value<Akonadi::Collection::Id>();
}

Akonadi::Collection::List CalendarSupport::collectionsFromIndexes( const QModelIndexList &indexes )
{
  Akonadi::Collection::List l;
  Q_FOREACH ( const QModelIndex &idx, indexes ) {
    l.push_back( collectionFromIndex( idx ) );
  }
  return l;
}

QString CalendarSupport::displayName( Akonadi::ETMCalendar *calendar, const Akonadi::Collection &c )
{
  QString cName = c.name();
  if ( cName.isEmpty() && calendar ) {
    Akonadi::Collection realCollection = calendar->collection( c.id() );
    if ( realCollection.isValid() ) {
      cName = realCollection.name();
    }
  }

  // Kolab Groupware
  if ( c.resource().contains( "kolabproxy" ) ) {
    QString typeStr = cName; // contents type: "Calendar", "Tasks", etc
    QString ownerStr;        // folder owner: "fred", "ethel", etc
    QString nameStr;         // folder name: "Public", "Test", etc
    if ( calendar ) {
      Akonadi::Collection p = c.parentCollection();
      while ( p != Akonadi::Collection::root() ) {
        Akonadi::Collection tCol = calendar->collection( p.id() );
        const QString tName = tCol.name();
        if ( tName.toLower().startsWith( QLatin1String( "shared.cal" ) ) ) {
          ownerStr = "Shared";
          nameStr = cName;
          typeStr = "Calendar";
          break;
        } else if ( tName.toLower().startsWith( QLatin1String( "shared.tasks" ) ) ||
                    tName.toLower().startsWith( QLatin1String( "shared.todo" ) ) ) {
          ownerStr = "Shared";
          nameStr = cName;
          typeStr = "Tasks";
          break;
        } else if ( tName.toLower().startsWith( QLatin1String( "shared.journal" ) ) ) {
          ownerStr = "Shared";
          nameStr = cName;
          typeStr = "Journal";
          break;
        } else if ( tName.toLower().startsWith( QLatin1String( "shared.notes" ) ) ) {
          ownerStr = "Shared";
          nameStr = cName;
          typeStr = "Notes";
          break;
        } else if ( tName != i18n( "Calendar" ) &&
                    tName != i18n( "Tasks" ) &&
                    tName != i18n( "Journal" ) &&
                    tName != i18n( "Notes" ) ) {
          ownerStr = tName;
          break;
        } else {
          nameStr = typeStr;
          typeStr = tName;
        }
        p = p.parentCollection();
      }
    }

    if ( !ownerStr.isEmpty() ) {
      if ( ownerStr.toUpper() == QString( "INBOX" ) ) {
        return i18nc( "%1 is folder contents",
                      "My Kolab %1", typeStr );
      } else if ( ownerStr.toUpper() == QString( "SHARED" ) ) {
        return i18nc( "%1 is folder name, %2 is folder contents",
                      "Shared Kolab %1 %2", nameStr, typeStr );
      } else {
        if ( nameStr.isEmpty() ) {
          return i18nc( "%1 is folder owner name, %2 is folder contents",
                        "%1's Kolab %2", ownerStr, typeStr );
        } else {
          return i18nc( "%1 is folder owner name, %2 is folder name, %3 is folder contents",
                        "%1's %2 Kolab %3", ownerStr, nameStr, typeStr );
        }
      }
    } else {
      return i18nc( "%1 is folder contents",
                    "Kolab %1", typeStr );
    }
  } //end kolab section

  // Dav Groupware
  if ( c.resource().contains( "davgroupware" ) ) {
    return i18nc( "%1 is the folder name", "%1 CalDav Calendar", cName );
  } //end caldav section

  // Google
  if ( c.resource().contains( "google" ) ) {
    QString ownerStr;        // folder owner: "user@gmail.com"
    if ( calendar ) {
      Akonadi::Collection p = c.parentCollection();
      ownerStr = calendar->collection( p.id() ).displayName();
    }

    const QString nameStr = c.displayName(); // folder name: can be anything

    QString typeStr;
    const QString mimeStr = c.contentMimeTypes().join( "," );
    if ( mimeStr.contains( ".event" ) ) {
      typeStr = i18n( "Calendar" );
    } else if ( mimeStr.contains( ".todo" ) ) {
      typeStr = i18n( "Tasks" );
    } else if ( mimeStr.contains( ".journal" ) ) {
      typeStr = i18n( "Journal" );
    } else if ( mimeStr.contains( ".note" ) ) {
      typeStr = i18n( "Notes" );
    } else {
      typeStr = mimeStr;
    }

    if ( !ownerStr.isEmpty() ) {
      const int atChar = ownerStr.lastIndexOf( '@' );
      ownerStr = ownerStr.left( atChar );
      if ( nameStr.isEmpty() ) {
        return i18nc( "%1 is folder owner name, %2 is folder contents",
                      "%1's Google %2", ownerStr, typeStr );
      } else {
        return i18nc( "%1 is folder owner name, %2 is folder name",
                      "%1's %2", ownerStr, nameStr );
      }
    } else {
      return i18nc( "%1 is folder contents",
                    "Google %1", typeStr );
    }
  } //end google section

  // Not groupware so the collection is "mine"
  const QString dName = c.displayName();

  if ( !dName.isEmpty() ) {
    return i18n( "My %1", dName );
  } else {
    return i18nc( "unknown resource", "Unknown" );
  }
}

QString CalendarSupport::subMimeTypeForIncidence( const KCalCore::Incidence::Ptr &incidence )
{
  return incidence->mimeType();
}

QList<QDate> CalendarSupport::workDays( const QDate &startDate,
                                        const QDate &endDate )
{
  QList<QDate> result;

  const int mask( ~( KCalPrefs::instance()->mWorkWeekMask ) );
  const int numDays = startDate.daysTo( endDate ) + 1;

  for ( int i = 0; i < numDays; ++i ) {
    const QDate date = startDate.addDays( i );
    if ( !( mask & ( 1 << ( date.dayOfWeek() - 1 ) ) ) ) {
      result.append( date );
    }
  }

  if ( KCalPrefs::instance()->mExcludeHolidays ) {
    // NOTE: KOGlobals, where this method comes from, used to hold a pointer to
    //       a KHolidays object. I'm not sure about how expensive it is, just
    //       creating one here.
    const HolidayRegion holidays( KCalPrefs::instance()->mHolidays );
    const Holiday::List list = holidays.holidays( startDate, endDate );
    const int listCount( list.count() );
    for ( int i = 0; i < listCount; ++i ) {
      const Holiday &h = list.at( i );
      if ( h.dayType() == Holiday::NonWorkday ) {
        result.removeAll( h.date() );
      }
    }
  }

  return result;
}

QStringList CalendarSupport::holiday( const QDate &date )
{
  QStringList hdays;

  const HolidayRegion holidays( KCalPrefs::instance()->mHolidays );
  const Holiday::List list = holidays.holidays( date );
  const int listCount( list.count() );
  for ( int i = 0; i < listCount; ++i ) {
    hdays.append( list.at( i ).text() );
  }
  return hdays;
}

void CalendarSupport::saveAttachments( const Akonadi::Item &item, QWidget *parentWidget )
{
  Incidence::Ptr incidence = CalendarSupport::incidence( item );

  if ( !incidence ) {
    KMessageBox::sorry(
      parentWidget,
      i18n( "No item selected." ) );
    return;
  }

  Attachment::List attachments = incidence->attachments();

  if ( attachments.empty() ) {
    return;
  }

  QString targetFile, targetDir;
  if ( attachments.count() > 1 ) {
    // get the dir
    targetDir = KFileDialog::getExistingDirectory( KUrl( "kfiledialog:///saveAttachment" ),
                                                   parentWidget,
                                                   i18n( "Save Attachments To" ) );
    if ( targetDir.isEmpty() ) {
      return;
    }

    // we may not get a slash-terminated url out of KFileDialog
    if ( !targetDir.endsWith( '/' ) ) {
      targetDir.append( '/' );
    }
  } else {
    // only one item, get the desired filename
    QString fileName = attachments.first()->label();
    if ( fileName.isEmpty() ) {
      fileName = i18nc( "filename for an unnamed attachment", "attachment.1" );
    }
    targetFile = KFileDialog::getSaveFileName( KUrl( "kfiledialog:///saveAttachment/" + fileName ),
                                   QString(),
                                   parentWidget,
                                   i18n( "Save Attachment" ) );
    if ( targetFile.isEmpty() ) {
      return;
    }

    targetDir = QFileInfo( targetFile ).absolutePath() + '/';
  }

  Q_FOREACH ( Attachment::Ptr attachment, attachments ) {
    targetFile = targetDir + attachment->label();
    KUrl sourceUrl;
    if ( attachment->isUri() ) {
      sourceUrl = attachment->uri();
    } else {
      sourceUrl = incidence->writeAttachmentToTempFile( attachment );
    }
    // save the attachment url
    if ( !KIO::NetAccess::file_copy( sourceUrl, KUrl( targetFile ) ) &&
        KIO::NetAccess::lastError() ) {
      KMessageBox::error( parentWidget, KIO::NetAccess::lastErrorString() );
    }
  }
}

QStringList CalendarSupport::categories( const KCalCore::Incidence::List &incidences )
{
  QStringList cats, thisCats;
  // @TODO: For now just iterate over all incidences. In the future,
  // the list of categories should be built when reading the file.
  Q_FOREACH ( const KCalCore::Incidence::Ptr &incidence, incidences ) {
    thisCats = incidence->categories();
    for ( QStringList::ConstIterator si = thisCats.constBegin();
          si != thisCats.constEnd(); ++si ) {
      if ( !cats.contains( *si ) ) {
        cats.append( *si );
      }
    }
  }
  return cats;
}

bool CalendarSupport::mergeCalendar(const QString &srcFilename, const KCalCore::Calendar::Ptr &destCalendar)
{
    if (srcFilename.isEmpty()) {
        kError() << "Empty filename.";
        return false;
    }

    if (!QFile::exists(srcFilename)) {
        kError() << "File'" << srcFilename << "' doesn't exist.";
    }

    bool loadedSuccesfully = true;

    // merge in a file
    destCalendar->startBatchAdding();
    KCalCore::FileStorage storage(destCalendar);
    storage.setFileName(srcFilename);
    loadedSuccesfully = storage.load();
    destCalendar->endBatchAdding();

    return loadedSuccesfully;
}
