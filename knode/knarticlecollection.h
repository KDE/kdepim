/***************************************************************************
                          knarticlecollection.h  -  description
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


#ifndef KNARTICLECOLLECTION_H
#define KNARTICLECOLLECTION_H

#include <qfile.h>

#include "kncollection.h"

class KNArticle;

class KNArticleCollection : public KNCollection {

	public:
		KNArticleCollection(KNCollection *p=0);
		~KNArticleCollection();
				
		virtual void saveInfo()=0;
		bool resize(int s=0);
		bool append(KNArticle *a);
		void clearList();
		void compactList();
		
		//get		
		bool isEmpty()								{ return ( (list==0) || (len==0) ); }
		bool isFilled()								{ return (!(c_ount>0 && len==0)); }
		int size()										{ return siz; }
		int length()									{ return len; }
		int increment()								{ return incr; }
		
		//set
		void setIncrement(int i)			{ incr=i; }
		void setLastID(); 							
					
	protected:
		int findId(int id);
		int siz, len, lastID, incr;
		KNArticle **list;
};


//==============================================================================


class KNFile : public QFile {

  public:
    KNFile(const QString& fname=QString::null);
    ~KNFile();
    const QCString& readLine();
    const QCString& readLineWnewLine();

   protected:
    bool increaseBuffer();

    QCString buffer;
    char *dataPtr;
    int filePos, readBytes;
};

#endif
