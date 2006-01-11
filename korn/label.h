/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef MK_LABEL_H
#define MK_LABEL_H

/**
 * @file 
 *
 * This file contains a class which adds a signal to a QLabel.
 */

#include <qlabel.h>
#include <QMouseEvent>

/**
 * A simple overriding of the QLabel class to get a mouseButtonPressed() signal
 */
class Label : public QLabel
{ Q_OBJECT
public:
	/**
	 * Constructor.
	 * It just calls the QLabel constructor.
	 *
	 * @param parent the parent of the Label
	 */
	Label( QWidget * parent = 0 ) : QLabel( parent ) {}
	/**
	 * Empty destructor
	 */
	virtual ~Label() {}
	
protected:
	/**
	 * This function is called when the user clicks on the label.
	 * After this function function is called, it emits the signal mouseButtonPressed.
	 * Overloaded from QWidget::mousePressEvent().
	 *
	 * @param e the mouseevent used to determine the button
	 */
	virtual void mousePressEvent( QMouseEvent *e ) { emit mouseButtonPressed( e->button() ); }
signals:
	/**
	 * Emitted when a button is pressed.
	 * 
	 * @param button the button which is clicked
	 */
	void mouseButtonPressed( Qt::MouseButton button );
};

#endif //MK_LABEL_H
