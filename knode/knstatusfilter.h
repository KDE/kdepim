/***************************************************************************
                          knstatusfilter.h  -  description
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

#ifndef KNSTATUSFILTER_H
#define KNSTATUSFILTER_H

#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <ksimpleconfig.h>
#include "knfetcharticle.h"


class KNStatusFilter {
	
	friend class KNStatusFilterWidget;

	public:
		KNStatusFilter();
		~KNStatusFilter();
	
	  KNStatusFilter& operator=(const KNStatusFilter &sf)
			{ for(int i=0; i<8; i++) data.setBit(i, sf.data.at(i)); return (*this); }
		
		void load(KSimpleConfig *conf);
		void save(KSimpleConfig *conf);	
			
		bool doFilter(KNFetchArticle *a);
		
	protected:	
		QBitArray data;

};



class KNStatusFilterWidget : public QButtonGroup  {
	
	Q_OBJECT
	
	public:
		KNStatusFilterWidget(QWidget *parent);
		~KNStatusFilterWidget();

		KNStatusFilter filter();
		void setFilter(KNStatusFilter &f);
		void clear();		
		
		
	protected:
		
		class TFCombo : public QComboBox {
			
			public:
				TFCombo(QWidget *parent);
				~TFCombo();
				void setValue(bool b)	{ if(b) setCurrentItem(0); else setCurrentItem(1); }
				bool value()					{ return (currentItem()==0); }
		};
				
		
		QCheckBox *enR, *enN, *enUS, *enNS;
		TFCombo *rCom, *nCom, *usCom, *nsCom;
	
	protected slots:
		void slotEnabled(int c);

};


#define EN_R 	0
#define EN_N 	1
#define EN_US 2
#define EN_NS 3

#define DAT_R 	4
#define DAT_N 	5
#define DAT_US 	6
#define DAT_NS 	7





#endif












