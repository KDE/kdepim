/***************************************************************************
                          knmimeinfo.h  -  description
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


#ifndef KNMIMEINFO_H
#define KNMIMEINFO_H

#include "knarticlebase.h"

class KNMimeContent;


class KNMimeInfo : public KNArticleBase {
	
	public:
		KNMimeInfo();
		~KNMimeInfo();
	
		void operator=(const KNMimeInfo &i);
		void clear();
		
		//parse
		void parse(KNMimeContent *c);
		
		//get
		mediaType ctMediaType()				  { return c_tMType; }
		subType ctSubType()						  { return c_tSType; }
		encoding ctEncoding()					  { return c_tEncoding; }
		disposition ctDisposition()		  { return c_tDisposition; }
		contentCategory ctCategory()    { return c_tCategory; }
		bool isReadable();
		bool decoded();							
		const QCString& contentType();
		QCString contentTransferEncoding();
		QCString contentDisposition();
		QCString getCTParameter(const char *param);
		bool ctParameterIsSet(const char *param);
						
		//set
		void setCTMediaType(mediaType mt)					{ c_tMType=mt; }
		void setCTSubType(subType st)							{ c_tSType=st; }
		void setCTEncoding(encoding ec)						{ c_tEncoding=ec; }
		void setCTEncoding()                      { c_tEncoding=defaultTEncoding; }
		void setCTDisposition(disposition dp)			{ c_tDisposition=dp; }
		void setCTCategory(contentCategory c)     { c_tCategory=c; }
		void setDecoded(bool b)								    { d_ecoded=b; }
		void setCustomMimeType(const QCString &m);
		//void addCTParameter(const QCString &s);
		void setCTParameter(const QCString &name, const QCString &value, bool doubleQuotes=true);
		void setCharsetParameter(const QCString &p);
		void setBoundaryParameter(const QCString &p);
		void setNameParameter(const QCString &p);
			
	protected:
	 	QCString assembleMimeType();
	 	void parseMimeType(const QCString &s);
	 	void parseEncoding(const QCString &s);
	 	void parseDisposition(const QCString &s);
	 	mediaType 		    c_tMType;
	  subType    		    c_tSType;
	  encoding			    c_tEncoding;
	  disposition       c_tDisposition;
	  contentCategory   c_tCategory;
	  bool 					    d_ecoded;
	  QCString          c_ontentType;
		
};

#endif
