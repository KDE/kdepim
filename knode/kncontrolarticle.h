/***************************************************************************
                          kncontrolarticle.h  -  description
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


#ifndef KNCONTROLARTICLE_H
#define KNCONTROLARTICLE_H

#include "knsavedarticle.h"

class KNControlArticle : public KNSavedArticle  {
	
	public:
		KNControlArticle();
		~KNControlArticle();
		
		void updateListItem();
		
		//get
		articleType type()							{ return ATcontrol; }
		controlType ctlType()						{ return c_tlType; }
		
		//set
		void setCtlType(controlType t)	{ c_tlType=t; }
		
	protected:
		controlType c_tlType;
		
};

#endif
