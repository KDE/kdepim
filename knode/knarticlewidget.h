/***************************************************************************
                          knarticlewidget.h  -  description
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


#ifndef KNARTICLEWIDGET_H
#define KNARTICLEWIDGET_H

#include <qvbox.h>
#include <qscrollview.h>

#include <kurl.h>
#include <kparts/browserextension.h>
#include <khtml_part.h>

class KNArticle;
class KNArticleCollection;
class KNMimeContent;

class KNArticleWidget : public QVBox  {

  Q_OBJECT
	
	public:
		enum browserType { BTkonqueror=0 , BTnetscape=1 };
		
		KNArticleWidget(QWidget *parent=0, const char *name=0);
		~KNArticleWidget();
		
//=======================================	
		static void readConfig();
		static void updateInstances();
		static KNArticleWidget* find(KNArticle *a);
		static KNArticleWidget* mainWidget();
		static void showArticle(KNArticle *a);
		static void setFullHeaders(bool b);					
		static void toggleFullHeaders();
		static bool fullHeaders();						
//=======================================

    bool scrollingDownPossible();       // needed for "read-through"
    void scrollDown();

		void applyConfig();	
		
		void setData(KNArticle *a, KNArticleCollection *c);			
		void createHtmlPage();
		void showBlankPage();
		void showErrorMessage(const QString &s);

		void updateContents();
				
		KNArticle* article()							{ return a_rticle; }		
		KNArticleCollection* collection()	{ return c_oll; }
		bool htmlDone()										{ return h_tmlDone; }
    KHTMLPart* part() const           { return p_art; }

  protected:
		void focusInEvent(QFocusEvent *e);
		void focusOutEvent(QFocusEvent *e);
		void keyPressEvent(QKeyEvent *e);
		QString toHtmlString(const QString &line, bool parseURLs=true, bool beautification=true);
		void openURL(const QString &url);
		void saveAttachement(const QString &id);
		void openAttachement(const QString &id);
		bool inlinePossible(KNMimeContent *c);		
		KNArticle *a_rticle;	
		KNArticleCollection *c_oll;
		QList<KNMimeContent> *att;			
		bool h_tmlDone;
		QPopupMenu *urlPopup, *attPopup;
		KHTMLPart *p_art;
		QScrollView *view;
					
		static bool showSig, fullHdrs, inlineAtt, openAtt, altAsAtt;
		static QString hexColors[7];
		static QString htmlFont;
		static int htmlFontSize;
		static browserType browser;
		static QList<KNArticleWidget> instances;
			
	protected slots:
		void slotURLRequest (const KURL &url, const KParts::URLArgs &args);
		void slotSelectionChanged();
		void slotPopup(const QString &url, const QPoint &p);
		
	signals:
		void focusChanged(QFocusEvent*);				
				
};

#endif
