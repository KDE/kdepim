/***************************************************************************
                          knsenderrordialog.h  -  description
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


#ifndef KNSENDERRORDIALOG_H
#define KNSENDERRORDIALOG_H

#include <qsemimodal.h>
#include <qlist.h>

class QPushButton;
class QLabel;

class KNListBox;
class KNJobData;


class KNSendErrorDialog : public QSemiModal  {
	
	Q_OBJECT	

	public:
		KNSendErrorDialog();
		~KNSendErrorDialog();
		
		void appendJob(KNJobData *job);
		
	protected:
		KNListBox *jobs;
		QLabel *error;
		QPushButton *closeBtn;
	  QList<KNJobData> jobList;
		
	protected slots:
		void slotJobHighlighted(int idx);
    void slotCloseBtnClicked();

	signals:
		void dialogDone();

};

#endif
