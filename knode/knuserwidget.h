/***************************************************************************
                          knuserwidget.h  -  description
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


#ifndef KNUSERWIDGET_H
#define KNUSERWIDGET_H

#include <qgroupbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>

#include "knuserentry.h"

class KNUserWidget : public QGroupBox  {
	
	Q_OBJECT

	public:
		KNUserWidget(QString title=QString::null, QWidget *parent=0, const char *n=0);
		~KNUserWidget();
		
		void setData(KNUserEntry *user);
		void applyData();
		
	protected:
		QLineEdit *name, *email, *replyTo, *orga, *sig;
		QPushButton *sigBtn;
		KNUserEntry *entry;

	protected slots:
		void slotSigButton();
	
};

#endif
