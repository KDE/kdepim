/*-
 * Copyright 2009 KDAB and Guillermo A. Amaral B. <gamaral@amaral.com.mx>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
 
#ifndef LSN_STANDALONENOTEITEM_H
#define LSN_STANDALONENOTEITEM_H

#include <StickyNotes/HookNoteItem>

namespace StickyNotes {

class StandaloneNoteItemPrivate;

class LSN_EXPORT StandaloneNoteItem : public HookNoteItem
{
Q_OBJECT
Q_DECLARE_PRIVATE(StandaloneNoteItem)

public:
	/*! Create a new StandaloneNoteItem
	 * All items can have a _parent Item that will supply the data
         * required as well as free us on destruction.
	 * \param _parent Parent Item
	 */
	StandaloneNoteItem(BaseNoteItem *_parent = 0);
	virtual ~StandaloneNoteItem(void);

	/*! Show note editor.
	 */
	void edit(void);

private:
	StandaloneNoteItemPrivate *d_ptr;
};

} // namespace StickyNotes

#endif // !LSN_STANDALONENOTEITEM_H

