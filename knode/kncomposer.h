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

class KNNntpAccount;
class KNSavedArticle;
class KNMimeContent;
class KNAttachment;


class KNComposer : public KMainWindow  {

  Q_OBJECT
  
  public:
    enum composerResult { CRsendNow, CRsendLater, CRdelAsk,
                          CRdel, CRsave, CRcancel };

    // firstEdit==true: place the cursor at the end of the article
    // n==0: eMail
    KNComposer(KNSavedArticle *a, const QCString &sig, bool firstEdit=false, KNNntpAccount *n=0);
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

    virtual QSize sizeHint() const;   // useful default value
            
  protected:
    void closeEvent(QCloseEvent *e);
    void initData();    
    // inserts at cursor position if clear is false, replaces content otherwise
    void insertFile(QString fileName, bool clear=false);

    class Editor : public KEdit {      // handle Tabs... (expanding them in textLine(), etc.)

      public:
        Editor(QWidget *parent=0, char *name=0);
        ~Editor();

        QString textLine(int line) const;

      protected:
        bool eventFilter(QObject*, QEvent* e);
    };

    class AttachmentView;
    class AttachmentViewItem;
    class AttachmentPropertiesDlg;

    class ComposerView  : public QSplitter {
      
      public:
        ComposerView(QWidget *parent=0, bool mail=false);
        ~ComposerView();
        
        void showAttachmentView();
        void hideAttachmentView();
        void showExternalNotification();
        void hideExternalNotification();

        void saveOptions();
      
        Editor *edit;
        QGroupBox *notification;
        QPushButton *cancelEditorButton, *attRemoveButton, *attEditButton;
        QWidget *attWidget;
        AttachmentView *attView;
        bool viewOpen;
        QLineEdit *subject, *dest;
        QComboBox *fup2;
        QCheckBox *fupCheck;
        QPushButton *destButton;
            
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
    KToggleAction *actShowToolbar;
    KProcess *externalEditor;
    KTempFile *editorTempfile;
    QList<KNAttachment> *delAttList;

    static bool appSig, useExternalEditor;
    static int lineLen;
    static QString externalEditorCommand;
    
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
    void slotSaveOptions();
    void slotConfKeys();
    void slotConfToolbar();
    
    // spellcheck operation
    void slotSpellStarted(KSpell *);
    void slotSpellDone(const QString&);
    void slotSpellFinished();

    // external editor
    void slotEditorFinished(KProcess *);
    void slotCancelEditor();

    // attachment list view
    void slotAttachmentPopup(QListViewItem *it, const QPoint &p, int);
    void slotAttachmentSelected(QListViewItem *it);
    void slotAttachmentEdit(QListViewItem *it);
    void slotAttachmentRemove(QListViewItem *it);
          
  signals:
    void composerDone(KNComposer*);
              
};



// === attachment handling ===========================================================


class KNComposer::AttachmentView : public QListView {

  Q_OBJECT

  public:
    AttachmentView(QWidget *parent, char *name=0);
    ~AttachmentView();

  protected:
    void keyPressEvent( QKeyEvent *e );

  signals:
    void delPressed ( QListViewItem * );      // the user used Key_Delete on list view item
};


class KNComposer::AttachmentViewItem : public QListViewItem {

  public:
    AttachmentViewItem(QListView *v, KNAttachment *a);
    ~AttachmentViewItem();

  KNAttachment *attachment;

};


class KNComposer::AttachmentPropertiesDlg : public KDialogBase {

  Q_OBJECT

  public:
    AttachmentPropertiesDlg(QWidget *p, KNAttachment *a);
    ~AttachmentPropertiesDlg();

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
