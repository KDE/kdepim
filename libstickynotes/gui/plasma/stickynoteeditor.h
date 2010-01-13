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
 
#ifndef PLASMA_APPLET_STICKYNOTEEDITOR_H
#define PLASMA_APPLET_STICKYNOTEEDITOR_H

#include <QDialog>
#include "ui_stickynoteeditor.h"

class StickyNoteItem;

class StickyNoteEditor :	public QDialog,
				public Ui::StickyNoteEditor
{
Q_OBJECT

public:
	/*! Construct a StickyNoteEditor
	 * All items can have a _parent Item that will supply the data
         * required as well as free us on destruction.
	 * \param _item   Owner item
	 * \param _parent Parent widget
	 * \param _f      Window flags
	 */
	StickyNoteEditor(StickyNoteItem &_item,
	    QWidget *_parent = 0, Qt::WindowFlags _f = 0);
	virtual ~StickyNoteEditor(void);

public slots:
	virtual void accept(void);

private slots:
	void on_boldBtn_clicked(void);
	void on_centerJustifyBtn_clicked(void);
	void on_fontComboBox_currentFontChanged(const QFont &_font);
	void on_fontSizeBox_valueChanged(int _i);
	void on_italicBtn_clicked(void);
	void on_item_appliedContent(const QString &_content);
	void on_item_appliedSubject(const QString &_subject);
	void on_item_bound(void);
	void on_leftJustifyBtn_clicked(void);
	void on_rightJustifyBtn_clicked(void);
	void on_underlineBtn_clicked(void);

private:
	StickyNoteItem *m_item;
};

#endif // !PLASMA_APPLET_STICKYNOTEEDITOR_H

