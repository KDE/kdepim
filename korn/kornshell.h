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

#ifndef MK_KORNSHELL_H
#define MK_KORNSHELL_H

class BoxContainer;
class KDialogBase;

class KConfig;

#include <qwidget.h>

/**
 * This is a rewritten KornShell class. It is rewritten because the depending classes changed.
 */
class KornShell : public QWidget
{ Q_OBJECT
public:
	KornShell( QWidget * parent = 0, const char * name = 0 );
	~KornShell();

	void show();	
public slots:
	/**
	 * This function shows the configuration-dialog
	 */
	void optionDlg();
	
private slots:
	void slotDialogClosed();
	void slotApply();
	
private:
	void readConfig();
	
	KConfig *_config;
	BoxContainer *_box;
	KDialogBase *_configDialog;
};

#endif //MK_KORNSHELL_H
