/***************************************************************************
                          kncleanup.h  -  description
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


#ifndef KNCLEANUP_H
#define KNCLEANUP_H

#include "kngroup.h"
#include "knfolder.h"

class KNCleanUp {
	
	public:
		KNCleanUp();
		~KNCleanUp();
		
		void group(KNGroup *g, bool withGUI=false);
		void folder(KNFolder *f);
		
		int deleted()								{ return delCnt; }
		int left()                 	{ return leftCnt; }
	
	protected:
		int delCnt, leftCnt;
		int rDays, uDays;
		bool saveThr;
		
};

#endif
