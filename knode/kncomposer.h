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

#include <qlineedit.h>
#include <qsplitter.h>

#include <ktmainwindow.h>
#include <keditcl.h>

class KNNntpAccount;
class KNSavedArticle;


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
		QCString subject()                    { return QCString(view->subject->text().local8Bit()); }
		QCString destination()                { return d_estination; }
		QCString followUp2();
		int lines()														{ return view->edit->numLines(); }
				
		static void readConfig();
	  				
	protected:
		void closeEvent(QCloseEvent *e);
		void initData();				
	
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
 		void slotDestButtonClicked();
 		void slotFupCheckToggled(bool b);
 		void slotSubjectChanged(const QString &t);			
 		void slotSendNow();
 		void slotSendLater(); 		
 		void slotSaveAsDraft(); 		
 		void slotArtDelete();
  	void slotFileClose();
  	void slotFind();
  	void slotFindNext();
  	void slotReplace();
  	void slotSpellcheck();
  	void slotAppendSig();
  	void slotInsertFile();
  	void slotAttachFile();
   	void slotToggleToolBar();
  	void slotConfKeys();
  	void slotConfToolbar();
  	void slotConfSpellchecker();
					
 	signals:
 		void composerDone(KNComposer*);
							
};


#endif
