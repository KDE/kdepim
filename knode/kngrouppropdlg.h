/***************************************************************************
                          kngrouppropdlg.h  -  description
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


#ifndef KNGROUPPROPDLG_H
#define KNGROUPPROPDLG_H


#include <qtabdialog.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include "knuserwidget.h"

class KNGroup;

class KNGroupPropDlg : public QTabDialog  {

	public:
		KNGroupPropDlg(KNGroup *group, QWidget *parent=0, const char *name=0);
		~KNGroupPropDlg();
		
		void apply();
		bool nickHasChanged()	{ return nChanged; }	
		
	protected:
	
		class statistics : public QWidget {
		
			public:
				statistics(QWidget *parent=0, const char *name=0, int *values=0);
				~statistics();
				
				void resizeEvent(QResizeEvent*);
								
				QGroupBox *gb1;
				QLabel *t_otal, *u_nread, *n_ew, *u_nrThr, *n_ewThr;
		};
		
		class settings : public QWidget {
			
			public:
				settings(QWidget *parent=0, const char *name=0);
				~settings();
								
				QCheckBox *useID;
				KNUserWidget *uw;
				QLineEdit *nick;
				
		};
		
		settings *set;
		statistics *sta;
		KNGroup *grp;
		bool nChanged;
};

#endif
