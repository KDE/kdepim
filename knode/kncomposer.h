/***************************************************************************
                          kncomposer.h  -  description
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


#ifndef KNCOMPOSER_H
#define KNCOMPOSER_H

#include <ktmainwindow.h>
#include <qsplitter.h>
#include <keditcl.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include "knsavedarticle.h"
#include "knnntpaccount.h"


class KNComposer : public KTMainWindow  {

  Q_OBJECT
	
	public:
		enum composerResult { CRsendNow, CRsendLater, CRdelAsk,
													CRdel, CRsave, CRcancel };
													
		KNComposer(KNSavedArticle *a, const QCString &sig, KNNntpAccount *n=0);
		~KNComposer();
		void setConfig();
		
		composerResult result()		{ return r_esult; }
		KNSavedArticle* article()	{ return a_rticle; }
		bool hasValidData();
		
		bool textChanged()										{ return (view->edit->isModified()); }
		bool attachmentsChanged()							{ return attChanged; }
		void bodyContent(KNMimeContent *b);
		QCString subject()
                    { return QCString(view->subject->text().local8Bit()); }
		QCString destination()                { return d_estination; }
		QCString followUp2();
		int lines()														{ return view->edit->numLines(); }
				
		static void readConfig();
	  				
	protected:
		void closeEvent(QCloseEvent *e);
		void initData();
		void appendSignature();
		void attachFile();
				
		QPopupMenu *fileMenu, *editMenu, *appendMenu;
			
		class ComposerView  : public QSplitter {
			
			public:
			 	ComposerView(QWidget *parent=0, bool mail=false);
			 	~ComposerView();
			 	
			 	void showAttachementList();
			
				KEdit *edit;
				QListView *attList;
				QLineEdit *subject, *dest;
				QComboBox *fup2;
				QCheckBox *fupCheck;
				QPushButton *destButton;
		 	
		};
		ComposerView *view;
		composerResult r_esult;
		KNSavedArticle *a_rticle;
		KNNntpAccount *nntp;
		QCString s_ignature, d_estination;
		bool attChanged;
		
		static bool appSig, useViewFnt;
		static int lineLen;
		static QString fntFam;
		
 	protected slots:
 		void slotDestinationChanged(const QString &t);
 		void slotCallback(int id);
 		void slotDestButtonClicked();
 		void slotFupCheckToggled(bool b);
 		void slotSubjectChanged(const QString &t);			
					
 	signals:
 		void composerDone(KNComposer*);
							
};



#define FILE_SEND					100
#define FILE_SEND_LATER		110
#define FILE_SAVE					120
#define FILE_DELETE				130
#define FILE_CLOSE					140
#define EDIT_CUT						200
#define EDIT_COPY					210
#define EDIT_PASTE					220
#define EDIT_SEL_ALL				225
#define EDIT_FIND					230
#define EDIT_REPLACE				240
#define APP_SIG						300
#define APP_FILE						310
#define APP_ATT_FILE				320
#endif
