/***************************************************************************
                          knkeysettings.h  -  description
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


#ifndef KNKEYSETTINGS_H
#define KNKEYSETTINGS_H

#include "knsettingswidget.h"

class KKeyChooser;


class KNKeySettings : public KNSettingsWidget  {
	
	public:
		KNKeySettings(QWidget *parent);
		virtual ~KNKeySettings();
				
		void apply();
		
	protected:
	  KKeyChooser *kc;
	  QPushButton *stdBtn;
	
};

#endif
