/***************************************************************************
                          kncollection.h  -  description
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


#ifndef KNCOLLECTION_H
#define KNCOLLECTION_H

#include <qstring.h>

class KNCollectionViewItem;


class KNCollection {

	public:
		enum collectionType { 	CTnntpAccount, CTgroup,
                            CTfolder, CTcategory,
                            CTvirtualGroup };

		KNCollection(KNCollection *p);
		virtual ~KNCollection();
		virtual void updateListItem();
		
		// trys to read information, returns false if it fails to do so
    virtual bool readInfo(const QString &confPath)=0;
    virtual void saveInfo()=0;		
		
		//get
		virtual collectionType type()=0;
		virtual QString path()=0;
		KNCollection* parent()						{ return p_arent; }
		virtual const QString& name() 		{ return n_ame; }
		KNCollectionViewItem* listItem()  { return l_istItem; }
		int count()												{ return c_ount; }
		
		//set
		void setName(const QString &s)		{ n_ame=s; }
		void setListItem(KNCollectionViewItem *i);
		void setCount(int i)							{ c_ount=i; }
		void incCount(int i)							{ c_ount+=i; }
		void decCount(int i)							{ c_ount-=i; }
		
		
	protected:
		KNCollection *p_arent;
		KNCollectionViewItem *l_istItem;
		QString n_ame;
		int c_ount;
	
};

#endif
