/* Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
   Author: Sérgio Martins <sergio.martins@kdab.com>

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
#ifndef MEMORYCALENDARMEMENTO_H
#define MEMORYCALENDARMEMENTO_H

#include <messageviewer/interfaces/bodypart.h>

#include <messageviewer/viewer.h>

#include <KCalCore/MemoryCalendar>
#include <KUrl>

#include <QObject>

class KJob;

namespace CalendarSupport {
  class IncidenceFetchJob;
}

namespace MessageViewer {

class MemoryCalendarMemento : public QObject, public Interface::BodyPartMemento
{
  Q_OBJECT
  public:
    MemoryCalendarMemento();

    bool finished() const;
    KCalCore::MemoryCalendar::Ptr calendar() const;

    virtual void detach();

  signals:
    // TODO: Factor our update and detach into base class
    void update( MessageViewer::Viewer::UpdateMode );

  private slots:
    void slotSearchJobFinished( KJob *job );

  private:
    bool mFinished;
    KCalCore::MemoryCalendar::Ptr mCalendar;
};

}

#endif
