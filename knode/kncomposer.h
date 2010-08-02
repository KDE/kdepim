/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNCOMPOSER_H
#define KNCOMPOSER_H

#include <klistview.h>

#include <kmainwindow.h>
#include <kdialogbase.h>
#include <keditcl.h>
#include <tqlineedit.h>
#include <tqregexp.h>

#include <kdeversion.h>
#include <keditcl.h>

#include <kabc/addresslineedit.h>
#include <knodecomposeriface.h>

class TQGroupBox;

class KProcess;
class KSpell;
class KDictSpellingHighlighter;
class KSelectAction;
class KToggleAction;

class KNLocalArticle;
class KNAttachment;
class SpellingFilter;

class KNComposer : public KMainWindow , virtual public KNodeComposerIface {

  Q_OBJECT

  public:
    enum composerResult { CRsendNow, CRsendLater, CRdelAsk,
                          CRdel, CRsave, CRcancel };
    enum MessageMode { news=0, mail=1, news_mail=2 };

    // unwraped == original, not rewraped text
    // firstEdit==true: place the cursor at the end of the article
    KNComposer(KNLocalArticle *a, const TQString &text=TQString::null, const TQString &sig=TQString::null, const TQString &unwraped=TQString::null, bool firstEdit=false, bool dislikesCopies=false, bool createCopy=false);
    ~KNComposer();
    void setConfig(bool onlyFonts);
    void setMessageMode(MessageMode mode);

    //get result
    bool hasValidData();
    composerResult result() const              { return r_esult; }
    KNLocalArticle* article()const             { return a_rticle; }
    bool applyChanges();

    void closeEvent(TQCloseEvent *e);

    //set data from the given article
    void initData(const TQString &text);

    // inserts at cursor position if clear is false, replaces content otherwise
    // puts the file content into a box if box==true
    // "file" is already open for reading
    void insertFile(TQFile *file, bool clear=false, bool box=false, TQString boxTitle=TQString::null);

    // ask for a filename, handle network urls
    void insertFile(bool clear=false, bool box=false);

    TQPopupMenu * popupMenu( const TQString& name );
    int listOfResultOfCheckWord( const TQStringList & lst , const TQString & selectWord);

//internal classes
    class ComposerView;
    class Editor;
    class AttachmentView;
    class AttachmentViewItem;
    class AttachmentPropertiesDlg;

    //GUI
    ComposerView *v_iew;
    TQPopupMenu *a_ttPopup;

    //Data
    composerResult r_esult;
    KNLocalArticle *a_rticle;
    TQString s_ignature, u_nwraped;
    TQCString c_harset;
    MessageMode m_ode;
    bool n_eeds8Bit,    // false: fall back to us-ascii
         v_alidated,    // hasValidData was run and found no problems, n_eeds8Bit is valid
         a_uthorDislikesMailCopies;

    //edit
    bool e_xternalEdited;
    KProcess *e_xternalEditor;
    KTempFile *e_ditorTempfile;
    KSpell *s_pellChecker;
    SpellingFilter* mSpellingFilter;

    //Attachments
    TQValueList<KNAttachment*> mDeletedAttachments;
    TQPtrList<KAction> m_listAction;
    bool a_ttChanged;

  //------------------------------ <Actions> -----------------------------

    KAccel        *a_ccel;
    KAction       *a_ctExternalEditor,
                  *a_ctSpellCheck,
                  *a_ctRemoveAttachment,
                  *a_ctAttachmentProperties,
                  *a_ctSetCharsetKeyb;
    KToggleAction *a_ctPGPsign,
                  *a_ctDoPost, *a_ctDoMail, *a_ctWordWrap;
    KSelectAction *a_ctSetCharset;
    bool spellLineEdit;
  protected slots:
    void slotSendNow();
    void slotSendLater();
    void slotSaveAsDraft();
    void slotArtDelete();
    void slotAppendSig();
    void slotInsertFile();
    void slotInsertFileBoxed();
    void slotAttachFile();
    void slotRemoveAttachment();
    void slotAttachmentProperties();
    void slotToggleDoPost();
    void slotToggleDoMail();
    void slotSetCharset(const TQString &s);
    void slotSetCharsetKeyboard();
    void slotToggleWordWrap();
    void slotUndoRewrap();
    void slotExternalEditor();
    void slotSpellcheck();

    void slotUpdateStatusBar();
    void slotUpdateCursorPos();
    void slotConfKeys();
    void slotConfToolbar();
    void slotNewToolbarConfig();

  //------------------------------ </Actions> ----------------------------

    // GUI
    void slotSubjectChanged(const TQString &t);
    void slotGroupsChanged(const TQString &t);
    void slotToBtnClicked();
    void slotGroupsBtnClicked();

    // external editor
    void slotEditorFinished(KProcess *);
    void slotCancelEditor();

    // attachment list
    void slotAttachmentPopup(KListView*, TQListViewItem *it, const TQPoint &p);
    void slotAttachmentSelected(TQListViewItem *it);
    void slotAttachmentEdit(TQListViewItem *it);
    void slotAttachmentRemove(TQListViewItem *it);

    // spellcheck operation
    void slotSpellStarted(KSpell *);
    void slotSpellDone(const TQString&);
    void slotSpellFinished();

    // DND handling
    virtual void slotDragEnterEvent(TQDragEnterEvent *);
    virtual void slotDropEvent(TQDropEvent *);

    void slotUndo();
    void slotRedo();
    void slotCut();
    void slotCopy();
    void slotPaste();
    void slotSelectAll();
    void slotMisspelling(const TQString &text, const TQStringList &lst, unsigned int pos);
    void slotCorrected (const TQString &oldWord, const TQString &newWord, unsigned int pos);
    void addRecentAddress();

  protected:

    // DND handling
    virtual void dragEnterEvent(TQDragEnterEvent *);
    virtual void dropEvent(TQDropEvent *);

  signals:
    void composerDone(KNComposer*);

  private:
    bool mFirstEdit;

};



class KNLineEditSpell;
class KNLineEdit;

class KNComposer::ComposerView  : public TQSplitter {

  public:
    ComposerView(KNComposer *_composer, const char *n=0);
    ~ComposerView();
    void focusNextPrevEdit(const TQWidget* aCur, bool aNext);
    void setMessageMode(KNComposer::MessageMode mode);
    void showAttachmentView();
    void hideAttachmentView();
    void showExternalNotification();
    void hideExternalNotification();
    void restartBackgroundSpellCheck();
    TQValueList<TQWidget*> mEdtList;

    TQLabel      *l_to,
                *l_groups,
                *l_fup2;
    KNLineEditSpell *s_ubject;

    KNLineEdit   *g_roups;
    KNLineEdit  *t_o;

    KComboBox   *f_up2;
    TQPushButton *g_roupsBtn,
                *t_oBtn;

    Editor      *e_dit;
    TQGroupBox   *n_otification;
    TQPushButton *c_ancelEditorBtn;

    TQWidget         *a_ttWidget;
    AttachmentView  *a_ttView;
    TQPushButton     *a_ttAddBtn,
                    *a_ttRemoveBtn,
                    *a_ttEditBtn;
    KDictSpellingHighlighter *mSpellChecker;

    bool v_iewOpen;
};


//internal class : handle Tabs... (expanding them in textLine(), etc.)
class KNComposer::Editor : public KEdit {

  Q_OBJECT

  public:
    Editor(KNComposer::ComposerView *_composerView, KNComposer *_composer, TQWidget *parent=0, char *name=0);
    ~Editor();
    TQStringList processedText();

  public slots:
    void slotPasteAsQuotation();
    void slotFind();
    void slotSearchAgain();
    void slotReplace();
    void slotAddQuotes();
    void slotRemoveQuotes();
    void slotAddBox();
    void slotRemoveBox();
    void slotRot13();
    void slotCorrectWord();

protected slots:
    void slotSpellStarted( KSpell *);
    void slotSpellDone(const TQString &);
    void slotSpellFinished();
    void slotMisspelling (const TQString &, const TQStringList &lst, unsigned int);
    virtual void cut();
    virtual void clear();
    virtual void del();
    void slotAddSuggestion( const TQString &, const TQStringList &lst, unsigned int );
  signals:
    void sigDragEnterEvent(TQDragEnterEvent *);
    void sigDropEvent(TQDropEvent *);

  protected:

    // DND handling
    virtual void contentsDragEnterEvent(TQDragEnterEvent *);
    virtual void contentsDropEvent(TQDropEvent *);
    virtual void contentsContextMenuEvent( TQContextMenuEvent *e );
    virtual void keyPressEvent ( TQKeyEvent *e);

    virtual bool eventFilter(TQObject*, TQEvent*);
private:
    KNComposer *m_composer;
    KNComposer::ComposerView *m_composerView;
    KSpell *spell;
    TQMap<TQString,TQStringList> m_replacements;
    TQRegExp m_bound;
};


class KNComposer::AttachmentView : public KListView {

  Q_OBJECT

  public:
    AttachmentView(TQWidget *parent, char *name=0);
    ~AttachmentView();

  protected:
    void keyPressEvent( TQKeyEvent *e );

  signals:
    void delPressed ( TQListViewItem * );      // the user used Key_Delete on a list view item
};


class KNComposer::AttachmentViewItem : public KListViewItem {

  public:
    AttachmentViewItem(KListView *v, KNAttachment *a);
    ~AttachmentViewItem();

  KNAttachment *attachment;

};


class KNComposer::AttachmentPropertiesDlg : public KDialogBase {

  Q_OBJECT

  public:
    AttachmentPropertiesDlg( KNAttachment *a, TQWidget *p=0, const char *n=0);
    ~AttachmentPropertiesDlg();

    void apply();

  protected:
    KLineEdit *m_imeType,
              *d_escription;
    TQComboBox *e_ncoding;

    KNAttachment *a_ttachment;
    bool n_onTextAsText;

  protected slots:
    void accept();
    void slotMimeTypeTextChanged(const TQString &text);
};

//-----------------------------------------------------------------------------
class KNLineEdit : public KABC::AddressLineEdit
{
    Q_OBJECT
    typedef KABC::AddressLineEdit KNLineEditInherited;
public:

    KNLineEdit(KNComposer::ComposerView *_composerView, bool useCompletion, TQWidget *parent = 0,
               const char *name = 0);
protected:
    // Inherited. Always called by the parent when this widget is created.
    virtual void loadAddresses();
    void keyPressEvent(TQKeyEvent *e);
    virtual TQPopupMenu *createPopupMenu();
private slots:
    void editRecentAddresses();
private:
    KNComposer::ComposerView *composerView;
};

class KNLineEditSpell : public KNLineEdit
{
    Q_OBJECT
public:
    KNLineEditSpell(KNComposer::ComposerView *_composerView, bool useCompletion,TQWidget * parent, const char * name = 0);
    void highLightWord( unsigned int length, unsigned int pos );
    void spellCheckDone( const TQString &s );
    void spellCheckerMisspelling( const TQString &text, const TQStringList &, unsigned int pos);
    void spellCheckerCorrected( const TQString &old, const TQString &corr, unsigned int pos);
};

#endif
