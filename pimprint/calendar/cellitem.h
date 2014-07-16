/*
  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef PIMPRINT_CELLITEM_H
#define PIMPRINT_CELLITEM_H

#include "pimprint_calendar_export.h"

#include <KCalCore/Event>

#include <QList>

namespace PimPrint
{

namespace Calendar
{

class PIMPRINT_CALENDAR_EXPORT CellItem
{
public:
    CellItem()
        : mSubCells(0), mSubCell(-1)
    {
    }

    virtual ~CellItem()
    {
    }

    void setSubCells(int v)
    {
        mSubCells = v;
    }

    int subCells() const
    {
        return mSubCells;
    }

    void setSubCell(int v)
    {
        mSubCell = v;
    }

    int subCell() const
    {
        return mSubCell;
    }

    virtual bool overlaps(CellItem *other) const = 0;

    virtual QString label() const;

    /**
      Place item @p placeItem into stripe containing items @p cells in a
      way that items don't overlap.
      @param cells The list of other cell items to be laid out parallel to the placeItem.
      @param placeItem The item to be laid out.

      @return Placed items
    */
    static QList<CellItem *> placeItem(QList<CellItem *> cells, CellItem *placeItem);

private:
    int mSubCells;
    int mSubCell;
};

class PIMPRINT_CALENDAR_EXPORT PrintCellItem : public CellItem
{
public:
    PrintCellItem(const KCalCore::Event::Ptr &event,
                  const KDateTime &start, const KDateTime &end)
        : mEvent(event), mStart(start), mEnd(end)
    {
    }

    KCalCore::Event::Ptr event() const
    {
        return mEvent;
    }

    QString label() const
    {
        return mEvent->summary();
    }

    KDateTime start() const
    {
        return mStart;
    }

    KDateTime end() const
    {
        return mEnd;
    }

    /** Calculate the start and end date/time of the recurrence that
        happens on the given day */
    bool overlaps(CellItem *o) const
    {
        PrintCellItem *other = static_cast<PrintCellItem *>(o);
        return !(other->start() >= end() || other->end() <= start());
    }

private:
    KCalCore::Event::Ptr mEvent;
    KDateTime mStart, mEnd;
};

}

}

#endif
