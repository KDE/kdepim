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

#ifndef MK_SYSTEMTRAY_H
#define MK_SYSTEMTRAY_H

#include <ksystemtray.h>

class QWidget;

/**
 * This class is an implementation of a KSystemTray class.
 * It uses different handling of button-clicks.
 *
 * @author Mart Kelder <mart.kde@hccnet.nl>
 */
class SystemTray : public KSystemTray
{ Q_OBJECT
public:
	/**
	 * This contructor gives all it parameters to its parents.
	 * @param parant The parent window
	 * @param name The name of the QObject's parents.
	 */
	SystemTray( QWidget * parent = 0, const char * name = 0 );
	/**
	 * Empty destructor; does nothing at the moment
	 */
	~SystemTray();
	
protected:
	/**
	 * Reimplementation because in the reimplementation of KSystray it popup's of restores.
	 * In this implemention, the action depends on the settings.
	 * @param me An object which contains the mousebutton which is pressed.
	 */
	virtual void mousePressEvent( QMouseEvent* me );
	
signals:
	void mouseButtonPressed( Qt::ButtonState );
};

#endif //MK_SYSTEMTRAY_H
