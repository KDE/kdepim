/***************************************************************************
                          knsavedarticle.h  -  description
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


#ifndef KNSAVEDARTICLE_H
#define KNSAVEDARTICLE_H

#include "knarticle.h"

class KNFolder;
class KNSavedArticle : public KNArticle  {
	
	public:
		KNSavedArticle(articleStatus s=ASunknown);
		~KNSavedArticle();
		
		void parse();
		void clear();
		void updateListItem();
				
		//get		
		articleType type()										{ return ATsaved; }
		articleStatus status()								{ return s_tatus; }
		const QCString& destination()					{ return d_estination; }
		const QCString& firstDestination();
		int serverId()							{ return s_erverId; }
		KNFolder* folder()					{ return f_older; }
		int startOffset()						{ return s_tartOffset; }
		int endOffset()							{ return e_ndOffset; }
		bool locked()								{ return l_ocked; }
		bool sent()									{ return (s_tatus==ASmailed || s_tatus==ASposted || s_tatus==AScanceled); }
		bool editable();							
		bool isMail()								{ return (s_tatus==AStoMail || s_tatus==ASmailed); }
		bool hasDestination()				{ return (!d_estination.isEmpty()); }
		bool canceled()							{ return (s_tatus==AScanceled); }
				
				
		//set
		void setDestination(const QCString &s){ d_estination=s; }
		void setServerId(int s)								{ s_erverId=s; }	
		void setFolder(KNFolder *f)						{ f_older=f; }
		void setStartOffset(int o)						{ s_tartOffset=o; }								
		void setEndOffset(int o)							{ e_ndOffset=o; }
		void setLocked(bool b)								{ l_ocked=b; }
		void setStatus(articleStatus s)				{ s_tatus=s; }
								
	protected:
		QCString d_estination;
		articleStatus s_tatus;
		int s_tartOffset, e_ndOffset, s_erverId;
		KNFolder *f_older;
		bool l_ocked;
		
};

#endif
