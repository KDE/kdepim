/*
    This file is part of libkdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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
*/

#ifndef KSYNC_CALENDARDIFFALGO_H
#define KSYNC_CALENDARDIFFALGO_H

#include <kcal/event.h>
#include <kcal/todo.h>
#include <libkdepim/diffalgo.h>

using namespace KPIM;

namespace KSync {

class CalendarDiffAlgo : public DiffAlgo
{
  public:
    CalendarDiffAlgo( KCal::Incidence *leftIncidence, KCal::Incidence *rightIncidence );

    void run();

  private:
    template <class L>
    void diffList( const QString &id, const QList<L> &left, const QList<L> &right );

    void diffIncidenceBase( KCal::IncidenceBase *inc1,
                            KCal::IncidenceBase *inc2 );
    void diffIncidence( KCal::Incidence *inc1, KCal::Incidence *inc2 );
    void diffEvent( KCal::Event *event1, KCal::Event *event2 );
    void diffTodo( KCal::Todo *todo1, KCal::Todo *todo2 );

    KCal::Incidence *mLeftIncidence;
    KCal::Incidence *mRightIncidence;
};

}

#endif
