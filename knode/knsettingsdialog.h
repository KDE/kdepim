/***************************************************************************
                          knsettingsdialog.h  -  description
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


#ifndef KNSETTINGSDIALOG_H
#define KNSETTINGSDIALOG_H

#include <qdialog.h>
#include <qlistview.h>

class QWidgetStack;
class QSplitter;

class KNSettingsDialog : public QDialog  {
	
	Q_OBJECT	

	public:
		KNSettingsDialog();
		~KNSettingsDialog();
		
		void apply();
		
	protected:
		QListView *lv;
		QWidgetStack *stack;
		QSplitter *split;
		QPushButton *helpBtn, *okBtn, *cancelBtn;
				
		class lvItem : public QListViewItem {
			public:
				lvItem(QListView *p, const QString& t, int i);
				lvItem(QListViewItem *p, const  QString& t, int i);
				~lvItem() {}
				
				int id;
		};			
			
		
	protected slots:
		void slotHelpBtnClicked();
		void slotLVChanged(QListViewItem *it);
};

#endif
