/***************************************************************************
                     knstringsplitter.h - description
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

#ifndef KNSTRINGSPLITTER_H
#define KNSTRINGSPLITTER_H

#include <qstring.h>


class KNStringSplitter {
	
	public:
		KNStringSplitter();
		~KNStringSplitter();
				
		void reset()									{ start=0; end=0; sep=""; incSep=false;}
		
		void init(QCString &str, const char *s);
		void init(const char *str, const char *s);
		void setIncludeSep(bool inc) 	{ incSep=inc; }
		
		bool first();
		bool last();
		
		bool next();
		bool prev();	
			
		QCString& string()							{ return dst; }
					
	private:
		QCString src, dst, sep;
		int start,end;
		bool incSep;
			
};

#endif
