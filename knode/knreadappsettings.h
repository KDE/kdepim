/***************************************************************************
                          knreadappsettings.h  -  description
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


#ifndef KNREADAPPSETTINGS_H
#define KNREADAPPSETTINGS_H

#include "knsettingswidget.h"
#include <qlistbox.h>
#include <kcolorbtn.h>
#include <qcombobox.h>
#include <qspinbox.h>


class KNReadAppSettings : public KNSettingsWidget  {

	Q_OBJECT	

	public:
		KNReadAppSettings(QWidget *p);
		~KNReadAppSettings();
		
		void init();
		void apply();
		
	protected:
		QListBox *cList;
		KColorButton *colBtn;
		QComboBox *fntFam;
		QComboBox *fntSize;
		QColor colors[7];
		
	protected slots:
		void slotCListChanged(int id);
		void slotColorChanged(const QColor &col);
};

#endif
