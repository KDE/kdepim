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

class KNMimeInfo : public KNArticleBase{
	
	public:
		KNMimeInfo();
		~KNMimeInfo();
	
		//get
		mediaType ctMediaType()				{ return c_tMType; }
		subType ctSubType()						{ return c_tSType; }
		encoding ctEncoding()					{ return c_tEncoding; }
		disposition ctDisposition()		{ return c_tDisposition; }
		contentCategory ctCategory()  { return c_tCategory; }
		bool isReadable()							{ return i_sReadable; }
				
		//set
		void setCTMediaType(mediaType mt)					{ c_tMType=mt; }
		void setCTSubType(subType st)							{ c_tSType=st; }
		void setCTEncoding(encoding ec)						{ c_tEncoding=ec; }
		void setCTDisposition(disposition dp)			{ c_tDisposition=dp; }
		void setCTCategory(contentCategory c)     { c_tCategory=c; }
		void setIsReadable(bool i)								{ i_sReadable=i; }
		
		
	protected:
	 	mediaType 		    c_tMType;
	  subType    		    c_tSType;
	  encoding			    c_tEncoding;
	  disposition       c_tDisposition;
	  contentCategory   c_tCategory;
	  bool 					    i_sReadable;
		
		
};

#endif
