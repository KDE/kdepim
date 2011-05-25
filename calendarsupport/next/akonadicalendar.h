/*
   Copyright (C) 2011 Sérgio Martins <sergio.martins@kdab.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _CALENDARSUPPORT_AKONADICALENDAR_H_
#define _CALENDARSUPPORT_AKONADICALENDAR_H_

#include <Akonadi/Item>
#include <KCalCore/MemoryCalendar>
#include <KDateTime>

namespace CalendarSupport {

  class AkonadiCalendar : public KCalCore::MemoryCalendar
  {
  Q_OBJECT
  public:

    typedef QSharedPointer<AkonadiCalendar> Ptr;

    explicit AkonadiCalendar( const KDateTime::Spec &timeSpec );
    ~AkonadiCalendar();
    Akonadi::Item item( const QString &uid ) const;
    Akonadi::Item item( Akonadi::Item::Id ) const;

  Q_SIGNALS:
    void loaded();
  
  private:
    class Private;
    Private *const d;
  };
}


#endif