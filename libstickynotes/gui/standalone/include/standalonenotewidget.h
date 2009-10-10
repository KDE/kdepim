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
 
#ifndef LSN_STANDALONENOTEWIDGET_H
#define LSN_STANDALONENOTEWIDGET_H

#include <StickyNotes/global.h>
#include <QSvgWidget>

namespace StickyNotes {

class StandaloneNoteItem;
class StandaloneNoteWidgetPrivate;

class LSN_EXPORT StandaloneNoteWidget : public QSvgWidget
{
Q_OBJECT
Q_DECLARE_PRIVATE(StandaloneNoteWidget)

public:
	/*! Construct a StandaloneNoteWidget
	 * All items can have a _parent Item that will supply the data
         * required as well as free us on destruction.
	 * \param _item   Owner item
	 * \param _parent Parent widget
	 * \param _f      Window flags
	 */
	StandaloneNoteWidget(StandaloneNoteItem &_item,
	    QWidget *_parent = 0, Qt::WindowFlags _f = 0);
	virtual ~StandaloneNoteWidget(void);

protected:
	/*! Event filter for child label.
	 */
	virtual bool eventFilter(QObject *_watched, QEvent *_event);

private:
	/*! Scale note to screen.
	 */
	void scaleToScreen(void);

private slots:
	void on_item_appliedAttribute(const QString &_name, const QVariant &_value);
	void on_item_appliedContent(const QString &_content);
	void on_item_appliedSubject(const QString &_subject);
	void on_item_bound(void);

private:
	StandaloneNoteWidgetPrivate *d_ptr;
};

} // namespace StickyNotes

#endif // !LSN_STANDALONENOTEWIDGET_H

