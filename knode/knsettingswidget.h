/***************************************************************************
                          knsettingswidget.h  -  description
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


#ifndef KNSETTINGSWIDGET_H
#define KNSETTINGSWIDGET_H

#include <qwidget.h>


class KNSettingsWidget : public QWidget  {
	
	Q_OBJECT	

	public:
		KNSettingsWidget(QWidget *parent);
		virtual ~KNSettingsWidget();
		
		virtual void apply() {}
		
	protected:
	
		virtual void init()  {}		
		
};

#endif
