/*
 *  undo.h  -  undo/redo facility
 *  Program:  kalarm
 *  Copyright (C) 2005 by David Jarvie <software@astrojar.org.uk>
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

#ifndef UNDO_H
#define UNDO_H

/**  @file undo.h - undo/redo facility */

#include <tqvaluelist.h>
#include <tqstringlist.h>

class KAEvent;
class UndoItem;


class Undo : public QObject
{
		Q_OBJECT
	public:
		enum Type { NONE, UNDO, REDO };

		static Undo*       instance();
		static void        saveAdd(const KAEvent&);
		static void        saveEdit(const KAEvent& oldEvent, const KAEvent& newEvent);
		static void        saveDelete(const KAEvent&);
		static void        saveDeletes(const TQValueList<KAEvent>&);
		static void        saveReactivate(const KAEvent&);
		static void        saveReactivates(const TQValueList<KAEvent>&);
		static bool        undo(TQWidget* parent, const TQString& action)
		                                      { return undo(mUndoList.begin(), UNDO, parent, action); }
		static bool        undo(int id, TQWidget* parent, const TQString& action)
		                                      { return undo(findItem(id, UNDO), UNDO, parent, action); }
		static bool        redo(TQWidget* parent, const TQString& action)
		                                      { return undo(mRedoList.begin(), REDO, parent, action); }
		static bool        redo(int id, TQWidget* parent, const TQString& action)
		                                      { return undo(findItem(id, REDO), REDO, parent, action); }
		static void        clear();
		static bool        haveUndo()         { return !mUndoList.isEmpty(); }
		static bool        haveRedo()         { return !mRedoList.isEmpty(); }
		static TQString     actionText(Type);
		static TQString     actionText(Type, int id);
		static TQString     description(Type, int id);
		static TQValueList<int> ids(Type);
		static void        emitChanged();

		// Types for use by UndoItem class and its descendants
		typedef TQValueList<UndoItem*>  List;

	signals:
		void               changed(const TQString& undo, const TQString& redo);

	protected:
		// Methods for use by UndoItem class
		static void        add(UndoItem*, bool undo);
		static void        remove(UndoItem*, bool undo);
		static void        replace(UndoItem* old, UndoItem* New);

	private:
		typedef TQValueList<UndoItem*>::Iterator Iterator;

		Undo(TQObject* parent)  : TQObject(parent) { }
		static void        removeRedos(const TQString& eventID);
		static bool        undo(Iterator, Type, TQWidget* parent, const TQString& action);
		static UndoItem*   getItem(int id, Type);
		static Iterator    findItem(int id, Type);
		void               emitChanged(const TQString& undo, const TQString& redo)
		                                   { emit changed(undo, redo); }

		static Undo*       mInstance;     // the one and only Undo instance
		static List        mUndoList;     // edit history for undo, latest undo first
		static List        mRedoList;     // edit history for redo, latest redo first

	friend class UndoItem;
};

#endif // UNDO_H
