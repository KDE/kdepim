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
#include <qgroupbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlistview.h>

#include <kmainwindow.h>
#include <keditcl.h>
#include <kdialogbase.h>
#include <kprocess.h>
#include <kspell.h>
#include <ktempfile.h>

class KNLocalArticle;
class KNMimeContent;
class KNAttachment;


class KNComposer : public KMainWindow  {

  Q_OBJECT
  
  public:
    enum composerResult { CRsendNow, CRsendLater, CRdelAsk,
                          CRdel, CRsave, CRcancel };

    // firstEdit==true: place the cursor at the end of the article
    KNComposer(KNLocalArticle *a, const QString &text=QString::null, const QString &sig=QString::null, bool firstEdit=false);
    ~KNComposer();
    void setConfig();

    //get result
    bool hasValidData();
    composerResult result()               { return r_esult; }
    KNLocalArticle* article()             { return a_rticle; }
    void applyChanges();

    // this tells closeEvent() whether it can accept or not:
    void setDoneSuccess(bool b)           { d_oneSuccess = b; }

    void closeEvent(QCloseEvent *e);

    //set data from the given article
    void initData(const QString &text);

    // inserts at cursor position if clear is false, replaces content otherwise
    void insertFile(QString fileName, bool clear=false);

    //internal classes
    class ComposerView;
    class Editor;
    class AttachmentView;
    class AttachmentViewItem;
    class AttachmentPropertiesDlg;

    //GUI
    ComposerView *v_iew;
    QPopupMenu *a_ttPopup;

    //Data
    composerResult r_esult;
    KNLocalArticle *a_rticle;
    QString s_ignature;
    QCString c_harset;
    bool d_oneSuccess;

    //edit
    bool e_xternalEdited;
    KProcess *e_xternalEditor;
    KTempFile *e_ditorTempfile;
    KSpell *s_pellChecker;

    //Attachments
    QList<KNAttachment> d_elAttList;
    bool a_ttChanged;


  //------------------------------ <Actions> -----------------------------

    KAction       *a_ctExternalEditor,
                  *a_ctSpellCheck,
                  *a_ctRemoveAttachment,
                  *a_ctAttachmentProperties;
    KToggleAction *a_ctShowToolbar;
    KSelectAction *a_ctSetCharset;
    

  protected slots:
    void slotSendNow();
    void slotSendLater();     
    void slotSaveAsDraft();   
    void slotArtDelete();
    void slotSetCharset(const QString &s);
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

  //------------------------------ </Actions> ----------------------------

    // GUI
    void slotSubjectChanged(const QString &t);
    void slotGroupsChanged(const QString &t);
    void slotToBtnClicked();
    void slotGroupsBtnClicked();
    void slotToCheckBoxToggled(bool b);
    void slotGroupsCheckBoxToggled(bool b);
    void slotFupCheckBoxToggled(bool b);

    // external editor
    void slotEditorFinished(KProcess *);
    void slotCancelEditor();

    // attachment list
    void slotAttachmentPopup(QListViewItem *it, const QPoint &p, int);
    void slotAttachmentSelected(QListViewItem *it);
    void slotAttachmentEdit(QListViewItem *it);
    void slotAttachmentRemove(QListViewItem *it);

    // spellcheck operation
    void slotSpellStarted(KSpell *);
    void slotSpellDone(const QString&);
    void slotSpellFinished();


  signals:
    void composerDone(KNComposer*);
              
};


class KNComposer::ComposerView  : public QSplitter {

  public:
    ComposerView(QWidget *p=0, const char *n=0);
    ~ComposerView();

    void showAttachmentView();
    void hideAttachmentView();
    void showExternalNotification();
    void hideExternalNotification();

    QLineEdit   *s_ubject,
                *g_roups,
                *t_o;
    QComboBox   *f_up2;
    QCheckBox   *g_roupsCB,
                *t_oCB,
                *f_up2CB;
    QPushButton *g_roupsBtn,
                *t_oBtn;

    Editor      *e_dit;
    QGroupBox   *n_otification;
    QPushButton *c_ancelEditorBtn;


    QWidget         *a_ttWidget;
    AttachmentView  *a_ttView;
    QPushButton     *a_ttAddBtn,
                    *a_ttRemoveBtn,
                    *a_ttEditBtn;

    bool v_iewOpen;
};


//internal class : handle Tabs... (expanding them in textLine(), etc.)
class KNComposer::Editor : public KEdit {

  public:
    Editor(QWidget *parent=0, char *name=0);
    ~Editor();
    QString textLine(int line) const;

  protected:
    bool eventFilter(QObject*, QEvent* e);
};


class KNComposer::AttachmentView : public QListView {

  Q_OBJECT

  public:
    AttachmentView(QWidget *parent, char *name=0);
    ~AttachmentView();

  protected:
    void keyPressEvent( QKeyEvent *e );

  signals:
    void delPressed ( QListViewItem * );      // the user used Key_Delete on a list view item
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
    AttachmentPropertiesDlg( KNAttachment *a, QWidget *p=0, const char *n=0);
    ~AttachmentPropertiesDlg();

    void apply();

  protected:
    QLineEdit *m_imeType,
              *d_escription;
    QComboBox *e_ncoding;

    KNAttachment *a_ttachment;
    bool n_onTextAsText;

  protected slots:
    void accept();
    void slotMimeTypeTextChanged(const QString &text);
};

#endif
