/*
    kncomposer.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

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
    enum MessageMode { news=0, mail=1, news_mail=2 };

    // unwraped == original, not rewraped text
    // firstEdit==true: place the cursor at the end of the article
    KNComposer(KNLocalArticle *a, const QString &text=QString::null, const QString &sig=QString::null, const QString &unwraped=QString::null, bool firstEdit=false);
    ~KNComposer();
    void setConfig(bool onlyFonts);
    void setMessageMode(MessageMode mode);

    //get result
    bool hasValidData();
    composerResult result()               { return r_esult; }
    KNLocalArticle* article()             { return a_rticle; }
    void applyChanges();

    // this tells closeEvent() whether it can accept or not:
    void setDoneSuccess(bool b)           { d_oneSuccess = b; }

    void closeEvent(QCloseEvent *e);

    //set data from the given article
    void initData(const QString &text, bool firstEdit);

    // inserts at cursor position if clear is false, replaces content otherwise
    // puts the file content into a box if box==true
    // "file" is already open for reading
    void insertFile(QFile *file, bool clear=false, bool box=false, QString boxTitle=QString::null);

    // ask for a filename, handle network urls
    void insertFile(bool clear=false, bool box=false);

    //internal classes
    class ComposerView;
    class Editor;
    class AttachmentView;
    class AttachmentViewItem;
    class AttachmentPropertiesDlg;

    //GUI
    ComposerView *v_iew;
    QPopupMenu *a_ttPopup, *e_ditPopup;

    //Data
    composerResult r_esult;
    KNLocalArticle *a_rticle;
    QString s_ignature, u_nwraped;
    QCString c_harset;
    MessageMode m_ode;
    bool d_oneSuccess,
         n_eeds8Bit,    // false: fall back to us-ascii
         v_alidated;    // hasValidData was run and found no problems, n_eeds8Bit is valid

    //edit
    bool e_xternalEdited;
    KProcess *e_xternalEditor;
    KTempFile *e_ditorTempfile;
    KSpell *s_pellChecker;

    //Attachments
    QList<KNAttachment> d_elAttList;
    bool a_ttChanged;

  //------------------------------ <Actions> -----------------------------

    KAccel        *a_ccel;
    KAction       *a_ctExternalEditor,
                  *a_ctSpellCheck,
                  *a_ctRemoveAttachment,
                  *a_ctAttachmentProperties,
                  *a_ctPGPsign,
                  *a_ctSetCharsetKeyb;
    KToggleAction *a_ctShowToolbar, *a_ctShowStatusbar,
                  *a_ctDoPost, *a_ctDoMail, *a_ctWordWrap;
    KSelectAction *a_ctSetCharset;

  protected slots:
    void slotSendNow();
    void slotSendLater();     
    void slotSaveAsDraft();   
    void slotArtDelete();
    void slotAppendSig();
    void slotInsertFile();
    void slotInsertFileBoxed();
    void slotAttachFile();
    void slotSignArticle();
    void slotRemoveAttachment();
    void slotAttachmentProperties();
    void slotToggleDoPost();
    void slotToggleDoMail();
    void slotSetCharset(const QString &s);
    void slotSetCharsetKeyboard();
    void slotToggleWordWrap();
    void slotUndoRewrap();
    void slotExternalEditor();
    void slotSpellcheck();
    void slotToggleToolBar();
    void slotToggleStatusBar();
    void slotUpdateStatusBar();
    void slotUpdateCursorPos();
    void slotConfKeys();
    void slotConfToolbar();

  //------------------------------ </Actions> ----------------------------

    // GUI
    void slotSubjectChanged(const QString &t);
    void slotGroupsChanged(const QString &t);
    void slotToBtnClicked();
    void slotGroupsBtnClicked();

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

    void setMessageMode(KNComposer::MessageMode mode);
    void showAttachmentView();
    void hideAttachmentView();
    void showExternalNotification();
    void hideExternalNotification();

    QLabel      *l_to,
                *l_groups,
                *l_fup2;
    QLineEdit   *s_ubject,
                *g_roups,
                *t_o;
    QComboBox   *f_up2;
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

  Q_OBJECT

  public:
    Editor(QWidget *parent=0, char *name=0);
    ~Editor();
    QString textLine(int line) const;

    // inserts s at the current cusor position, deletes the current selection
    void pasteString(const QString &s);

  public slots:
    void slotPasteAsQuotation();
    void slotFind();
    void slotFindNext();
    void slotReplace();
    void slotAddQuotes();
    void slotRemoveQuotes();
    void slotAddBox();
    void slotRemoveBox();
    void slotRot13();

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
