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
 
#ifndef LSN_MEMORYNOTEITEM_H
#define LSN_MEMORYNOTEITEM_H

#include <StickyNotes/BaseNoteItem>

namespace StickyNotes {

class MemoryNoteItemPrivate;

class LSN_EXPORT MemoryNoteItem : public BaseNoteItem
{
Q_DECLARE_PRIVATE(MemoryNoteItem)

public:
	/*! Construct a MemoryNoteItem.
	 * \param _subject  Note subject
	 * \param _content  Note content
	 */
	MemoryNoteItem(const QString &_subject = QString(),
	    const QString &_content = QString());
	virtual ~MemoryNoteItem(void);

	/* virtual */

	virtual QVariant attribute(const QString &_name) const;
	virtual QList<QString> attributeNames(void) const;
	virtual QString  content(void) const;
	virtual QString  subject(void) const;

protected:
	void init(const QString &_subject, const QString &_content);

	/* virtual */

	virtual bool applyAttribute(BaseNoteItem * const _sender,
	    const QString &_name, const QVariant &_value);
	virtual bool applyContent(BaseNoteItem * const _sender,
	    const QString &_content);
	virtual bool applySubject(BaseNoteItem * const _sender,
	    const QString &_subject);

private:
	MemoryNoteItemPrivate * d_ptr;
};

} // namespace StickyNotes

#endif // !LSN_MEMORYNOTEITEM_H

