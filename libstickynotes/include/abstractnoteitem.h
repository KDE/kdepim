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
 
#ifndef LSN_ABSTRACTNOTEITEM_H
#define LSN_ABSTRACTNOTEITEM_H

#include <StickyNotes/global.h>

#include <QVariant>

namespace StickyNotes {

struct LSN_EXPORT AbstractNoteItem
{
        virtual ~AbstractNoteItem() {}

	/*! Get an attribute value.
	 * \param _name Attribute name
	 * \return Attribute value
	 */
	virtual QVariant attribute(const QString &_name) const = 0;

	/*! Get all attribute names.
	 * \return List of attribute names
	 */
	virtual QList<QString> attributeNames(void) const = 0;

	/*! Get note content.
	 * \return Content
	 */
	virtual QString content(void) const = 0;

	/*! Get note subject.
	 * \return Subject
	 */
	virtual QString subject(void) const = 0;

	/*! Set an attribute value.
	 * \param _name	    Note attribute name
	 * \param _value    Note attribute value
	 */
	virtual void setAttribute(const QString &_name,
	    const QVariant &_value) = 0;

	/*! Set note content.
	 * \param _content Note content
	 */
	virtual void setContent(const QString &_content) = 0;

	/*! Set note subject.
	 * \param _subject Note subject
	 */
	virtual void setSubject(const QString &_subject) = 0;
};

} // namespace StickyNotes

#endif // !LSN_ABSTRACTNOTEITEM_H

