/***************************************************************************
                          knuserentry.h  -  description
                             -------------------
    
    copyright            : (C) 1999 by Christian Thurner
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


#ifndef KNUSERENTRY_H
#define KNUSERENTRY_H

#include <qstring.h>

class KConfigBase;


class KNUserEntry {
	
	public:
		KNUserEntry();
		~KNUserEntry();
		
		const QCString& name()				{ return n_ame; }
		const QCString& email()				{ return e_mail; }
		const QCString& replyTo()    	{ return r_eplyTo; }
		const QCString& orga()				{ return o_rga; }
		const QCString& sigPath()			{ return s_igPath; }
				
		void setName(const QCString &n)				{ n_ame=n; }
		void setEmail(const QCString &e)			{ e_mail=e; }
		void setReplyTo(const QCString &r) 		{ r_eplyTo=r; }
		void setOrga(const QCString &o)				{ o_rga=o; }
		void setSigPath(const QCString &s)   	{ s_igPath=s; }
				
		void load(KConfigBase *c);
		void save(KConfigBase *c);
		
		bool hasName()				{ return (!n_ame.isEmpty()); }
		bool hasEmail()				{ return (!e_mail.isEmpty()); }
		bool hasReplyTo()			{ return (!r_eplyTo.isEmpty()); }
		bool hasOrga()				{ return (!o_rga.isEmpty()); }
		bool hasSigPath()   	{ return (!s_igPath.isEmpty()); }
		bool isValid()				{ return (!e_mail.isEmpty() && !n_ame.isEmpty()); }
		bool isEmpty();
		
	protected:
		QCString n_ame, e_mail, o_rga, r_eplyTo, s_igPath;
		
};

#endif
