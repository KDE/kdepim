/***************************************************************************
                          knaccmailsettings.h  -  description
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


#ifndef KNACCMAILSETTINGS_H
#define KNACCMAILSETTINGS_H

#include "knsettingswidget.h"

class QLineEdit;
class QSpinBox;
class KNServerInfo;

class KNAccMailSettings : public KNSettingsWidget  {
	
	public:
		KNAccMailSettings(QWidget *p);
		~KNAccMailSettings();
		
		void apply();
		
	protected:
		void init();
		
		KNServerInfo *serverInfo;
		QLineEdit *s_erver, *p_ort;
    QSpinBox *h_old, *t_imeout;
};

#endif
