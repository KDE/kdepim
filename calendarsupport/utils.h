/*
  Copyright (C) 2009 KDAB (author: Frank Osterfeld <osterfeld@kde.org>)

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
#ifndef CALENDARSUPPORT_UTILS_H
#define CALENDARSUPPORT_UTILS_H

#include "calendarsupport_export.h"

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <KCalCore/Event>
#include <KCalCore/Incidence>
#include <KCalCore/Journal>
#include <KCalCore/Todo>

#include <KDateTime>

#include <QModelIndex>

namespace KPIMIdentities {
class IdentityManager;
}

namespace KCalCore {
  class CalFilter;
}

class QAbstractItemModel;
class QDrag;
class QMimeData;

typedef QList<QModelIndex> QModelIndexList;

namespace CalendarSupport
{
  /**
   * returns the incidence from an akonadi item, or a null pointer if the item has no such payload
   */
  CALENDARSUPPORT_EXPORT KCalCore::Incidence::Ptr incidence( const Akonadi::Item &item );

  /**
   * returns the event from an akonadi item, or a null pointer if the item has no such payload
   */
  CALENDARSUPPORT_EXPORT KCalCore::Event::Ptr event( const Akonadi::Item &item );

  /**
   * returns event pointers from an akonadi item, or a null pointer if the item has no such payload
   */
  CALENDARSUPPORT_EXPORT KCalCore::Event::List eventsFromItems(
    const Akonadi::Item::List &items );

  /**
   * returns incidence pointers from an akonadi item.
   */
  CALENDARSUPPORT_EXPORT KCalCore::Incidence::List incidencesFromItems(
    const Akonadi::Item::List &items );


  /**
  * returns the todo from an akonadi item, or a null pointer if the item has no such payload
  */
  CALENDARSUPPORT_EXPORT KCalCore::Todo::Ptr todo( const Akonadi::Item &item );

  /**
  * returns the journal from an akonadi item, or a null pointer if the item has no such payload
  */
  CALENDARSUPPORT_EXPORT KCalCore::Journal::Ptr journal( const Akonadi::Item &item );

  /**
   * returns whether an Akonadi item contains an incidence
   */
  CALENDARSUPPORT_EXPORT bool hasIncidence( const Akonadi::Item &item );

  /**
   * returns whether an Akonadi item contains an event
   */
  CALENDARSUPPORT_EXPORT bool hasEvent( const Akonadi::Item &item );

  /**
   * returns whether an Akonadi item contains a todo
   */
  CALENDARSUPPORT_EXPORT bool hasTodo( const Akonadi::Item &item );

  /**
   * returns whether an Akonadi item contains a journal
   */
  CALENDARSUPPORT_EXPORT bool hasJournal( const Akonadi::Item &item );

  /**
   * returns whether this item can be deleted
   */
  CALENDARSUPPORT_EXPORT bool hasDeleteRights( const Akonadi::Item &item );

  /**
   * returns whether this item can be changed
   */
  CALENDARSUPPORT_EXPORT bool hasChangeRights( const Akonadi::Item &item );

  /**
  * returns @p true if the URL represents an Akonadi item and has one of the given mimetypes.
  */
  CALENDARSUPPORT_EXPORT bool isValidIncidenceItemUrl( const KUrl &url,
                                                       const QStringList &supportedMimeTypes );

  CALENDARSUPPORT_EXPORT bool isValidIncidenceItemUrl( const KUrl &url );

  /**
  * returns @p true if the mime data object contains any of the following:
  *
  * * An akonadi item with a supported KCal mimetype
  * * an iCalendar
  * * a VCard
  */
  CALENDARSUPPORT_EXPORT bool canDecode( const QMimeData *mimeData );

  CALENDARSUPPORT_EXPORT QList<KUrl> incidenceItemUrls( const QMimeData *mimeData );

  CALENDARSUPPORT_EXPORT QList<KUrl> todoItemUrls( const QMimeData *mimeData );

  CALENDARSUPPORT_EXPORT bool mimeDataHasTodo( const QMimeData *mimeData );

  CALENDARSUPPORT_EXPORT KCalCore::Todo::List todos( const QMimeData *mimeData,
                                                     const KDateTime::Spec &timeSpec );

  /**
  * returns @p true if the URL represents an Akonadi item and has one of the given mimetypes.
  */
  CALENDARSUPPORT_EXPORT bool isValidTodoItemUrl( const KUrl &url );

  /**
  * creates mime data object for dragging an akonadi item containing an incidence
  */
  CALENDARSUPPORT_EXPORT QMimeData *createMimeData( const Akonadi::Item &item,
                                                    const KDateTime::Spec &timeSpec );

  /**
  * creates mime data object for dragging akonadi items containing an incidence
  */
  CALENDARSUPPORT_EXPORT QMimeData *createMimeData( const Akonadi::Item::List &items,
                                                    const KDateTime::Spec &timeSpec );

#ifndef QT_NO_DRAGANDDROP
  /**
  * creates a drag object for dragging an akonadi item containing an incidence
  */
  CALENDARSUPPORT_EXPORT QDrag *createDrag( const Akonadi::Item &item,
                                            const KDateTime::Spec &timeSpec, QWidget *parent );

  /**
  * creates a drag object for dragging akonadi items containing an incidence
  */
  CALENDARSUPPORT_EXPORT QDrag *createDrag( const Akonadi::Item::List &items,
                                            const KDateTime::Spec &timeSpec, QWidget *parent );
#endif
  /**
    Applies a filter to a list of items containing incidences.
    Items not containing incidences or not matching the filter are removed.
    Helper method anologous to KCalCore::CalFilter::apply()
    @see KCalCore::CalFilter::apply()
    @param items the list of items to filter
    @param filter the filter to apply to the list of items
    @return the filtered list of items
  */
  CALENDARSUPPORT_EXPORT Akonadi::Item::List applyCalFilter( const Akonadi::Item::List &items,
                                                             const KCalCore::CalFilter *filter );

  /**
    Shows a modal dialog that allows to select a collection.

    @param will contain the dialogCode, QDialog::Accepted if the user pressed Ok,
    QDialog::Rejected otherwise
    @param parent The optional parent of the modal dialog.
    @return The select collection or an invalid collection if
    there was no collection selected.
  */
  CALENDARSUPPORT_EXPORT Akonadi::Collection selectCollection(
    QWidget *parent, int &dialogCode,
    const QStringList &mimeTypes,
    const Akonadi::Collection &defaultCollection = Akonadi::Collection() );

  CALENDARSUPPORT_EXPORT Akonadi::Item itemFromIndex( const QModelIndex &index );

  CALENDARSUPPORT_EXPORT Akonadi::Item::List itemsFromModel(
    const QAbstractItemModel *model,
    const QModelIndex &parentIndex = QModelIndex(),
    int start = 0,
    int end = -1 );

  CALENDARSUPPORT_EXPORT Akonadi::Collection::List collectionsFromModel(
    const QAbstractItemModel *model,
    const QModelIndex &parentIndex = QModelIndex(),
    int start = 0,
    int end = -1 );

  CALENDARSUPPORT_EXPORT Akonadi::Collection collectionFromIndex( const QModelIndex &index );

  CALENDARSUPPORT_EXPORT Akonadi::Collection::Id collectionIdFromIndex( const QModelIndex &index );

  CALENDARSUPPORT_EXPORT Akonadi::Collection::List collectionsFromIndexes(
    const QModelIndexList &indexes );

  CALENDARSUPPORT_EXPORT QString displayName( const Akonadi::Collection &coll );

  CALENDARSUPPORT_EXPORT QString subMimeTypeForIncidence(
    const KCalCore::Incidence::Ptr &incidence );

  /**
      Returns a list containing work days between @p start and @end.
  */
  CALENDARSUPPORT_EXPORT QList<QDate> workDays( const QDate &start, const QDate &end );

  /**
    Returns a list of holidays that occur at @param date.
  */
  CALENDARSUPPORT_EXPORT QStringList holiday( const QDate &date );

  CALENDARSUPPORT_EXPORT void sendAsICalendar( const Akonadi::Item& item, KPIMIdentities::IdentityManager *identityManager, QWidget* parentWidget = 0 );

}

#endif
