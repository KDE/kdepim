/***************************************************************************
                          knscoredialog.h  -  description
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


#ifndef KNSCOREDIALOG_H
#define KNSCOREDIALOG_H

#include <qdialog.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qpushbutton.h>



class KNScoreDialog : public QDialog  {

  Q_OBJECT
	
	public:
		KNScoreDialog(short sc=50, QWidget *parent=0, const char *name=0);
		~KNScoreDialog();
		
		short score();
		
	protected:
		QButtonGroup *bg;
		QRadioButton *iBtn, *nBtn, *wBtn, *cBtn;
		QSpinBox *spin;
		QPushButton *okBtn, *cancelBtn;
		
		
};

#endif

