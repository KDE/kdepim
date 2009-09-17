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
 
#ifndef LSN_HOOKNOTEITEM_H
#define LSN_HOOKNOTEITEM_H

#include <StickyNotes/BaseNoteItem>

namespace StickyNotes {

class LSN_EXPORT HookNoteItem :	    public QObject,
				    public BaseNoteItem
{
Q_OBJECT

public:
	/*! Construct a HookNoteItem.
	 * All items can have a _parent Item that will supply the data
         * required as well as free us on destruction.
	 * \param _parent Parent Item
	 */
	HookNoteItem(BaseNoteItem *_parent = 0);
	virtual ~HookNoteItem(void);

	/* virtual */

	virtual bool setParent(BaseNoteItem *_parent);

protected:
	virtual bool applyAttribute(BaseNoteItem * const _sender,
	    const QString &_name, const QVariant &_value);
	virtual bool applyContent(BaseNoteItem * const _sender,
	    const QString &_content);
	virtual bool applySubject(BaseNoteItem * const _sender,
	    const QString &_subject);

signals:
	/*! Triggered on confirmed data change, after pushing to children.
	 */
	void appliedAttribute(const QString &_name, const QVariant &_value);

	/*! Triggered on confirmed data change, after pushing to children.
	 */
	void appliedContent(const QString &_content);

	/*! Triggered on confirmed data change, after pushing to children.
	 */
	void appliedSubject(const QString &_subject);

	/*! Triggered on confirmed parent binding.
	 */
	void bound(void);

	/*! Triggered on confirmed parent unbinding.
	 */
	void unbound(void);
};

} // namespace StickyNotes

#endif // !LSN_HOOKNOTEITEM_H

