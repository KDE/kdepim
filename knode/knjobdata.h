/***************************************************************************
                          knjobdata.h  -  description
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


#ifndef KNJOBDATA_H
#define KNJOBDATA_H


#include "knserverinfo.h"

class KNJobData {
	
	public:
		
		enum jobType {	JTlistGroups,
										JTfetchNewHeaders,
										JTfetchArticle,
										JTpostArticle,
										JTmail };
										
		
		KNJobData(jobType t, KNServerInfo *a, void *d);
		~KNJobData();
		
		jobType type() const { return t_ype; }
		
		KNServerInfo* account()	const { return a_ccount; }
		void* data() const 	{ return d_ata; }
		
		const QString& errorString() const { return e_rrorString; }
		bool success() const { return e_rrorString.isEmpty(); }
		bool canceled()	const { return c_anceled; }
		
		void setErrorString(const QString& s)	{ e_rrorString=s; }
		void cancel()	{ c_anceled=true; }
		
	protected:
		jobType t_ype;
		void *d_ata;
		KNServerInfo *a_ccount;
		QString e_rrorString;
		bool c_anceled;		
		
	
		
};

#endif
