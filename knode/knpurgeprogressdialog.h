/***************************************************************************
                          knpurgeprogressdialog.h  -  description
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


#ifndef KNPURGEPROGRESSDIALOG_H
#define KNPURGEPROGRESSDIALOG_H

#include <qframe.h>
#include <qlabel.h>

class QProgressBar;

class KNPurgeProgressDialog : public QFrame  {
 	
 	public:
		KNPurgeProgressDialog();
		~KNPurgeProgressDialog();
	
		void init(const QString& txt, int st);
		void setInfo(const QString& txt) { info->setText(txt); }
		void progress();
		
		
	protected:
		QLabel *text, *info;
		QProgressBar *pb;
		
		int s_teps, p_rogress;	
};

#endif
