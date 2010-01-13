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
 
#ifndef LSN_BASENOTEITEM_H
#define LSN_BASENOTEITEM_H

#include <StickyNotes/AbstractNoteItem>

namespace StickyNotes {

class BaseNoteItemPrivate;

class LSN_EXPORT BaseNoteItem : public AbstractNoteItem
{
Q_DECLARE_PRIVATE(BaseNoteItem)

public:
	/*! Construct a BaseNoteItem.
	 * All items can have a _parent Item that will supply the data
         * required as well as free us on destruction.
	 * \param _parent Parent Item
	 */
	explicit BaseNoteItem(BaseNoteItem *_parent = 0);
	virtual ~BaseNoteItem(void);

	/*! Get our children items.
	 * /return A list of all child Items
	 */
	const QList<AbstractNoteItem *> & children(void) const;

	/*! Get our parent item (if any).
	 * /return Parent Item or null
	 */
	BaseNoteItem * parent(void) const;

	/* virtual */

	virtual QVariant attribute(const QString &_name) const;
	virtual QList<QString> attributeNames(void) const;
	virtual QString  content(void) const;
	virtual void setAttribute(const QString &_name, const QVariant &_value);
	virtual void setContent(const QString &_content);

	/*! Set parent item.
	 * /param _parent Parent item
	 * /return TRUE if an actual change was made
	 */
	virtual bool setParent(BaseNoteItem *_parent);

	virtual void setSubject(const QString &_subject);
	virtual QString subject(void) const;

protected:

	/*! Push note attribute to children.
	 * \param _sender  Item updating content
	 * \param _name	   Note attribute name
	 * \param _value   Note attribute value
	 */
	void pushAttribute(AbstractNoteItem * const _sender,
	    const QString &_name, const QVariant &_value);

	/*! Push note content to children.
	 * \param _sender  Item updating content
	 * \param _content Note content
	 */
	void pushContent(AbstractNoteItem * const _sender,
	    const QString &_content);

	/*! Push note subject to children.
	 * \param _sender  Item updating content
	 * \param _subject Note subject
	 */
	void pushSubject(AbstractNoteItem * const _sender,
	    const QString &_subject);

	/* virtual */

	/*! Set note attribute value.
	 * \param _sender  Item updating content
	 * \param _name	   Note attribute name
	 * \param _value   Note attribute value
	 * /return TRUE if an actual change was made
	 */
	virtual bool applyAttribute(BaseNoteItem * const _sender,
	    const QString &_name, const QVariant &_value);

	/*! Set note content.
	 * \param _sender  Item updating content
	 * \param _content Note content
	 * /return TRUE if an actual change was made
	 */
	virtual bool applyContent(BaseNoteItem * const _sender,
	    const QString &_content);

	/*! Set note subject.
	 * \param _sender  Item updating content
	 * \param _subject Note subject
	 * /return TRUE if an actual change was made
	 */
	virtual bool applySubject(BaseNoteItem * const _sender,
	    const QString &_subject);

	/*! Bind child Item.
	 * /param _child Item
	 * /return TRUE if an actual change was made
	 */
	virtual bool bindChild(AbstractNoteItem *_child);

	/*! Unbind child item.
	 * /param _child Child Item
	 * /return TRUE if an actual change was made
	 */
	virtual bool unbindChild(AbstractNoteItem *_child);

private:
	BaseNoteItemPrivate *d_ptr;
};

} // namespace StickyNotes

#endif // !LSN_BASENOTEITEM_H

