/***************************************************************************
                          knarticle.h  -  description
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


#ifndef KNARTICLE_H
#define KNARTICLE_H

#include "knmimecontent.h"
#include <time.h>

class KNHdrViewItem;

class KNArticle : public KNMimeContent  {
	
	public:
		KNArticle();
		~KNArticle();
				
		virtual void parse();
		void parseDate(const QCString &s);
		virtual void assemble();
		virtual void clear();
		virtual void updateListItem()=0;		
		
		//get
		virtual articleStatus status()=0;
		int id()																{ return i_d; }
		time_t timeT()													{ return t_imeT; }
		bool hasData()													{ return (!s_ubject.isEmpty()); }
		bool hasReferences()						        { return (!r_eferences.isEmpty()); }
		virtual int age();
		virtual int lines();						
		virtual const QCString& subject()				{ return s_ubject; }
		virtual const QCString& fromName();
		virtual const QCString& fromEmail();
		const QCString& replyToEmail();
		ReferenceLine& references()             { return r_eferences; }
		const char* timeString();
		bool hasSubject()												{ return (!s_ubject.isEmpty()); }
		virtual bool isNew()										{ return false; }	
		KNHdrViewItem* listItem()								{ return i_tem; }
		
		//set			
		void setSubject(const QCString &s)				{ s_ubject=decodeRFC1522String(s); }
		void setListItem(KNHdrViewItem *it);
		void setId(int i)												{ i_d=i; }
		void setTimeT(time_t t);					
				
	protected:
		QCString s_ubject;
		ReferenceLine r_eferences;
		char* t_imeString;
		
		int i_d;
		time_t t_imeT;
		
		KNHdrViewItem *i_tem;
};

#endif
