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

#include <kmainwindow.h>
#include <keditcl.h>
#include <kdialogbase.h>

class QGroupBox;
class QComboBox;

class KSpell;
class KProcess;
class KTempFile;
class KEdit;

class KNNntpAccount;
class KNSavedArticle;
class KNMimeContent;
class KNAttachment;


class KNComposer : public KMainWindow  {

  Q_OBJECT
  
  public:
    enum composerResult { CRsendNow, CRsendLater, CRdelAsk,
                          CRdel, CRsave, CRcancel };
                          
    KNComposer(KNSavedArticle *a, const QCString &sig, KNNntpAccount *n=0);
    ~KNComposer();
    static void readConfig();

    //get
    composerResult result()               { return r_esult; }
    KNSavedArticle* article()             { return a_rticle; }
    bool hasValidData();
    bool textChanged();
    //bool attachementsChanged();

    // this tells closeEvent() whether it can accept or not:
    void setDoneSuccess(bool b)           { doneSuccess = b; }
    void setConfig();
    void applyChanges();      
  
            
  protected:
    void closeEvent(QCloseEvent *e);
    void initData();    
    // inserts at cursor position if clear is false, replaces content otherwise
    void insertFile(QString fileName, bool clear=false);
  
    class ComposerView  : public QSplitter {
      
      public:
        ComposerView(QWidget *parent=0, bool mail=false);
        ~ComposerView();
        
        void showAttachmentView();
        void hideAttachmentView();
        void showExternalNotification();
        void hideExternalNotification();
      
        KEdit *edit;
        QGroupBox *notification;
        QPushButton *cancelEditorButton;
        QListView *attView;
        QLineEdit *subject, *dest;
        QComboBox *fup2;
        QCheckBox *fupCheck;
        QPushButton *destButton;
            
    };
    
    class AttachmentItem : public QListViewItem {
    
      public:
        AttachmentItem(QListView *v, KNAttachment *a);
        ~AttachmentItem();
    
        KNAttachment *attachment;
    
    };
        
    
    ComposerView *view;
    QPopupMenu *attPopup;
    KSpell *spellChecker;
    composerResult r_esult;
    KNSavedArticle *a_rticle;
    KNNntpAccount *nntp;
    QCString s_ignature, d_estination;
    bool doneSuccess, externalEdited, attChanged;
    KAction *actExternalEditor, *actSpellCheck,
            *actRemoveAttachment, *actAttachmentProperties;
    KProcess *externalEditor;
    KTempFile *editorTempfile;
    QList<KNAttachment> *delAttList;

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
    void slotRemoveAttachment();
    void slotAttachmentProperties();    
    void slotToggleToolBar();
    void slotConfKeys();
    void slotConfToolbar();
    
    // spellcheck operation
    void slotSpellStarted(KSpell *);
    void slotSpellDone(const QString&);
    void slotSpellFinished();

    // external editor
    void slotEditorFinished(KProcess *);
    void slotCancelEditor();

    // misc slots
    void slotAttachmentPopup(QListViewItem *it, const QPoint &p, int);
    void slotAttachmentSelected(QListViewItem *it);
          
  signals:
    void composerDone(KNComposer*);
              
};



class KNAttachmentPropertyDialog : public KDialogBase {

  Q_OBJECT

  public:
    KNAttachmentPropertyDialog(QWidget *p, KNAttachment *a);
    ~KNAttachmentPropertyDialog();

    void apply();

  protected:
    QLineEdit *mimeType, *description;
    QComboBox *encoding;
    KNAttachment *attachment;
    bool nonTextAsText;

  protected slots:
    void accept();
    void slotMimeTypeTextChanged(const QString &text);
};

#endif
