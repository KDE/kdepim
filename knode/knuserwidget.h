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

class QLineEdit;
class QPushButton;
class QRadioButton;
class QMultiLineEdit;

class KNUserEntry;


class KNUserWidget : public QWidget  {
	
	Q_OBJECT

	public:
		KNUserWidget(QWidget *parent=0, const char *name=0);
		~KNUserWidget();
		
		void setData(KNUserEntry *user);
		void applyData();
		
	protected:
		QLineEdit *name, *orga, *email, *replyTo, *sig;
		QRadioButton *sigFile, *sigEdit;
		QPushButton *chooseBtn, *editBtn;
		QMultiLineEdit *sigEditor;
		KNUserEntry *entry;

	protected slots:
	  void slotSignatureType(int type);
		void slotSignatureChoose();
		void slotSignatureEdit();

};

#endif
