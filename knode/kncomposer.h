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

class QGroupBox;

class KSpell;
class KProcess;
class KTempFile;
class KEdit;

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

    // this tells closeEvent() whether it can accept or not:
    void setDoneSuccess(bool b)           { doneSuccess = b; }
		
    composerResult result()		            { return r_esult; }
    KNSavedArticle* article()	            { return a_rticle; }
    bool hasValidData();
		
    bool textChanged()										{ return (view->edit->isModified() || externalEdited); }
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
		// inserts at cursor position if clear is false, replaces content otherwise
		void insertFile(QString fileName, bool clear=false);
	
		class ComposerView  : public QSplitter {
			
			public:
			 	ComposerView(QWidget *parent=0, bool mail=false);
			 	~ComposerView();
			 	
			 	void showAttachmentList();
			 	void showExternalNotification();
			 	void hideExternalNotification();
			
				KEdit *edit;
				QGroupBox *notification;
				QPushButton *cancelEditorButton;
				QListView *attList;
				QLineEdit *subject, *dest;
				QComboBox *fup2;
				QCheckBox *fupCheck;
				QPushButton *destButton;
		 	
		};
		ComposerView *view;
		KSpell *spellChecker;
		KAction *actSpellCheck;
		composerResult r_esult;
		KNSavedArticle *a_rticle;
		KNNntpAccount *nntp;
		QCString s_ignature, d_estination;
		bool attChanged;
		bool doneSuccess;
		
		bool externalEdited;
		KAction *actExternalEditor;
		KProcess *externalEditor;
		KTempFile *editorTempfile;

		static bool appSig, useViewFnt, useExternalEditor;
		static int lineLen;
		static QString fntFam, externalEditorCommand;
		
 	protected slots:
 		void slotDestinationChanged(const QString &t);
 		void slotDestButtonClicked();
 		void slotFupCheckToggled(bool b);
 		void slotSubjectChanged(const QString &t);			
 		
 		// action slots
 		void slotSendNow();
 		void slotSendLater(); 		
 		void slotSaveAsDraft(); 	
 		void slotArtDelete();
  	void slotFind();
  	void slotFindNext();
  	void slotReplace();
 		void slotExternalEditor();  	
  	void slotSpellcheck();
  	void slotAppendSig();
  	void slotInsertFile();
  	void slotAttachFile();
   	void slotToggleToolBar();
  	void slotConfKeys();
  	void slotConfToolbar();
  	void slotPreferences();
  	
    // spellcheck operation
    void slotSpellStarted(KSpell *);
    void slotSpellDone(const QString&);
    void slotSpellFinished();

    // external editor
    void slotEditorFinished(KProcess *);
    void slotCancelEditor();
					
 	signals:
 		void composerDone(KNComposer*);
							
};


#endif
