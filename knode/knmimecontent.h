/***************************************************************************
                          knmimecontent.h  -  description
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


#ifndef KNMIMECONTENT_H
#define KNMIMECONTENT_H

#include <qlist.h>
#include <mimelib/string.h>
#include <qfile.h>
#include <qtextstream.h>

#include "knmimeinfo.h"


class KNMimeContent : public KNArticleBase  {
	
	public:
		KNMimeContent();
		virtual ~KNMimeContent();
	
	  void initContent();
		virtual void parse();
		virtual void clear();
		void clearAttachements()				{ if(ct_List) ct_List->clear(); }		
		void clearHead()								{ if(h_ead) h_ead->clear(); }		
		void clearBody()								{ if(b_ody) b_ody->clear(); }				
		void prepareForDisplay();
		void prepareHtml();		
		
		//get
		virtual articleType type()			{ return ATmimeContent; }
		KNMimeInfo* mimeInfo()					{ return mInfo; }
		bool isMultipart()							{ return mInfo->ctMediaType()==MTmultipart; }
		bool hasContent()								{ return ((h_ead!=0) && (!h_ead->isEmpty())); }
		bool isMainContent()            { return (mInfo->ctCategory()==CCmain); }
		KNMimeContent* mainContent();
		void attachements(QList<KNMimeContent> *dst, bool incAlternatives=false);
		const QCString ctParam(const char* name);
		const QCString& ctCharset();
		const QCString& ctMimeType();
		const QCString& ctName();
		const QCString& ctDescription();
		QCString headerLine(const char* name);				
		char* firstHeaderLine()								{ if(h_ead) return h_ead->first();
																						else return 0; }
		char* nextHeaderLine()								{ if(h_ead) return h_ead->next();
																						else return 0; }
		char* firstBodyLine()									{ if(b_ody) return b_ody->first();
																						else return 0; }
		char* nextBodyLine()									{ if(b_ody) return b_ody->next();
																						else return 0; }
		virtual DwString decodedData();
		virtual DwString encodedData();
		void toStream(QTextStream &ts);			
								
		
		//set	
		void setData(QStrList *data, bool crfl=true);
		void addBodyLine(const char* line) 	{ if(b_ody) b_ody->append(line); }
		void addHeaderLine(const char *line, bool encode=false);
		void setHeader(const char* name, const char *value, bool encode=false);
		void setHeader(headerType t, const char *v, bool encode=false);
		bool removeHeader(const char* name);
			
						
	protected:
		QStrList      				*h_ead, *b_ody;
		QList<KNMimeContent>  *ct_List;
		KNMimeInfo *mInfo;
				
};

#endif
