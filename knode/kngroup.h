/***************************************************************************
                          kngroup.h  -  description
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


#ifndef KNGROUP_H
#define KNGROUP_H

#include "knarticlecollection.h"

class KNFetchArticle;
class KNUserEntry;
class KNNntpAccount;

class QStrList;


class KNGroup : public KNArticleCollection  {
	
	public:
		KNGroup(KNCollection *p=0);
		~KNGroup();
		
		void updateListItem();
		void saveInfo();
		void showProperties();
		bool loadHdrs();
	  void insortNewHeaders(QStrList *hdrs);
	  int saveStaticData(int cnt,bool ovr=false);
	  void saveDynamicData(int cnt,bool ovr=false);
	  void syncDynamicData();
		void updateThreadInfo();
		void resort();
		
		//get
		collectionType type()								{ return CTgroup; }
		QString path();
		KNNntpAccount* account();
		KNFetchArticle* at(int i)						{ return (KNFetchArticle*) list[i]; }
		KNFetchArticle* byId(int id);				
		KNFetchArticle* byMessageId(const QCString &mId);
		int newCount()											{ return n_ewCount; }
		int readCount()											{ return r_eadCount; }
		int lastNr() 												{ return l_astNr; }
		int maxFetch() 	                    { return m_axFetch; }
		const QString& name();
		const QCString& groupname()  				{ return g_roupname; }
		KNUserEntry* user()									{ return u_ser; }
		bool hasName()											{ return (!n_ame.isEmpty()); }
		int statThrWithNew();
		int statThrWithUnread();
		bool locked()												{ return l_ocked; }
		
		//set
		void setGroupname(const QCString &s)	{ g_roupname=s; }
		void setNewCount(int i)								{ n_ewCount=i; }
		void incNewCount(int i=1)           	{ n_ewCount+=i; }
		void decNewCount(int i=1)           	{	n_ewCount-=i; }
		void setReadCount(int i)							{ r_eadCount=i; }
		void incReadCount(int i=1)          	{ r_eadCount+=i; }
		void decReadCount(int i=1)          	{ r_eadCount-=i; }
		void setLastNr(int i)               	{ l_astNr=i; }
		void setMaxFetch(int i)             	{ m_axFetch=i; }
		void setUser(KNUserEntry *u)					{ u_ser=u; }
		void setLocked(bool l)								{ l_ocked=l; }
								
	protected:
		void sortHdrs(int cnt);
		int findRef(KNFetchArticle *a, int from, int to, bool reverse=false);
				
		int n_ewCount, r_eadCount, l_astNr, m_axFetch;
		QCString g_roupname;
		KNUserEntry *u_ser;
		bool l_ocked;
		
		
		class dynData {
			
			public:
				dynData()			{ id=-1; idRef=-1; read=0; thrLevel=0; score=50; }
				~dynData()		{}	
		    void setData(KNFetchArticle *a);
			
				int id;
				int idRef;
				bool read;
				short thrLevel, score;
		};
		
};

#endif
