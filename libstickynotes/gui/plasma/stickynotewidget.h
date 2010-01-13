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

#ifndef PLASMA_APPLET_STICKYNOTEWIDGET_H
#define PLASMA_APPLET_STICKYNOTEWIDGET_H

#include <Plasma/Applet>

namespace Plasma { class Label; }

class QGraphicsLinearLayout;
class StickyNoteItem;

class StickyNoteWidget : public Plasma::Applet
{
Q_OBJECT

	friend class StickyNote;
public:	
	explicit StickyNoteWidget(StickyNoteItem &_item);
	virtual ~StickyNoteWidget(void);

	virtual void destroy(void); 
	virtual void init(void);
	virtual void paintInterface(QPainter *_painter,
	    const QStyleOptionGraphicsItem *_option,
	    const QRect& _rect);

protected:
	virtual bool eventFilter(QObject *_watched, QEvent *_event);
	virtual void resizeEvent(QGraphicsSceneResizeEvent *_event);

private:
	void edit(void);
	void setupUi(void);

private slots:
	void on_item_appliedAttribute(const QString &_name, const QVariant &_value);
	void on_item_appliedContent(const QString &_content);
	void on_item_appliedSubject(const QString &_subject);
	void on_item_bound(void);
	void on_item_unbound(void);

private:
	
	Plasma::Label    *m_content;
	StickyNoteItem   *m_item;
	QGraphicsLinearLayout *m_layout;
	bool m_readonly;
	Plasma::Label    *m_subject;
	Plasma::FrameSvg *m_theme;
};
 
#endif // ! PLASMA_APPLET_STICKYNOTE_H

