/***************************************************************************
                          knfolder.h  -  description
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


#ifndef KNFOLDER_H
#define KNFOLDER_H

#include "knarticlecollection.h"

class KNSavedArticle;


class KNFolder : public KNArticleCollection  {
	
	friend class KNCleanUp;	

	public:
		KNFolder(KNCollection *p=0);
		~KNFolder();
		
		void updateListItem();
		void saveInfo();
		void syncDynamicData(bool force=false);							
		bool loadHdrs();
		bool loadArticle(KNSavedArticle *a);
		bool addArticle(KNSavedArticle *a);
		bool saveArticle(KNSavedArticle *a);
		void removeArticle(KNSavedArticle *a);
		void deleteAll();
		
		///get
		collectionType type()								{ return CTfolder; }
		QString path();
		KNSavedArticle* at(int i)						{ return (KNSavedArticle*) list[i]; }
		KNSavedArticle* byId(int id);				
		int id()														{ return i_d; }
		bool toSync()												{ return t_oSync; }
						
		//set
		void setId(int i)										{ i_d=i; }	
		void setToSync(bool b)							{ t_oSync=b; }
				
		protected:
			void saveDynamicData(int start, int cnt, bool ovr=false);
			void saveStaticData(int start, int cnt, bool ovr=false);
			
			int i_d;
			bool t_oSync;
			
			class dynData {
				public:
					dynData()  {}
					~dynData() {}
					void setData(KNSavedArticle *a);
					
					int id, status, so, eo, sId;
					time_t ti;
			};
					
					
};

#endif
