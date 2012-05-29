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

#ifndef CONFLICT_PREVENTER_H_
#define CONFLICT_PREVENTER_H_

#include <qstring.h>

namespace KCal {
  class Incidence;
}

/**
 * Class to try to avoid false positive conflicts, as the one reported in issue4826.
 *
 * Here's the use-case:
 *
 * 1. userA changes an event; syncs
 * 2. userB changes the same event; syncs ( userB has write access to userA's folder. )
 * 3. userB gets conflict dialog and chooses "keep theirs".
 *    As of 2012, "keep theirs" will create a new object on the server ( to avoid dataloss )
 *    userA doesn't know about this new object.
 * 4. userA changes the event again; syncs
 *    userA gets a conflict dialog! This conflict dialog unecessary, because although userA
 *    doesn't know about the object userB created, the event that userA had, had the same contents has
 *    said new object.
 *    This is where the conflict-preventer comes in, it keeps track of payloads from userA, and when
 *    fromKMailAddIncidence() is called, we can see if it's a duplicated payload, and just discard it
 *    instead of showing the conflict dialog.
 */
class ConflictPreventer {

public:
  ConflictPreventer();
  ~ConflictPreventer();

  /**
   * @param incidence original payload, before a change.
   * @param subresource the folder this incidence belongs too
   * This method clones the incidence internally, ownership is not taken.
   */
  void registerOldPayload( KCal::Incidence *incidence, const QString &subresource );

  /**
   * Checks if a payload is registered.
   */
  bool isRegistered( KCal::Incidence *incidence, const QString &subresource ) const;

  /**
   * Returns true if the payload is equal to an old payload, and marks it as a false positive
   * conflict.
   *
   * Used by ResourceKolab::fromKMailAddIncidence() to ignore the addition, and trigger a deletion
   * of the dummy item.
   */
  bool processNewPayload( KCal::Incidence *incidence, const QString &resource,
                          Q_INT32 sernum );

  /**
   * Returns true if a message is a false positive.
   * Used by ResourceKolab::fromKMailDeleteIncidence() to ignore the deletion.
   */
  bool isFalsePositive( const QString &resource, Q_INT32 sernum ) const;

  void cleanup( const QString &uid, const QString &resource = QString(), Q_INT32 sernum = 0 );

private:
  class Private;
  Private *const d;
};

#endif
