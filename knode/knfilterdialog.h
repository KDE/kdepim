/***************************************************************************
                          knfilterdialog.h  -  description
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


#ifndef KNFILTERDIALOG_H
#define KNFILTERDIALOG_H

#include <qsemimodal.h>

class KNFilterConfigWidget;
class KNArticleFilter;
class QLineEdit;
class QComboBox;
class QCheckBox;

class KNFilterDialog : public QSemiModal {
	
	Q_OBJECT

	friend class KNFilterManager;
	
	public:
		KNFilterDialog(QWidget *parent=0, const char *name=0, KNArticleFilter *f=0);
		~KNFilterDialog();
		
		KNArticleFilter* filter()	{ return fltr; }
 	  		
	protected:
		void apply();
				
		KNFilterConfigWidget *fw;
		QLineEdit *fname;
		QComboBox *apon;
		QCheckBox *enabled;
		
		KNArticleFilter *fltr;
		QString savedName;
	
	protected slots:
		void slotOK();
		void slotCancel();
		void slotHelp();
		
	signals:
		void editDone(KNFilterDialog*);
				
};

#endif





