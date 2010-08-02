/*
 *  eventlistviewbase.h  -  base classes for widget showing list of events
 *  Program:  kalarm
 *  Copyright (c) 2004-2006 by David Jarvie <software@astrojar.org.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef EVENTLISTVIEWBASE_H
#define EVENTLISTVIEWBASE_H

#include "kalarm.h"

#include <tqvaluelist.h>
#include <klistview.h>

#include "alarmevent.h"

class TQPixmap;
class EventListViewItemBase;
class Find;


class EventListViewBase : public KListView
{
		Q_OBJECT
	public:
		typedef TQValueList<EventListViewBase*>              InstanceList;
		typedef TQValueListIterator<EventListViewBase*>      InstanceListIterator;
		typedef TQValueListConstIterator<EventListViewBase*> InstanceListConstIterator;

		EventListViewBase(TQWidget* parent = 0, const char* name = 0);
		virtual ~EventListViewBase()  { }
		EventListViewItemBase* getEntry(const TQString& eventID) const;
		void                   addEvent(const KAEvent& e)  { addEvent(e, instances(), this); }
		void                   modifyEvent(const KAEvent& e)
		                                              { modifyEvent(e.id(), e, instances(), this); }
		void                   modifyEvent(const TQString& oldEventID, const KAEvent& newEvent)
		                                              { modifyEvent(oldEventID, newEvent, instances(), this); }
		void                   deleteEvent(const TQString& eventID)  { deleteEvent(eventID, instances()); }
		static void            addEvent(const KAEvent&, const InstanceList&, EventListViewBase* selectionView);
		static void            modifyEvent(const KAEvent& e, const InstanceList& list, EventListViewBase* selectionView)
		                                              { modifyEvent(e.id(), e, list, selectionView); }
		static void            modifyEvent(const TQString& oldEventID, const KAEvent& newEvent, const InstanceList&, EventListViewBase* selectionView);
		static void            deleteEvent(const TQString& eventID, const InstanceList&);
		static void            undeleteEvent(const TQString& oldEventID, const KAEvent& event, const InstanceList& list, EventListViewBase* selectionView)
		                                              { modifyEvent(oldEventID, event, list, selectionView); }
		void                   resizeLastColumn();
		int                    itemHeight();
		EventListViewItemBase* currentItem() const    { return (EventListViewItemBase*)KListView::currentItem(); }
		EventListViewItemBase* firstChild() const     { return (EventListViewItemBase*)KListView::firstChild(); }
		bool                   anySelected() const;    // are any items selected?
		const KAEvent*         selectedEvent() const;
		EventListViewItemBase* selectedItem() const;
		TQValueList<EventListViewItemBase*> selectedItems() const;
		int                    selectedCount() const;
		int                    lastColumn() const     { return mLastColumn; }
		virtual TQString        whatsThisText(int column) const = 0;
		virtual InstanceList   instances() = 0; // return all instances

	public slots:
		void                   refresh();
		virtual void           slotFind();
		virtual void           slotFindNext()         { findNext(true); }
		virtual void           slotFindPrev()         { findNext(false); }
		virtual void           slotSelectAll();
		virtual void           slotDeselect();

	signals:
		void                   itemDeleted();
		void                   findActive(bool);

	protected:
		virtual void           populate() = 0;         // populate the list with all desired events
		virtual EventListViewItemBase* createItem(const KAEvent&) = 0;   // only used by default addEntry() method
		virtual bool           shouldShowEvent(const KAEvent&) const  { return true; }
		EventListViewItemBase* addEntry(const KAEvent&, bool setSize = false, bool reselect = false);
		EventListViewItemBase* addEntry(EventListViewItemBase*, bool setSize, bool reselect);
		EventListViewItemBase* updateEntry(EventListViewItemBase*, const KAEvent& newEvent, bool setSize = false, bool reselect = false);
		void                   addLastColumn(const TQString& title);
		virtual void           showEvent(TQShowEvent*);
		virtual void           resizeEvent(TQResizeEvent*);

	private:
		void                   deleteEntry(EventListViewItemBase*, bool setSize = false);
		void                   findNext(bool forward);

		Find*                  mFind;                 // alarm search object
		int                    mLastColumn;           // index to last column
		int                    mLastColumnHeaderWidth;
};


class EventListViewItemBase : public QListViewItem
{
	public:
		EventListViewItemBase(EventListViewBase* parent, const KAEvent&);
		const KAEvent&         event() const             { return mEvent; }
		TQPixmap*               eventIcon() const;
		int                    lastColumnWidth() const   { return mLastColumnWidth; }
		EventListViewItemBase* nextSibling() const       { return (EventListViewItemBase*)TQListViewItem::nextSibling(); }
		static int             iconWidth();

	protected:
		void                   setLastColumnText();
		virtual TQString        lastColumnText() const = 0;   // get the text to display in the last column

	private:
		static TQPixmap*        mTextIcon;
		static TQPixmap*        mFileIcon;
		static TQPixmap*        mCommandIcon;
		static TQPixmap*        mEmailIcon;
		static int             mIconWidth;        // maximum width of any icon

		KAEvent                mEvent;            // the event for this item
		int                    mLastColumnWidth;  // width required to display message column
};

#endif // EVENTLISTVIEWBASE_H

