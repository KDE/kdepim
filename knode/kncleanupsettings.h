/***************************************************************************
                          kncleanupsettings.h  -  description
                             -------------------
    
    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef KNCLEANUPSETTINGS_H
#define KNCLEANUPSETTINGS_H

#include "knsettingswidget.h"
#include <qcheckbox.h>
#include <qspinbox.h>

class KNCleanupSettings : public KNSettingsWidget  {
	
	Q_OBJECT

	public:
		KNCleanupSettings(QWidget *p);
		~KNCleanupSettings();
		
		void init();
		void apply();
		
	protected:
		QCheckBox *folderCB, *groupCB, *thrCB;
		QSpinBox *folderDays, *groupDays, *readDays, *unreadDays;
		
	protected slots:
		void slotGroupCBtoggled(bool b);
		void slotFolderCBtoggled(bool b);
};

#endif
