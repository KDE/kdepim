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

#include <tqlabel.h>
#include <tqevent.h>

/**
 * A simple overriding of the TQLabel class to get a mouseButtonPressed() signal
 */
class Label : public QLabel
{ Q_OBJECT
public:
	Label( TQWidget * parent = 0, const char * name = 0 ) : TQLabel( parent, name ) {}
	virtual ~Label() {}
	
protected:
	virtual void mousePressEvent( TQMouseEvent *e ) { emit mouseButtonPressed( e->button() ); }
signals:
	/**
	 * Emitted when a button is pressed.
	 */
	void mouseButtonPressed( Qt::ButtonState );
};

#endif //MK_LABEL_H
