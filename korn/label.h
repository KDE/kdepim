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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MK_LABEL_H
#define MK_LABEL_H

#include <qlabel.h>
#include <qevent.h>

/**
 * A simple overriding of the QLabel class to get a mouseButtonPressed() signal
 */
class Label : public QLabel
{ Q_OBJECT
public:
	Label( QWidget * parent = 0, const char * name = 0 ) : QLabel( parent, name ) {}
	virtual ~Label() {}
	
protected:
	virtual void mousePressEvent( QMouseEvent *e ) { emit mouseButtonPressed( e->button() ); }
signals:
	/**
	 * Emitted when a button is pressed.
	 */
	void mouseButtonPressed( Qt::ButtonState );
};

#endif //MK_LABEL_H
