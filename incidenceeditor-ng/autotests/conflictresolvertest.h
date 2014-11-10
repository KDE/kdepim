/*
  Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
  Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#ifndef CONFLICTRESOLVERTEST_H
#define CONFLICTRESOLVERTEST_H

#include "../freebusyitem.h"

#include <KCalCore/FreeBusy>
#include <KCalCore/Attendee>

#include <QObject>

namespace IncidenceEditorNG
{
class ConflictResolver;
}

class ConflictResolverTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void cleanup();
    void simpleTest();
    void stillPrettySimpleTest();
    void akademy2010();
    void testPeriodBeginsBeforeTimeframeBegins();
    void testPeriodEndsAfterTimeframeEnds();
    void testPeriodIsLargerThenTimeframe();
    void testPeriodEndsAtSametimeAsTimeframe();

private:
    void insertAttendees();
    void addAttendee(const QString &email, const KCalCore::FreeBusy::Ptr &fb,
                     KCalCore::Attendee::Role role = KCalCore::Attendee::ReqParticipant) ;
    QList<IncidenceEditorNG::FreeBusyItem::Ptr> attendees;
    QWidget *parent;
    IncidenceEditorNG::ConflictResolver *resolver;
    KDateTime base, end;
};

#endif
