/***************************************************************************
                          knpostcomsettings.h  -  description
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


#ifndef KNPOSTCOMSETTINGS_H
#define KNPOSTCOMSETTINGS_H

#include "knsettingswidget.h"

class QCheckBox;
class QSpinBox;
class QLineEdit;


class KNPostComSettings : public KNSettingsWidget  {
	
	public:
		KNPostComSettings(QWidget *p);
		~KNPostComSettings();
		
		void apply();
		
	protected:
    void init();
		QSpinBox *maxLen;
		QCheckBox *ownSigCB, *fontCB, *authSigCB;
		QLineEdit *intro, *quot;
		
};

#endif
