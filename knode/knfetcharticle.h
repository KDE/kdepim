/***************************************************************************
                          knfetcharticle.h  -  description
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


#ifndef KNFETCHARTICLE_H
#define KNFETCHARTICLE_H

#include <qbitarray.h>
#include "knarticle.h"


class KNFetchArticle : public KNArticle  {
	
	public:
		KNFetchArticle();
		~KNFetchArticle();
				
		void parse();
		void parseFrom(const QCString &f);
		void parseReferences(const QCString &s);
		void clear();
		void initListItem();
	  void updateListItem();
				
		//get
		articleStatus status()									{ return AStemp; }
		articleType type()											{ return ATfetch; }
		virtual const QCString& messageId()			{ return m_Id; }
		virtual const QCString& fromName()				{ return f_rom; }
		virtual const QCString& fromEmail()			{ return e_mail;}
		const QCString& reference(int i)					{ return r_eferences[i]; }
		int lines()															{ return l_ines; }
		int idRef()															{ return i_dRef; }
		time_t fetchTime()												{ return fTimeT; }
		unsigned short threadingLevel()					{ return thrLevel; }
		unsigned short score()									{ return s_core; }
		unsigned short newFollowUps()						{ return newFups; }
		unsigned short unreadFollowUps()				{ return unrFups; }
		bool isRead()														{ return flags.at(0); }
		bool isExpired()          							{ return flags.at(1); }
		bool isNew()														{ return flags.at(2); }
		bool filterResult()											{ return flags.at(3); }
		bool filtered()													{ return flags.at(4); }
		bool hasNewFollowUps()									{ return (newFups > 0); }
		bool hasUnreadFollowUps()								{ return (unrFups > 0); }
		bool hasChanged()												{ return flags.at(5); }
		bool hasReferences() 										{ return (!r_eferences[0].isEmpty()); }
		bool locked()														{ return flags.at(6); }
				
		//set
		void setMessageId(const QCString &s)							{ m_Id=s; }
		void setFromName(const QCString &s)          		{ f_rom=s; }
		void setFromEmail(const QCString &s)         		{ e_mail=s; }
		void setReference(int i, const QCString &s)  		{ r_eferences[i]=s; }
		void setLines(int i)														{ l_ines=i; }
		void setIdRef(int v)														{ i_dRef=v; }
		void setFetchTime(time_t v)											{ fTimeT=v; }
		void setThreadingLevel(unsigned short v)  			{ thrLevel=v; }
		void setScore(unsigned short v)           			{ s_core=v; }
		void setNewFollowUps(unsigned short v)    			{ newFups=v; }
		void incNewFollowUps(unsigned short v=1)				{ newFups+=v; }
		void decNewFollowUps(unsigned short v=1)				{ newFups-=v; }
		void setUnreadFollowUps(unsigned short v) 			{ unrFups=v; }
		void incUnreadFollowUps(unsigned short v=1)			{ unrFups+=v; }
		void decUnreadFollowUps(unsigned short v=1)			{ unrFups-=v; }
		void setRead(bool b)														{ flags.setBit(0,b); }
		void setExpired(bool b)     										{ flags.setBit(1,b); }
		void setNew(bool b)															{ flags.setBit(2,b); }
		void setFilterResult(bool b)										{ flags.setBit(3,b); }
		void setFiltered(bool b)												{ flags.setBit(4,b); }
	  void setHasChanged(bool b)											{ flags.setBit(5,b); }
		void setLocked(bool b)													{ flags.setBit(6,b); }
	
				
	protected:
		QCString m_Id, f_rom, e_mail, r_eferences[5];
		int i_dRef, l_ines;
		unsigned short thrLevel, s_core, newFups, unrFups;
		time_t fTimeT;
		QBitArray flags;
		
};

#endif
