/***************************************************************************
                          knnntpaccount.h  -  description
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


#ifndef KNNNTPACCOUNT_H
#define KNNNTPACCOUNT_H

//#include <qlist.h>
#include "knserverinfo.h"
#include "kngroup.h"


class KNNntpAccount : public KNCollection, public KNServerInfo {
	
	public:
		KNNntpAccount();
		~KNNntpAccount();
		
		collectionType type()							{ return CTnntpAccount; }		
									
		void saveInfo();		
		void syncInfo();
		QString path();
		
		//get
		int unsentCount()								{ return u_nsentCount; }
		bool hasUnsent()								{ return (u_nsentCount>0); }
	
		//set
	  void setUnsentCount(int i)      { u_nsentCount=i; }
	  void incUnsentCount(int i=1)		{ u_nsentCount+=i; }
	  void decUnsentCount(int i=1)		{ u_nsentCount-=i; }
	
	protected:
		int u_nsentCount;
	
};

#endif
