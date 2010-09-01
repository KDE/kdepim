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

#include <tqheader.h>
#include <tqtextcodec.h>
#include <tqclipboard.h>
#include <tqapplication.h>
#include <kspelldlg.h>
#include <kdeversion.h>
#include "addressesdialog.h"
using KPIM::AddressesDialog;
#include "recentaddresses.h"
using KRecentAddress::RecentAddresses;
#include <kaccel.h>
#include <kcharsets.h>
#include <kmessagebox.h>
#include <kabc/addresseedialog.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kpopupmenu.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kspell.h>
#include <ktempfile.h>
#include <kpgp.h>
#include <kpgpblock.h>
#include <kprocess.h>
#include <kqcstringsplitter.h>
#include <ksyntaxhighlighter.h>
#include <tqcursor.h>
#include <kurldrag.h>
#include <kcompletionbox.h>

#include <kapplication.h>
#include "kngroupselectdialog.h"
#include "utilities.h"
#include "knglobals.h"
#include "kncomposer.h"
#include "knmainwidget.h"
#include "knconfigmanager.h"
#include "knaccountmanager.h"
#include "knnntpaccount.h"
#include "knarticlefactory.h"
#include <kstatusbar.h>
#include <klocale.h>
#include <tqpopupmenu.h>
#include <spellingfilter.h>
#include <kstdguiitem.h>

KNLineEdit::KNLineEdit(KNComposer::ComposerView *_composerView, bool useCompletion,
                       TQWidget *parent, const char *name)
    : KNLineEditInherited(parent,useCompletion,name)    , composerView(_composerView)

{
}


TQPopupMenu *KNLineEdit::createPopupMenu()
{
    TQPopupMenu *menu = KLineEdit::createPopupMenu();
    if ( !menu )
        return 0;

    menu->insertSeparator();
    menu->insertItem( i18n( "Edit Recent Addresses..." ),
                      this, TQT_SLOT( editRecentAddresses() ) );

    return menu;
}

void KNLineEdit::editRecentAddresses()
{
  KRecentAddress::RecentAddressDialog dlg( this );
  dlg.setAddresses( RecentAddresses::self( knGlobals.config() )->addresses() );
  if ( dlg.exec() ) {
    RecentAddresses::self( knGlobals.config() )->clear();
    TQStringList addrList = dlg.addresses();
    TQStringList::Iterator it;
    for ( it = addrList.begin(); it != addrList.end(); ++it )
      RecentAddresses::self( knGlobals.config() )->add( *it );

    loadAddresses();
  }
}

void KNLineEdit::loadAddresses()
{
    KNLineEditInherited::loadAddresses();

    TQStringList recent = RecentAddresses::self(knGlobals.config())->addresses();
    TQStringList::Iterator it = recent.begin();
    for ( ; it != recent.end(); ++it )
        addAddress( *it );
}

void KNLineEdit::keyPressEvent(TQKeyEvent *e)
{
    // ---sven's Return is same Tab and arrow key navigation start ---
    if ((e->key() == Key_Enter || e->key() == Key_Return) &&
        !completionBox()->isVisible())
    {
        composerView->focusNextPrevEdit( this, true );
      return;
    }
    if (e->key() == Key_Up)
    {
        composerView->focusNextPrevEdit( this, false ); // Go up
      return;
    }
    if (e->key() == Key_Down)
    {
        composerView->focusNextPrevEdit( this, true ); // Go down
      return;
    }
    // ---sven's Return is same Tab and arrow key navigation end ---
  KNLineEditInherited::keyPressEvent(e);
}

KNLineEditSpell::KNLineEditSpell( KNComposer::ComposerView *_composerView, bool useCompletion,TQWidget * parent, const char * name)
    :KNLineEdit( _composerView, useCompletion, parent,name )
{
}

void KNLineEditSpell::highLightWord( unsigned int length, unsigned int pos )
{
    setSelection ( pos, length );
}

void KNLineEditSpell::spellCheckDone( const TQString &s )
{
    if( s != text() )
	setText( s );
}

void KNLineEditSpell::spellCheckerMisspelling( const TQString &_text, const TQStringList &, unsigned int pos)
{
     highLightWord( _text.length(),pos );
}

void KNLineEditSpell::spellCheckerCorrected( const TQString &old, const TQString &corr, unsigned int pos)
{
    if( old!= corr )
    {
        setSelection ( pos, old.length() );
        insert( corr );
        setSelection ( pos, corr.length() );
    }
}


KNComposer::KNComposer(KNLocalArticle *a, const TQString &text, const TQString &sig, const TQString &unwraped, bool firstEdit, bool dislikesCopies, bool createCopy)
    : KMainWindow(0,"composerWindow"), r_esult(CRsave), a_rticle(a), s_ignature(sig), u_nwraped(unwraped),
      n_eeds8Bit(true), v_alidated(false), a_uthorDislikesMailCopies(dislikesCopies), e_xternalEdited(false), e_xternalEditor(0),
      e_ditorTempfile(0), s_pellChecker(0), a_ttChanged(false),
      mFirstEdit( firstEdit )
{
    mSpellingFilter = 0;
    spellLineEdit = false;
    m_listAction.setAutoDelete( true );

  if(knGlobals.instance)
    setInstance(knGlobals.instance);

  // activate dnd of attachments...
  setAcceptDrops(true);

  //init v_iew
  v_iew=new ComposerView(this);
  setCentralWidget(v_iew);

  connect(v_iew->c_ancelEditorBtn, TQT_SIGNAL(clicked()), TQT_SLOT(slotCancelEditor()));
  connect(v_iew->e_dit, TQT_SIGNAL(sigDragEnterEvent(TQDragEnterEvent *)), TQT_SLOT(slotDragEnterEvent(TQDragEnterEvent *)));
  connect(v_iew->e_dit, TQT_SIGNAL(sigDropEvent(TQDropEvent *)), TQT_SLOT(slotDropEvent(TQDropEvent *)));

  //statusbar
  KStatusBar *sb=statusBar();
  sb->insertItem(TQString::null, 1,1);                 // type
  sb->setItemAlignment (1,AlignLeft | AlignVCenter);
  sb->insertItem(TQString::null, 2,1);                 // charset
  sb->setItemAlignment (2,AlignLeft | AlignVCenter);
  sb->insertItem(TQString::null, 3,0);                 // column
  sb->setItemAlignment (3,AlignCenter | AlignVCenter);
  sb->insertItem(TQString::null, 4,0);                 // column
  sb->setItemAlignment (4,AlignCenter | AlignVCenter);
  sb->insertItem(TQString::null, 5,0);                 // line
  sb->setItemAlignment (5,AlignCenter | AlignVCenter);
  connect(v_iew->e_dit, TQT_SIGNAL(CursorPositionChanged()), TQT_SLOT(slotUpdateCursorPos()));
  connect(v_iew->e_dit, TQT_SIGNAL(toggle_overwrite_signal()), TQT_SLOT(slotUpdateStatusBar()));

  //------------------------------- <Actions> --------------------------------------

  //file menu
  new KAction(i18n("&Send Now"),"mail_send", CTRL + Key_Return , this,
    TQT_SLOT(slotSendNow()), actionCollection(), "send_now");

  new KAction(i18n("Send &Later"), "queue", 0, this,
    TQT_SLOT(slotSendLater()), actionCollection(), "send_later");

  new KAction(i18n("Save as &Draft"),"filesave", 0 , this,
    TQT_SLOT(slotSaveAsDraft()), actionCollection(), "save_as_draft");

  new KAction(i18n("D&elete"),"editdelete", 0 , this,
    TQT_SLOT(slotArtDelete()), actionCollection(), "art_delete");

  KStdAction::close(this, TQT_SLOT(close()),actionCollection());

  //edit menu
  KStdAction::undo(this, TQT_SLOT(slotUndo()), actionCollection());
  KStdAction::redo(this, TQT_SLOT(slotRedo()), actionCollection());

  KStdAction::cut(this, TQT_SLOT(slotCut()), actionCollection());


  KStdAction::copy(this, TQT_SLOT(slotCopy()), actionCollection());

  KStdAction::pasteText(this, TQT_SLOT(slotPaste()), actionCollection());

  new KAction(i18n("Paste as &Quotation"), 0, v_iew->e_dit,
                   TQT_SLOT(slotPasteAsQuotation()), actionCollection(), "paste_quoted");

  KStdAction::selectAll(this, TQT_SLOT(slotSelectAll()), actionCollection());

  KStdAction::find(v_iew->e_dit, TQT_SLOT(slotFind()), actionCollection());
  KStdAction::findNext(v_iew->e_dit, TQT_SLOT(slotSearchAgain()), actionCollection());

  KStdAction::replace(v_iew->e_dit, TQT_SLOT(slotReplace()), actionCollection());

  //attach menu
  new KAction(i18n("Append &Signature"), 0 , this, TQT_SLOT(slotAppendSig()),
                   actionCollection(), "append_signature");

  new KAction(i18n("&Insert File..."), 0, this, TQT_SLOT(slotInsertFile()),
                   actionCollection(), "insert_file");

  new KAction(i18n("Insert File (in a &box)..."), 0, this, TQT_SLOT(slotInsertFileBoxed()),
                   actionCollection(), "insert_file_boxed");

  new KAction(i18n("Attach &File..."), "attach", 0, this, TQT_SLOT(slotAttachFile()),
                   actionCollection(), "attach_file");

  a_ctPGPsign = new KToggleAction(i18n("Sign Article with &PGP"),
		   "signature", 0,
                   actionCollection(), "sign_article");

  a_ctRemoveAttachment = new KAction(i18n("&Remove"), 0, this,
                                    TQT_SLOT(slotRemoveAttachment()), actionCollection(), "remove_attachment");

  a_ctAttachmentProperties  = new KAction(i18n("&Properties"), 0, this,
                                          TQT_SLOT(slotAttachmentProperties()), actionCollection(), "attachment_properties");

  //options menu

  a_ctDoPost = new KToggleAction(i18n("Send &News Article"), "filenew", 0 , this,
                   TQT_SLOT(slotToggleDoPost()), actionCollection(), "send_news");

  a_ctDoMail = new KToggleAction(i18n("Send E&mail"), "mail_generic" , 0 , this,
                   TQT_SLOT(slotToggleDoMail()), actionCollection(), "send_mail");

  a_ctSetCharset = new KSelectAction(i18n("Set &Charset"), 0, actionCollection(), "set_charset");
  a_ctSetCharset->setItems(knGlobals.configManager()->postNewsTechnical()->composerCharsets());
  a_ctSetCharset->setShortcutConfigurable(false);
  connect(a_ctSetCharset, TQT_SIGNAL(activated(const TQString&)),
  this, TQT_SLOT(slotSetCharset(const TQString&)));

  a_ctSetCharsetKeyb = new KAction(i18n("Set Charset"), 0, this,
                                   TQT_SLOT(slotSetCharsetKeyboard()), actionCollection(), "set_charset_keyboard");


  a_ctWordWrap  = new KToggleAction(i18n("&Word Wrap"), 0 , this,
                      TQT_SLOT(slotToggleWordWrap()), actionCollection(), "toggle_wordwrap");

  //tools menu

  new KAction(i18n("Add &Quote Characters"), 0, v_iew->e_dit,
              TQT_SLOT(slotAddQuotes()), actionCollection(), "tools_quote");

  new KAction(i18n("&Remove Quote Characters"), 0, v_iew->e_dit,
              TQT_SLOT(slotRemoveQuotes()), actionCollection(), "tools_unquote");

  new KAction(i18n("Add &Box"), 0, v_iew->e_dit,
              TQT_SLOT(slotAddBox()), actionCollection(), "tools_box");

  new KAction(i18n("Re&move Box"), 0, v_iew->e_dit,
              TQT_SLOT(slotRemoveBox()), actionCollection(), "tools_unbox");

  KAction *undoRewrap = new KAction(i18n("Get &Original Text (not re-wrapped)"), 0, this,
                                    TQT_SLOT(slotUndoRewrap()), actionCollection(), "tools_undoRewrap");
  undoRewrap->setEnabled(!u_nwraped.isNull());

  KAction *rot13 = new KAction(i18n("S&cramble (Rot 13)"), "encrypted", 0, v_iew->e_dit,
                               TQT_SLOT(slotRot13()), actionCollection(), "tools_rot13");
  rot13->setEnabled(false);
  connect(v_iew->e_dit, TQT_SIGNAL(copyAvailable(bool)), rot13, TQT_SLOT(setEnabled(bool)));

  a_ctExternalEditor = new KAction(i18n("Start &External Editor"), "run", 0, this,
                       TQT_SLOT(slotExternalEditor()), actionCollection(), "external_editor");

  a_ctSpellCheck = KStdAction::spelling (this, TQT_SLOT(slotSpellcheck()), actionCollection());

  //settings menu
  createStandardStatusBarAction();
  setStandardToolBarMenuEnabled(true);

  KStdAction::keyBindings(this, TQT_SLOT(slotConfKeys()), actionCollection());

  KStdAction::configureToolbars(this, TQT_SLOT(slotConfToolbar()), actionCollection());

  KStdAction::preferences(knGlobals.top, TQT_SLOT(slotSettings()), actionCollection());

  a_ccel=new KAccel(this);
  a_ctSetCharsetKeyb->plugAccel(a_ccel);

  createGUI("kncomposerui.rc",  false);

  //---------------------------------- </Actions> ----------------------------------------


  //attachment popup
  a_ttPopup=static_cast<TQPopupMenu*> (factory()->container("attachment_popup", this));
  if(!a_ttPopup) a_ttPopup = new TQPopupMenu();
  slotAttachmentSelected(0);

  //init
  initData(text);

  //apply configuration
  setConfig(false);

  if (firstEdit) {   // now we place the cursor at the end of the quoted text / below the attribution line
    if (knGlobals.configManager()->postNewsComposer()->cursorOnTop()) {
      int numLines = knGlobals.configManager()->postNewsComposer()->intro().contains("%L");
      v_iew->e_dit->setCursorPosition(numLines+1,0);
    }
    else
      v_iew->e_dit->setCursorPosition(v_iew->e_dit->numLines()-1,0);
  } else
    v_iew->e_dit->setCursorPosition(0,0);

  v_iew->e_dit->setFocus();

  if (v_iew->s_ubject->text().length() == 0) {
    v_iew->s_ubject->setFocus();
  }

  if (v_iew->g_roups->text().length() == 0 && m_ode == news) {
    v_iew->g_roups->setFocus();
  }

  if (v_iew->t_o->text().length() == 0 && m_ode == mail) {
    v_iew->t_o->setFocus();
  }

  if(firstEdit && knGlobals.configManager()->postNewsComposer()->appendOwnSignature())
    slotAppendSig();

  if (createCopy && (m_ode==news)) {
    a_ctDoMail->setChecked(true);
    slotToggleDoMail();
  }

  v_iew->e_dit->setModified(false);

  // restore window & toolbar configuration
  KConfig *conf = knGlobals.config();
  conf->setGroup("composerWindow_options");
  resize(535,450);    // default optimized for 800x600
  applyMainWindowSettings(conf);

  // starting the external editor
  if(knGlobals.configManager()->postNewsComposer()->useExternalEditor())
    slotExternalEditor();
}


KNComposer::~KNComposer()
{
  delete s_pellChecker;
  delete mSpellingFilter;
  delete e_xternalEditor;  // this also kills the editor process if it's still running

  if(e_ditorTempfile) {
    e_ditorTempfile->unlink();
    delete e_ditorTempfile;
  }

  for ( TQValueList<KNAttachment*>::Iterator it = mDeletedAttachments.begin(); it != mDeletedAttachments.end(); ++it )
    delete (*it);

  KConfig *conf = knGlobals.config();
  conf->setGroup("composerWindow_options");
  saveMainWindowSettings(conf);
}

int KNComposer::listOfResultOfCheckWord( const TQStringList & lst , const TQString & selectWord)
{
    createGUI("kncomposerui.rc",  false);
    unplugActionList("spell_result" );
    m_listAction.clear();
    if ( !lst.contains( selectWord ) )
    {
        TQStringList::ConstIterator it = lst.begin();
        for ( ; it != lst.end() ; ++it )
        {
            if ( !(*it).isEmpty() ) // in case of removed subtypes or placeholders
            {
                KAction * act = new KAction( *it );

                connect( act, TQT_SIGNAL(activated()), v_iew->e_dit, TQT_SLOT(slotCorrectWord()) );
                m_listAction.append( act );
            }
        }
    }
    if ( m_listAction.count()>0 )
        plugActionList("spell_result", m_listAction );
    return m_listAction.count();
}

void KNComposer::slotUndo()
{
    TQWidget* fw = focusWidget();
    if (!fw) return;

    if (fw->inherits("KEdit"))
        ((TQMultiLineEdit*)fw)->undo();
    else if (fw->inherits("TQLineEdit"))
        ((TQLineEdit*)fw)->undo();
}

void KNComposer::slotRedo()
{
    TQWidget* fw = focusWidget();
    if (!fw) return;

    if (fw->inherits("KEdit"))
        ((TQMultiLineEdit*)fw)->redo();
    else if (fw->inherits("TQLineEdit"))
        ((TQLineEdit*)fw)->redo();
}

void KNComposer::slotCut()
{
  TQWidget* fw = focusWidget();
  if (!fw) return;

  if (fw->inherits("KEdit"))
    ((TQMultiLineEdit*)fw)->cut();
  else if (fw->inherits("TQLineEdit"))
    ((TQLineEdit*)fw)->cut();
  else kdDebug(5003) << "wrong focus widget" << endl;
}

void KNComposer::slotCopy()
{
  TQWidget* fw = focusWidget();
  if (!fw) return;

  if (fw->inherits("KEdit"))
    ((TQMultiLineEdit*)fw)->copy();
  else if (fw->inherits("TQLineEdit"))
    ((TQLineEdit*)fw)->copy();
  else kdDebug(5003) << "wrong focus widget" << endl;

}


void KNComposer::slotPaste()
{
  TQWidget* fw = focusWidget();
  if (!fw) return;

  if (fw->inherits("KEdit"))
    ((TQMultiLineEdit*)fw)->paste();
  else if (fw->inherits("TQLineEdit"))
    ((TQLineEdit*)fw)->paste();
  else kdDebug(5003) << "wrong focus widget" << endl;
}

void KNComposer::slotSelectAll()
{
  TQWidget* fw = focusWidget();
  if (!fw) return;

  if (fw->inherits("TQLineEdit"))
      ((TQLineEdit*)fw)->selectAll();
  else if (fw->inherits("TQMultiLineEdit"))
    ((TQMultiLineEdit*)fw)->selectAll();
}


void KNComposer::setConfig(bool onlyFonts)
{
  if (!onlyFonts) {
    v_iew->e_dit->setWordWrap(knGlobals.configManager()->postNewsComposer()->wordWrap()?
                              TQMultiLineEdit::FixedColumnWidth : TQMultiLineEdit::NoWrap);
    v_iew->e_dit->setWrapColumnOrWidth(knGlobals.configManager()->postNewsComposer()->maxLineLength());
    a_ctWordWrap->setChecked(knGlobals.configManager()->postNewsComposer()->wordWrap());

    Kpgp::Module *pgp = Kpgp::Module::getKpgp();
    a_ctPGPsign->setEnabled(pgp->usePGP());
  }

  TQFont fnt=knGlobals.configManager()->appearance()->composerFont();
  v_iew->s_ubject->setFont(fnt);
  v_iew->t_o->setFont(fnt);
  v_iew->g_roups->setFont(fnt);
  v_iew->f_up2->setFont(fnt);
  v_iew->e_dit->setFont(fnt);

  slotUpdateStatusBar();
}


void KNComposer::setMessageMode(MessageMode mode)
{
  m_ode = mode;
  a_ctDoPost->setChecked(m_ode!=mail);
  a_ctDoMail->setChecked(m_ode!=news);
  v_iew->setMessageMode(m_ode);

  if (m_ode == news_mail) {
    TQString s = v_iew->e_dit->textLine(0);
    if (!s.contains(i18n("<posted & mailed>")))
      v_iew->e_dit->insertAt(i18n("<posted & mailed>\n\n"),0,0);
  } else {
    if (v_iew->e_dit->textLine(0)==i18n("<posted & mailed>")) {
      v_iew->e_dit->removeLine(0);
      if (v_iew->e_dit->textLine(0).isEmpty())
        v_iew->e_dit->removeLine(0);
    }
  }

  slotUpdateStatusBar();
}


bool KNComposer::hasValidData()
{
  v_alidated=false;
  n_eeds8Bit=false;

  // header checks

  if (v_iew->s_ubject->text().isEmpty()) {
    KMessageBox::sorry(this, i18n("Please enter a subject."));
    return false;
  }
  if (!n_eeds8Bit && !KMime::isUsAscii(v_iew->s_ubject->text()))
    n_eeds8Bit=true;

  if (m_ode != mail) {
    if (v_iew->g_roups->text().isEmpty()) {
      KMessageBox::sorry(this, i18n("Please enter a newsgroup."));
      return false;
    }

    int groupCount = TQStringList::split(',',v_iew->g_roups->text()).count();
    int fupCount = TQStringList::split(',',v_iew->f_up2->currentText()).count();
    bool followUp = !v_iew->f_up2->currentText().isEmpty();

    if (groupCount>12) {
      KMessageBox::sorry(this, i18n("You are crossposting to more than 12 newsgroups.\nPlease remove all newsgroups in which your article is off-topic."));
      return false;
    }

    if (groupCount>5)
      if (!(KMessageBox::warningYesNo( this, i18n("You are crossposting to more than five newsgroups.\nPlease reconsider whether this is really useful\nand remove groups in which your article is off-topic.\nDo you want to re-edit the article or send it anyway?"),
                                       TQString::null, i18n("&Send"),i18n("edit article","&Edit")) == KMessageBox::Yes))
        return false;

    if ( !followUp && groupCount > 2 ) {
      if ( KMessageBox::warningYesNo( this,
           i18n("You are crossposting to more than two newsgroups.\n"
                "Please use the \"Followup-To\" header to direct the replies "
                "to your article into one group.\n"
                "Do you want to re-edit the article or send it anyway?"),
           TQString::null, i18n("&Send"), i18n("edit article","&Edit"), "missingFollowUpTo" )
           != KMessageBox::Yes )
        return false;
    }

    if (fupCount>12) {
      KMessageBox::sorry(this, i18n("You are directing replies to more than 12 newsgroups.\nPlease remove some newsgroups from the \"Followup-To\" header."));
      return false;
    }

    if (fupCount>5)
      if (!(KMessageBox::warningYesNo( this, i18n("You are directing replies to more than five newsgroups.\nPlease reconsider whether this is really useful.\nDo you want to re-edit the article or send it anyway?"),
                                       TQString::null, i18n("&Send"),i18n("edit article","&Edit")) == KMessageBox::Yes))
        return false;
  }

  if (m_ode != news) {
    if (v_iew->t_o->text().isEmpty() ) {
      KMessageBox::sorry(this, i18n("Please enter the email address."));
      return false;
    }
    if (!n_eeds8Bit && !KMime::isUsAscii(v_iew->t_o->text()))
      n_eeds8Bit=true;
  }

  //GNKSA body checks
  bool firstLine = true;
  bool empty = true;
  bool longLine = false;
  bool hasAttributionLine = false;
  int sigLength = 0;
  int notQuoted = 0;
  int textLines = 0;
  TQStringList text = v_iew->e_dit->processedText();

  for (TQStringList::Iterator it = text.begin(); it != text.end(); ++it) {

    if (!n_eeds8Bit && !KMime::isUsAscii(*it))
      n_eeds8Bit=true;

    if (*it == "-- ") {   // signature text
      for (++it; it != text.end(); ++it) {

        if (!n_eeds8Bit && !KMime::isUsAscii(*it))
          n_eeds8Bit=true;

        sigLength++;
        if((*it).length()>80) {
          longLine = true;
        }
      }
      break;
    }

    if(!(*it).isEmpty()) {
      empty = false;
      textLines++;
      if ((*it)[0]!='>') {
        notQuoted++;
        if (firstLine) hasAttributionLine = true;
      }
    }
    if((*it).length()>80) {
      longLine = true;
    }

    firstLine = false;
  }

  if (n_eeds8Bit && (c_harset.lower()=="us-ascii")) {
    KMessageBox::sorry(this, i18n("Your message contains characters which are not included\nin the \"us-ascii\" character set; please choose\na suitable character set from the \"Options\" menu."));
    return false;
  }

  if (empty) {
    KMessageBox::sorry(this, i18n("You cannot post an empty message."));
    return false;
  }

  if ((textLines>1)&&(notQuoted==1)) {
    if (hasAttributionLine)
      if (!(KMessageBox::warningYesNo( this, i18n("Your article seems to consist entirely of quoted text;\ndo you want to re-edit the article or send it anyway?"),
                                       TQString::null, i18n("&Send"),i18n("edit article","&Edit")) == KMessageBox::Yes))
        return false;
  } else {
    if (notQuoted==0) {
      KMessageBox::sorry(this, i18n("You cannot post an article consisting\n"
			      "entirely of quoted text."));
      return false;
    }
  }

  if (longLine)
    if (!(KMessageBox::warningYesNo( this,
          i18n("Your article contains lines longer than 80 characters.\n"
	       "Do you want to re-edit the article or send it anyway?"),
          TQString::null, i18n("&Send"),
	  i18n("edit article","&Edit")) == KMessageBox::Yes))
      return false;

  if (sigLength>8) {
    if (!(KMessageBox::warningYesNo( this, i18n("Your signature is more than 8 lines long.\nYou should shorten it to match the widely accepted limit of 4 lines.\nDo you want to re-edit the article or send it anyway?"),
                                     TQString::null, i18n("&Send"),i18n("edit article","&Edit")) == KMessageBox::Yes))
      return false;
  } else
    if (sigLength>4)
       KMessageBox::information(this, i18n("Your signature exceeds the widely-accepted limit of 4 lines:\nplease consider shortening your signature;\notherwise, you will probably annoy your readers."),
                                TQString::null,"longSignatureWarning");

  // check if article can be signed
  if ( a_ctPGPsign->isChecked() ) {
    // try to get the signing key
    TQCString signingKey = knGlobals.configManager()->identity()->signingKey();
    KNNntpAccount *acc = knGlobals.accountManager()->account( a_rticle->serverId() );
    if ( acc ) {
      KMime::Headers::Newsgroups *grps = a_rticle->newsgroups();
      KNGroup *grp = knGlobals.groupManager()->group( grps->firstGroup(), acc );
      if (grp && grp->identity())
        signingKey = grp->identity()->signingKey();
      else if (acc->identity())
        signingKey = acc->identity()->signingKey();
    }

    // the article can only be signed if we have a key
    if (signingKey.isEmpty()) {
          if ( KMessageBox::warningContinueCancel( this,
                   i18n("You have not configured your preferred "
                        "signing key yet;\n"
                        "please specify it in the global "
                        "identity configuration,\n"
                        "in the account properties or in the "
                        "group properties.\n"
                        "The article will be sent unsigned." ),
                   TQString::null, i18n( "Send Unsigned" ),
                   "sendUnsignedDialog" )
               == KMessageBox::Cancel )
             return false;
    }
  }

  v_alidated=true;
  return true;
}


bool KNComposer::applyChanges()
{
  KMime::Content *text=0;
  KNAttachment *a=0;

  //Date
  a_rticle->date()->setUnixTime();    //set current date+time

  //Subject
  a_rticle->subject()->fromUnicodeString(v_iew->s_ubject->text(), c_harset);

  //Newsgroups
  if (m_ode != mail) {
    a_rticle->newsgroups()->fromUnicodeString(v_iew->g_roups->text().remove(TQRegExp("\\s")), KMime::Headers::Latin1);
    a_rticle->setDoPost(true);
  } else
    a_rticle->setDoPost(false);

  //To
  if (m_ode != news) {
    a_rticle->to()->fromUnicodeString(v_iew->t_o->text(), c_harset);
    a_rticle->setDoMail(true);
  } else
    a_rticle->setDoMail(false);

  //Followup-To
  if( a_rticle->doPost() && !v_iew->f_up2->currentText().isEmpty())
    a_rticle->followUpTo()->fromUnicodeString(v_iew->f_up2->currentText(), KMime::Headers::Latin1);
  else
    a_rticle->removeHeader("Followup-To");

  if(a_ttChanged && (v_iew->a_ttView)) {

    TQListViewItemIterator it(v_iew->a_ttView);
    while(it.current()) {
      a=(static_cast<AttachmentViewItem*> (it.current()))->attachment;
      if(a->hasChanged()) {
        if(a->isAttached())
          a->updateContentInfo();
        else
          a->attach(a_rticle);
      }
      ++it;
    }
  }

  for ( TQValueList<KNAttachment*>::Iterator it = mDeletedAttachments.begin(); it != mDeletedAttachments.end(); ++it )
    if ( (*it)->isAttached() )
      (*it)->detach( a_rticle );

  text=a_rticle->textContent();

  if(!text) {
    text=new KMime::Content();
    KMime::Headers::ContentType *type=text->contentType();
    KMime::Headers::CTEncoding *enc=text->contentTransferEncoding();
    type->setMimeType("text/plain");
    enc->setDecoded(true);
    text->assemble();
    a_rticle->addContent(text, true);
  }

  //set text
  KNConfig::PostNewsTechnical *pnt=knGlobals.configManager()->postNewsTechnical();
  if (v_alidated) {
    if (n_eeds8Bit) {
      text->contentType()->setCharset(c_harset);
      if (pnt->allow8BitBody())
        text->contentTransferEncoding()->setCte(KMime::Headers::CE8Bit);
      else
        text->contentTransferEncoding()->setCte(KMime::Headers::CEquPr);
    } else {
      text->contentType()->setCharset("us-ascii");   // fall back to us-ascii
      text->contentTransferEncoding()->setCte(KMime::Headers::CE7Bit);
    }
  } else {             // save as draft
    text->contentType()->setCharset(c_harset);
    if (c_harset.lower()=="us-ascii")
      text->contentTransferEncoding()->setCte(KMime::Headers::CE7Bit);
    else
      text->contentTransferEncoding()->setCte(pnt->allow8BitBody()? KMime::Headers::CE8Bit : KMime::Headers::CEquPr);
  }

  //assemble the text line by line
  TQString tmp;
  TQStringList textLines = v_iew->e_dit->processedText();
  for (TQStringList::Iterator it = textLines.begin(); it != textLines.end(); ++it)
    tmp += *it + "\n";

  // Sign article if needed
  if ( a_ctPGPsign->isChecked() ) {
      // first get the signing key
      TQCString signingKey = knGlobals.configManager()->identity()->signingKey();
      KNNntpAccount *acc = knGlobals.accountManager()->account( a_rticle->serverId() );
      if ( acc ) {
          KMime::Headers::Newsgroups *grps = a_rticle->newsgroups();
          KNGroup *grp = knGlobals.groupManager()->group( grps->firstGroup(), acc );
          if (grp && grp->identity())
              signingKey = grp->identity()->signingKey();
          else if (acc->identity())
              signingKey = acc->identity()->signingKey();
      }
      // now try to sign the article
      if (!signingKey.isEmpty()) {
          TQString tmpText = tmp;
          Kpgp::Block block;
          bool ok=true;
          TQTextCodec *codec=KGlobal::charsets()->codecForName(c_harset, ok);
          if(!ok) // no suitable codec found => try local settings and hope the best ;-)
              codec=KGlobal::locale()->codecForEncoding();

          block.setText( codec->fromUnicode(tmpText) );
          kdDebug(5003) << "signing article from " << article()->from()->email() << endl;
          if( block.clearsign( signingKey, codec->name() ) == Kpgp::Ok ) {
              TQCString result = block.text();
              tmp = codec->toUnicode(result.data(), result.length() );
          } else {
              return false;
          }
      }
  }

  text->fromUnicodeString(tmp);

  //text is set and all attached contents have been assembled => now set lines
  a_rticle->lines()->setNumberOfLines(a_rticle->lineCount());

  a_rticle->assemble();
  a_rticle->updateListItem();
  return true;
}


void KNComposer::closeEvent(TQCloseEvent *e)
{
  if(!v_iew->e_dit->isModified() && !a_ttChanged) {  // nothing to save, don't show nag screen
    if(a_rticle->id()==-1)
      r_esult=CRdel;
    else
      r_esult=CRcancel;
  }
  else {
    switch ( KMessageBox::warningYesNoCancel( this, i18n("Do you want to save this article in the draft folder?"),
                                              TQString::null, KStdGuiItem::save(), KStdGuiItem::discard())) {
      case KMessageBox::Yes :
        r_esult=CRsave;
        break;
      case KMessageBox::No :
        if (a_rticle->id()==-1) r_esult=CRdel;
          else r_esult=CRcancel;
        break;
      default:            // cancel
        e->ignore();
        return;
    }
  }

    e->accept();
  emit composerDone(this);
  // we're dead at this point, don't access members!
}


void KNComposer::initData(const TQString &text)
{
  //Subject
  if(a_rticle->subject()->isEmpty())
    slotSubjectChanged(TQString::null);
  else
    v_iew->s_ubject->setText(a_rticle->subject()->asUnicodeString());

  //Newsgroups
  v_iew->g_roups->setText(a_rticle->newsgroups()->asUnicodeString());

  //To
  v_iew->t_o->setText(a_rticle->to()->asUnicodeString());

  //Followup-To
  KMime::Headers::FollowUpTo *fup2=a_rticle->followUpTo(false);
  if(fup2 && !fup2->isEmpty())
    v_iew->f_up2->lineEdit()->setText(fup2->asUnicodeString());

  KMime::Content *textContent=a_rticle->textContent();
  TQString s;

  if(text.isEmpty()) {
    if(textContent)
      textContent->decodedText(s);
  } else
    s = text;

  v_iew->e_dit->setText(s);

  // initialize the charset select action
  if(textContent)
    c_harset=textContent->contentType()->charset();
  else
    c_harset=knGlobals.configManager()->postNewsTechnical()->charset();

  a_ctSetCharset->setCurrentItem(knGlobals.configManager()->postNewsTechnical()->indexForCharset(c_harset));

  // initialize the message type select action
  if (a_rticle->doPost() && a_rticle->doMail())
    m_ode = news_mail;
  else
    if (a_rticle->doPost())
      m_ode = news;
    else
      m_ode = mail;
  setMessageMode(m_ode);

  if(a_rticle->contentType()->isMultipart()) {
    v_iew->showAttachmentView();
    KMime::Content::List attList;
    AttachmentViewItem *item=0;
    attList.setAutoDelete(false);
    a_rticle->attachments(&attList);
    for(KMime::Content *c=attList.first(); c; c=attList.next()) {
      item=new AttachmentViewItem(v_iew->a_ttView, new KNAttachment(c));
    }
  }
}


// inserts at cursor position if clear is false, replaces content otherwise
// puts the file content into a box if box==true
// "file" is already open for reading
void KNComposer::insertFile(TQFile *file, bool clear, bool box, TQString boxTitle)
{
  TQString temp;
  bool ok=true;
  TQTextCodec *codec=KGlobal::charsets()->codecForName(c_harset, ok);
  TQTextStream ts(file);
  ts.setCodec(codec);

  if (box)
    temp = TQString::fromLatin1(",----[ %1 ]\n").arg(boxTitle);

  if (box && (v_iew->e_dit->wordWrap()!=TQMultiLineEdit::NoWrap)) {
    int wrapAt = v_iew->e_dit->wrapColumnOrWidth();
    TQStringList lst;
    TQString line;
    while(!file->atEnd()) {
      line=ts.readLine();
      if (!file->atEnd())
        line+="\n";
      lst.append(line);
    }
    temp+=KNHelper::rewrapStringList(lst, wrapAt, '|', false, true);
  } else {
    while(!file->atEnd()) {
      if (box)
        temp+="| ";
      temp+=ts.readLine();
      if (!file->atEnd())
        temp += "\n";
    }
  }

  if (box)
    temp += TQString::fromLatin1("`----");

  if(clear)
    v_iew->e_dit->setText(temp);
  else
    v_iew->e_dit->insert(temp);
}


// ask for a filename, handle network urls
void KNComposer::insertFile(bool clear, bool box)
{
  KNLoadHelper helper(this);
  TQFile *file = helper.getFile(i18n("Insert File"));
  KURL url;
  TQString boxName;

  if (file) {
    url = helper.getURL();

    if (url.isLocalFile())
      boxName = url.path();
    else
      boxName = url.prettyURL();

    insertFile(file,clear,box,boxName);
  }
}


//-------------------------------- <Actions> ------------------------------------


void KNComposer::addRecentAddress()
{
    if( !v_iew->t_o->isHidden() )
        RecentAddresses::self(knGlobals.config())->add( v_iew->t_o->text() );
}

void KNComposer::slotSendNow()
{
  r_esult=CRsendNow;
  addRecentAddress();
  emit composerDone(this);
}


void KNComposer::slotSendLater()
{
  r_esult=CRsendLater;
  addRecentAddress();
  emit composerDone(this);
}


void KNComposer::slotSaveAsDraft()
{
  r_esult=CRsave;
  addRecentAddress();
  emit composerDone(this);
}


void KNComposer::slotArtDelete()
{
  r_esult=CRdelAsk;
  emit composerDone(this);
}


void KNComposer::slotAppendSig()
{
  if(!s_ignature.isEmpty()) {
    v_iew->e_dit->append("\n"+s_ignature);
    v_iew->e_dit->setModified(true);
  }
}


void KNComposer::slotInsertFile()
{
  insertFile(false,false);
}


void KNComposer::slotInsertFileBoxed()
{
  insertFile(false,true);
}


void KNComposer::slotAttachFile()
{
  KNLoadHelper *helper = new KNLoadHelper(this);

  if (helper->getFile(i18n("Attach File"))) {
   if (!v_iew->v_iewOpen) {
      KNHelper::saveWindowSize("composer", size());
      v_iew->showAttachmentView();
    }
    (void) new AttachmentViewItem(v_iew->a_ttView, new KNAttachment(helper));
    a_ttChanged=true;
  } else {
    delete helper;
  }
}


void KNComposer::slotRemoveAttachment()
{
  if(!v_iew->v_iewOpen) return;

  if(v_iew->a_ttView->currentItem()) {
    AttachmentViewItem *it=static_cast<AttachmentViewItem*>(v_iew->a_ttView->currentItem());
    if(it->attachment->isAttached()) {
      mDeletedAttachments.append( it->attachment );
      it->attachment=0;
    }
    delete it;

    if(v_iew->a_ttView->childCount()==0) {
      KNHelper::saveWindowSize("composerAtt", size());
      v_iew->hideAttachmentView();
    }

    a_ttChanged=true;
  }
}

void KNComposer::slotAttachmentProperties()
{
  if(!v_iew->v_iewOpen) return;

  if(v_iew->a_ttView->currentItem()) {
    AttachmentViewItem *it=static_cast<AttachmentViewItem*>(v_iew->a_ttView->currentItem());
    AttachmentPropertiesDlg *d=new AttachmentPropertiesDlg(it->attachment, this);
    if(d->exec()) {
      d->apply();
      it->setText(1, it->attachment->mimeType());
      it->setText(3, it->attachment->description());
      it->setText(4, it->attachment->encoding());
    }
    delete d;
    a_ttChanged=true;
  }
}


void KNComposer::slotToggleDoPost()
{
  if (a_ctDoPost->isChecked()) {
    if (a_ctDoMail->isChecked())
      m_ode=news_mail;
    else
      m_ode=news;
  } else {
    if (a_ctDoMail->isChecked())
      m_ode=mail;
    else {     // invalid
      a_ctDoPost->setChecked(true); //revert
      return;
    }
  }
  setMessageMode(m_ode);
}


void KNComposer::slotToggleDoMail()
{
  if (a_ctDoMail->isChecked()) {
    if (a_uthorDislikesMailCopies) {
      if (!(KMessageBox::warningContinueCancel(this, i18n("The poster does not want a mail copy of your reply (Mail-Copies-To: nobody);\nplease respect their request."),
                                               TQString::null, i18n("&Send Copy")) == KMessageBox::Continue)) {
        a_ctDoMail->setChecked(false); //revert
        return;
      }
    }

    if (knGlobals.configManager()->postNewsTechnical()->useExternalMailer()) {
      TQString s = v_iew->e_dit->textLine(0);
      if (!s.contains(i18n("<posted & mailed>")))
        v_iew->e_dit->insertAt(i18n("<posted & mailed>\n\n"),0,0);
      TQString tmp;
      TQStringList textLines = v_iew->e_dit->processedText();
      for (TQStringList::Iterator it = textLines.begin(); it != textLines.end(); ++it) {
        if (*it == "-- ")   // try to be smart, don't include the signature,
          break;            // kmail will append one, too.
        tmp+=*it+"\n";
      }
      knGlobals.artFactory->sendMailExternal(v_iew->t_o->text(), v_iew->s_ubject->text(), tmp);
      a_ctDoMail->setChecked(false); //revert
      return;
    } else {
      if (a_ctDoPost->isChecked())
        m_ode=news_mail;
      else
        m_ode=mail;
    }
  } else {
    if (a_ctDoPost->isChecked())
      m_ode=news;
    else {     // invalid
      a_ctDoMail->setChecked(true); //revert
      return;
    }
  }
  setMessageMode(m_ode);
}


void KNComposer::slotSetCharset(const TQString &s)
{
  if(s.isEmpty())
    return;

  c_harset=s.latin1();
  setConfig(true); //adjust fonts
}


void KNComposer::slotSetCharsetKeyboard()
{
  int newCS = KNHelper::selectDialog(this, i18n("Select Charset"), a_ctSetCharset->items(), a_ctSetCharset->currentItem());
  if (newCS != -1) {
    a_ctSetCharset->setCurrentItem(newCS);
    slotSetCharset(*(a_ctSetCharset->items().at(newCS)));
  }
}


void KNComposer::slotToggleWordWrap()
{
  v_iew->e_dit->setWordWrap(a_ctWordWrap->isChecked()? TQMultiLineEdit::FixedColumnWidth : TQMultiLineEdit::NoWrap);
}


void KNComposer::slotUndoRewrap()
{
  if (KMessageBox::warningContinueCancel( this, i18n("This will replace all text you have written.")) == KMessageBox::Continue) {
    v_iew->e_dit->setText(u_nwraped);
    slotAppendSig();
  }
}

void KNComposer::slotExternalEditor()
{
  if(e_xternalEditor)   // in progress...
    return;

  TQString editorCommand=knGlobals.configManager()->postNewsComposer()->externalEditor();

  if(editorCommand.isEmpty())
    KMessageBox::sorry(this, i18n("No editor configured.\nPlease do this in the settings dialog."));

  if(e_ditorTempfile) {       // shouldn't happen...
    e_ditorTempfile->unlink();
    delete e_ditorTempfile;
    e_ditorTempfile=0;
  }

  e_ditorTempfile=new KTempFile();

  if(e_ditorTempfile->status()!=0) {
    KNHelper::displayTempFileError(this);
    e_ditorTempfile->unlink();
    delete e_ditorTempfile;
    e_ditorTempfile=0;
    return;
  }

  bool ok=true;
  TQTextCodec *codec=KGlobal::charsets()->codecForName(c_harset, ok);

  TQString tmp;
  TQStringList textLines = v_iew->e_dit->processedText();
  for (TQStringList::Iterator it = textLines.begin(); it != textLines.end();) {
    tmp += *it;
    ++it;
    if (it != textLines.end())
      tmp+="\n";
  }

  TQCString local = codec->fromUnicode(tmp);
  e_ditorTempfile->file()->writeBlock(local.data(),local.length());
  e_ditorTempfile->file()->flush();

  if(e_ditorTempfile->status()!=0) {
    KNHelper::displayTempFileError(this);
    e_ditorTempfile->unlink();
    delete e_ditorTempfile;
    e_ditorTempfile=0;
    return;
  }

  e_xternalEditor=new KProcess();

  // construct command line...
  TQStringList command = TQStringList::split(' ',editorCommand);
  bool filenameAdded=false;
  for ( TQStringList::Iterator it = command.begin(); it != command.end(); ++it ) {
    if ((*it).contains("%f")) {
      (*it).replace(TQRegExp("%f"),e_ditorTempfile->name());
      filenameAdded=true;
    }
    (*e_xternalEditor) << (*it);
  }
  if(!filenameAdded)    // no %f in the editor command
    (*e_xternalEditor) << e_ditorTempfile->name();

  connect(e_xternalEditor, TQT_SIGNAL(processExited(KProcess *)),this, TQT_SLOT(slotEditorFinished(KProcess *)));
  if(!e_xternalEditor->start()) {
    KMessageBox::error(this, i18n("Unable to start external editor.\nPlease check your configuration in the settings dialog."));
    delete e_xternalEditor;
    e_xternalEditor=0;
    e_ditorTempfile->unlink();
    delete e_ditorTempfile;
    e_ditorTempfile=0;
    return;
  }

  a_ctExternalEditor->setEnabled(false);   // block other edit action while the editor is running...
  a_ctSpellCheck->setEnabled(false);
  v_iew->showExternalNotification();
}


void KNComposer::slotSpellcheck()
{
  if(s_pellChecker)    // in progress...
    return;
  spellLineEdit = !spellLineEdit;
  a_ctExternalEditor->setEnabled(false);
  a_ctSpellCheck->setEnabled(false);

  s_pellChecker = new KSpell(this, i18n("Spellcheck"), this, TQT_SLOT(slotSpellStarted(KSpell *)));
  TQStringList l = KSpellingHighlighter::personalWords();
  for ( TQStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
      s_pellChecker->addPersonal( *it );
  }
  connect(s_pellChecker, TQT_SIGNAL(death()), this, TQT_SLOT(slotSpellFinished()));
  connect(s_pellChecker, TQT_SIGNAL(done(const TQString&)), this, TQT_SLOT(slotSpellDone(const TQString&)));
  connect(s_pellChecker, TQT_SIGNAL(misspelling (const TQString &, const TQStringList &, unsigned int)),
          this, TQT_SLOT(slotMisspelling (const TQString &, const TQStringList &, unsigned int)));
  connect(s_pellChecker, TQT_SIGNAL(corrected (const TQString &, const TQString &, unsigned int)),
          this, TQT_SLOT(slotCorrected (const TQString &, const TQString &, unsigned int)));
}


void KNComposer::slotMisspelling(const TQString &text, const TQStringList &lst, unsigned int pos)
{
     if( spellLineEdit )
         v_iew->s_ubject->spellCheckerMisspelling( text, lst, pos);
     else
         v_iew->e_dit->misspelling(text, lst, pos);

}

void KNComposer::slotCorrected (const TQString &oldWord, const TQString &newWord, unsigned int pos)
{
    if( spellLineEdit )
        v_iew->s_ubject->spellCheckerCorrected( oldWord, newWord, pos);
    else
        v_iew->e_dit->corrected(oldWord, newWord, pos);
}

void KNComposer::slotUpdateStatusBar()
{
  TQString typeDesc;
  switch (m_ode) {
    case news:  typeDesc = i18n("News Article");
                break;
    case mail:  typeDesc = i18n("Email");
                break;
    default  :  typeDesc = i18n("News Article & Email");
  }
  TQString overwriteDesc;
  if (v_iew->e_dit->isOverwriteMode())
    overwriteDesc = i18n(" OVR ");
  else
    overwriteDesc = i18n(" INS ");

  statusBar()->changeItem(i18n(" Type: %1 ").arg(typeDesc), 1);
  statusBar()->changeItem(i18n(" Charset: %1 ").arg(c_harset), 2);
  statusBar()->changeItem(overwriteDesc, 3);
  statusBar()->changeItem(i18n(" Column: %1 ").arg(v_iew->e_dit->currentColumn() + 1), 4);
  statusBar()->changeItem(i18n(" Line: %1 ").arg(v_iew->e_dit->currentLine() + 1), 5);
}


void KNComposer::slotUpdateCursorPos()
{
  statusBar()->changeItem(i18n(" Column: %1 ").arg(v_iew->e_dit->currentColumn() + 1), 4);
  statusBar()->changeItem(i18n(" Line: %1 ").arg(v_iew->e_dit->currentLine() + 1), 5);
}


void KNComposer::slotConfKeys()
{
  KKeyDialog::configure(actionCollection(), this, true);
}


void KNComposer::slotConfToolbar()
{
  KConfig *conf = knGlobals.config();
  conf->setGroup("composerWindow_options");
  saveMainWindowSettings(conf);
  KEditToolbar dlg(guiFactory(),this);
  connect(&dlg,TQT_SIGNAL( newToolbarConfig() ), this, TQT_SLOT( slotNewToolbarConfig() ));
  dlg.exec();
}

void KNComposer::slotNewToolbarConfig()
{
  createGUI("kncomposerui.rc");

  a_ttPopup=static_cast<TQPopupMenu*> (factory()->container("attachment_popup", this));
  if(!a_ttPopup) a_ttPopup = new TQPopupMenu();

  KConfig *conf = knGlobals.config();
  conf->setGroup("composerWindow_options");
  applyMainWindowSettings(conf);
}

//-------------------------------- </Actions> -----------------------------------


void KNComposer::slotSubjectChanged(const TQString &t)
{
  // replace newlines
  TQString subject = t;
  subject.replace( '\n', ' ' );
  subject.replace( '\r', ' ' );
  if ( subject != t ) // setText() sets the cursor to the end
    v_iew->s_ubject->setText( subject );
  // update caption
  if( !subject.isEmpty() )
    setCaption( subject );
  else
    setCaption( i18n("No Subject") );
}


void KNComposer::slotGroupsChanged(const TQString &t)
{
  KQCStringSplitter split;
  bool splitOk;
  TQString currText=v_iew->f_up2->currentText();

  v_iew->f_up2->clear();

  split.init(t.latin1(), ",");
  splitOk=split.first();
  while(splitOk) {
    v_iew->f_up2->insertItem(TQString::fromLatin1(split.string()));
    splitOk=split.next();
  }
  v_iew->f_up2->insertItem("");

  if ( !currText.isEmpty() || !mFirstEdit ) // user might have cleared fup2 intentionally during last edit
    v_iew->f_up2->lineEdit()->setText(currText);
}


void KNComposer::slotToBtnClicked()
{
  AddressesDialog dlg( this );
  TQString txt;
  TQString to = v_iew->t_o->text();
  dlg.setShowBCC(false);
  dlg.setShowCC(false);
#if 0
  TQStringList lst;


  txt = mEdtTo->text().stripWhiteSpace();
  if ( !txt.isEmpty() ) {
      lst = KMMessage::splitEmailAddrList( txt );
      dlg.setSelectedTo( lst );
  }
#endif
  dlg.setRecentAddresses( RecentAddresses::self(knGlobals.config())->kabcAddresses() );
  if (dlg.exec()==TQDialog::Rejected) return;

  if(!to.isEmpty())
      to+=", ";
  to+=dlg.to().join(", ");

  v_iew->t_o->setText(to);

}


void KNComposer::slotGroupsBtnClicked()
{
  int id=a_rticle->serverId();
  KNNntpAccount *nntp=0;

  if(id!=-1)
    nntp=knGlobals.accountManager()->account(id);

  if(!nntp)
    nntp=knGlobals.accountManager()->first();

  if(!nntp) {
    KMessageBox::error(this, i18n("You have no valid news accounts configured."));
    v_iew->g_roups->clear();
    return;
  }

  if(id==-1)
    a_rticle->setServerId(nntp->id());

  KNGroupSelectDialog *dlg=new KNGroupSelectDialog(this, nntp, v_iew->g_roups->text().remove(TQRegExp("\\s")));

  connect(dlg, TQT_SIGNAL(loadList(KNNntpAccount*)),
    knGlobals.groupManager(), TQT_SLOT(slotLoadGroupList(KNNntpAccount*)));
  connect(knGlobals.groupManager(), TQT_SIGNAL(newListReady(KNGroupListData*)),
    dlg, TQT_SLOT(slotReceiveList(KNGroupListData*)));

  if(dlg->exec())
    v_iew->g_roups->setText(dlg->selectedGroups());

  delete dlg;
}


void KNComposer::slotEditorFinished(KProcess *)
{
  if(e_xternalEditor->normalExit()) {
    e_ditorTempfile->file()->close();
    e_ditorTempfile->file()->open(IO_ReadOnly);
    insertFile(e_ditorTempfile->file(), true);
    e_xternalEdited=true;
  }

  slotCancelEditor();   // cleanup...
}


void KNComposer::slotCancelEditor()
{
  delete e_xternalEditor;  // this also kills the editor process if it's still running
  e_xternalEditor=0;
  e_ditorTempfile->unlink();
  delete e_ditorTempfile;
  e_ditorTempfile=0;

  a_ctExternalEditor->setEnabled(true);
  a_ctSpellCheck->setEnabled(true);
  v_iew->hideExternalNotification();
}


void KNComposer::slotAttachmentPopup(KListView*, TQListViewItem *it, const TQPoint &p)
{
  if(it)
    a_ttPopup->popup(p);
}


void KNComposer::slotAttachmentSelected(TQListViewItem *it)
{
  if(v_iew->a_ttWidget) {
    v_iew->a_ttRemoveBtn->setEnabled((it!=0));
    v_iew->a_ttEditBtn->setEnabled((it!=0));
  }
}


void KNComposer::slotAttachmentEdit(TQListViewItem *)
{
  slotAttachmentProperties();
}


void KNComposer::slotAttachmentRemove(TQListViewItem *)
{
  slotRemoveAttachment();
}


//==============================================================================
// spellchecking code copied form kedit (Bernd Johannes Wuebben)
//==============================================================================


void KNComposer::slotSpellStarted( KSpell *)
{
    if( !spellLineEdit )
    {
        v_iew->e_dit->spellcheck_start();
        s_pellChecker->setProgressResolution(2);

        // read the quote indicator from the preferences
        KConfig *config=knGlobals.config();
        KConfigGroupSaver saver(config, "READNEWS");
        TQString quotePrefix;
        quotePrefix = config->readEntry("quoteCharacters",">");
//todo fixme
//quotePrefix = mComposer->msg()->formatString(quotePrefix);

        kdDebug(5003) << "spelling: new SpellingFilter with prefix=\"" << quotePrefix << "\"" << endl;
        mSpellingFilter = new SpellingFilter(v_iew->e_dit->text(), quotePrefix, SpellingFilter::FilterUrls,
                                             SpellingFilter::FilterEmailAddresses);

        s_pellChecker->check(mSpellingFilter->filteredText());
    }
    else
        s_pellChecker->check( v_iew->s_ubject->text());
}

void KNComposer::slotSpellDone(const TQString &newtext)
{
    a_ctExternalEditor->setEnabled(true);
    a_ctSpellCheck->setEnabled(true);
    if ( !spellLineEdit )
        v_iew->e_dit->spellcheck_stop();

    int dlgResult = s_pellChecker->dlgResult();
    if ( dlgResult == KS_CANCEL )
    {
        if( spellLineEdit)
        {
            //stop spell check
            spellLineEdit = false;
            TQString tmpText( newtext);
            tmpText =  tmpText.remove('\n');

            if( tmpText != v_iew->s_ubject->text() )
                v_iew->s_ubject->setText( tmpText );
        }
        else
        {
            kdDebug(5003) << "spelling: canceled - restoring text from SpellingFilter" << endl;
            kdDebug(5003)<<" mSpellingFilter->originalText() :"<<mSpellingFilter->originalText()<<endl;
            v_iew->e_dit->setText(mSpellingFilter->originalText());

            //v_iew->e_dit->setModified(mWasModifiedBeforeSpellCheck);
        }
    }
    s_pellChecker->cleanUp();
    KDictSpellingHighlighter::dictionaryChanged();
}


void KNComposer::slotSpellFinished()
{
  a_ctExternalEditor->setEnabled(true);
  a_ctSpellCheck->setEnabled(true);
  KSpell::spellStatus status=s_pellChecker->status();
  delete s_pellChecker;
  s_pellChecker=0;

  kdDebug(5003) << "spelling: delete SpellingFilter" << endl;
  delete mSpellingFilter;
  mSpellingFilter = 0;

  if(status==KSpell::Error) {
    KMessageBox::error(this, i18n("ISpell could not be started.\n"
    "Please make sure you have ISpell properly configured and in your PATH."));
  }
  else if(status==KSpell::Crashed) {
    v_iew->e_dit->spellcheck_stop();
    KMessageBox::error(this, i18n("ISpell seems to have crashed."));
  }
  else
  {
      if( spellLineEdit )
          slotSpellcheck();
      else if( status == KSpell::FinishedNoMisspellingsEncountered )
          KMessageBox::information( this, i18n("No misspellings encountered."));
  }
}


void KNComposer::slotDragEnterEvent(TQDragEnterEvent *ev)
{
  TQStringList files;
  ev->accept(KURLDrag::canDecode(ev));
}


void KNComposer::slotDropEvent(TQDropEvent *ev)
{
  KURL::List urls;

  if (!KURLDrag::decode(ev, urls))
    return;

  for (KURL::List::ConstIterator it = urls.begin(); it != urls.end(); ++it) {
    const KURL &url = *it;
    KNLoadHelper *helper = new KNLoadHelper(this);

    if (helper->setURL(url)) {
      if (!v_iew->v_iewOpen) {
        KNHelper::saveWindowSize("composer", size());
        v_iew->showAttachmentView();
      }
      (void) new AttachmentViewItem(v_iew->a_ttView, new KNAttachment(helper));
      a_ttChanged=true;
    } else {
      delete helper;
    }
  }
}


void KNComposer::dragEnterEvent(TQDragEnterEvent *ev)
{
  slotDragEnterEvent(ev);
}


void KNComposer::dropEvent(TQDropEvent *ev)
{
  slotDropEvent(ev);
}

TQPopupMenu * KNComposer::popupMenu( const TQString& name )
{
    Q_ASSERT(factory());
    if ( factory() )
        return ((TQPopupMenu*)factory()->container( name, this ));
    return 0L;
}


//=====================================================================================


KNComposer::ComposerView::ComposerView(KNComposer *composer, const char *n)
  : TQSplitter(TQSplitter::Vertical, composer, n), a_ttWidget(0), a_ttView(0), v_iewOpen(false)
{
  TQWidget *main=new TQWidget(this);

  //headers
  TQFrame *hdrFrame=new TQFrame(main);
  hdrFrame->setFrameStyle(TQFrame::Box | TQFrame::Sunken);
  TQGridLayout *hdrL=new TQGridLayout(hdrFrame, 4,3, 7,5);
  hdrL->setColStretch(1,1);

  //To
  t_o=new KNLineEdit(this, true, hdrFrame);
  mEdtList.append(t_o);

  l_to=new TQLabel(t_o, i18n("T&o:"), hdrFrame);
  t_oBtn=new TQPushButton(i18n("&Browse..."), hdrFrame);
  hdrL->addWidget(l_to, 0,0);
  hdrL->addWidget(t_o, 0,1);
  hdrL->addWidget(t_oBtn, 0,2);
  connect(t_oBtn, TQT_SIGNAL(clicked()), parent(), TQT_SLOT(slotToBtnClicked()));

  //Newsgroups
  g_roups=new KNLineEdit(this, false, hdrFrame);
  mEdtList.append(g_roups);

  l_groups=new TQLabel(g_roups, i18n("&Groups:"), hdrFrame);
  g_roupsBtn=new TQPushButton(i18n("B&rowse..."), hdrFrame);
  hdrL->addWidget(l_groups, 1,0);
  hdrL->addWidget(g_roups, 1,1);
  hdrL->addWidget(g_roupsBtn, 1,2);
  connect(g_roups, TQT_SIGNAL(textChanged(const TQString&)),
          parent(), TQT_SLOT(slotGroupsChanged(const TQString&)));
  connect(g_roupsBtn, TQT_SIGNAL(clicked()), parent(), TQT_SLOT(slotGroupsBtnClicked()));

  //Followup-To
  f_up2=new KComboBox(true, hdrFrame);
  l_fup2=new TQLabel(f_up2, i18n("Follo&wup-To:"), hdrFrame);
  hdrL->addWidget(l_fup2, 2,0);
  hdrL->addMultiCellWidget(f_up2, 2,2, 1,2);

  //subject
  s_ubject=new KNLineEditSpell(this, false, hdrFrame);
  mEdtList.append(s_ubject);

  TQLabel *l=new TQLabel(s_ubject, i18n("S&ubject:"), hdrFrame);
  hdrL->addWidget(l, 3,0);
  hdrL->addMultiCellWidget(s_ubject, 3,3, 1,2);
  connect(s_ubject, TQT_SIGNAL(textChanged(const TQString&)),
          parent(), TQT_SLOT(slotSubjectChanged(const TQString&)));

  //Editor
  e_dit=new Editor(this, composer, main);
  e_dit->setMinimumHeight(50);

  KConfig *config = knGlobals.config();
  KConfigGroupSaver saver(config, "VISUAL_APPEARANCE");
  TQColor defaultColor1( kapp->palette().active().text()); // defaults from kmreaderwin.cpp
  TQColor defaultColor2( kapp->palette().active().text() );
  TQColor defaultColor3( kapp->palette().active().text() );
  TQColor defaultForeground( kapp->palette().active().text() );
  TQColor col1 = config->readColorEntry( "ForegroundColor", &defaultForeground );
  TQColor col2 = config->readColorEntry( "quote3Color", &defaultColor3 );
  TQColor col3 = config->readColorEntry( "quote2Color", &defaultColor2 );
  TQColor col4 = config->readColorEntry( "quote1Color", &defaultColor1 );
  TQColor c = TQColor("red");
  mSpellChecker = new KDictSpellingHighlighter(e_dit, /*active*/ true, /*autoEnabled*/ true,
                                       /*spellColor*/ config->readColorEntry("NewMessage", &c),
                                       /*colorQuoting*/ true, col1, col2, col3, col4);
  connect( mSpellChecker, TQT_SIGNAL(newSuggestions(const TQString&, const TQStringList&, unsigned int)), e_dit,
           TQT_SLOT(slotAddSuggestion(const TQString&, const TQStringList&, unsigned int)) );

  TQVBoxLayout *notL=new TQVBoxLayout(e_dit);
  notL->addStretch(1);
  n_otification=new TQGroupBox(2, Qt::Horizontal, e_dit);
  l=new TQLabel(i18n("You are currently editing the article body\nin an external editor. To continue, you have\nto close the external editor."), n_otification);
  c_ancelEditorBtn=new TQPushButton(i18n("&Kill External Editor"), n_otification);
  n_otification->setFrameStyle(TQFrame::Panel | TQFrame::Raised);
  n_otification->setLineWidth(2);
  n_otification->hide();
  notL->addWidget(n_otification, 0, Qt::AlignHCenter);
  notL->addStretch(1);

  //finish GUI
  TQVBoxLayout *topL=new TQVBoxLayout(main, 4,4);
  topL->addWidget(hdrFrame);
  topL->addWidget(e_dit, 1);
}


KNComposer::ComposerView::~ComposerView()
{
  if(v_iewOpen) {
    KConfig *conf=knGlobals.config();
    conf->setGroup("POSTNEWS");

    conf->writeEntry("Att_Splitter",sizes());   // save splitter pos

    TQValueList<int> lst;                        // save header sizes
    TQHeader *h=a_ttView->header();
    for (int i=0; i<5; i++)
      lst << h->sectionSize(i);
    conf->writeEntry("Att_Headers",lst);
  }
  delete mSpellChecker;
}


void KNComposer::ComposerView::focusNextPrevEdit(const TQWidget* aCur, bool aNext)
{
  TQValueList<TQWidget*>::Iterator it;

  if ( !aCur ) {
    it = --( mEdtList.end() );
  } else {
    for ( TQValueList<TQWidget*>::Iterator it2 = mEdtList.begin(); it2 != mEdtList.end(); ++it2 ) {
      if ( (*it2) == aCur ) {
        it = it2;
        break;
      }
    }
    if ( it == mEdtList.end() )
      return;
    if ( aNext )
      ++it;
    else {
      if ( it != mEdtList.begin() )
        --it;
      else
        return;
    }
  }
  if ( it != mEdtList.end() ) {
    if ( (*it)->isVisible() )
      (*it)->setFocus();
  } else if ( aNext )
    e_dit->setFocus();
}


void KNComposer::ComposerView::setMessageMode(KNComposer::MessageMode mode)
{
  if (mode != KNComposer::news) {
    l_to->show();
    t_o->show();
    t_oBtn->show();
  } else {
    l_to->hide();
    t_o->hide();
    t_oBtn->hide();
  }
  if (mode != KNComposer::mail) {
    l_groups->show();
    l_fup2->show();
    g_roups->show();
    f_up2->show();
    g_roupsBtn->show();

  } else {
    l_groups->hide();
    l_fup2->hide();
    g_roups->hide();
    f_up2->hide();
    g_roupsBtn->hide();
  }
}

void KNComposer::ComposerView::restartBackgroundSpellCheck()
{
    mSpellChecker->restartBackgroundSpellCheck();
}

void KNComposer::ComposerView::showAttachmentView()
{
  if(!a_ttWidget) {
    a_ttWidget=new TQWidget(this);
    TQGridLayout *topL=new TQGridLayout(a_ttWidget, 3, 2, 4, 4);

    a_ttView=new AttachmentView(a_ttWidget);
    topL->addMultiCellWidget(a_ttView, 0,2, 0,0);

    //connections
    connect(a_ttView, TQT_SIGNAL(currentChanged(TQListViewItem*)),
            parent(), TQT_SLOT(slotAttachmentSelected(TQListViewItem*)));
    connect(a_ttView, TQT_SIGNAL(clicked ( TQListViewItem * )),
            parent(), TQT_SLOT(slotAttachmentSelected(TQListViewItem*)));

    connect(a_ttView, TQT_SIGNAL(contextMenu(KListView*, TQListViewItem*, const TQPoint&)),
            parent(), TQT_SLOT(slotAttachmentPopup(KListView*, TQListViewItem*, const TQPoint&)));
    connect(a_ttView, TQT_SIGNAL(delPressed(TQListViewItem*)),
            parent(), TQT_SLOT(slotAttachmentRemove(TQListViewItem*)));
    connect(a_ttView, TQT_SIGNAL(doubleClicked(TQListViewItem*)),
            parent(), TQT_SLOT(slotAttachmentEdit(TQListViewItem*)));
    connect(a_ttView, TQT_SIGNAL(returnPressed(TQListViewItem*)),
            parent(), TQT_SLOT(slotAttachmentEdit(TQListViewItem*)));

    //buttons
    a_ttAddBtn=new TQPushButton(i18n("A&dd..."),a_ttWidget);
    connect(a_ttAddBtn, TQT_SIGNAL(clicked()), parent(), TQT_SLOT(slotAttachFile()));
    topL->addWidget(a_ttAddBtn, 0,1);

    a_ttRemoveBtn=new TQPushButton(i18n("&Remove"), a_ttWidget);
    a_ttRemoveBtn->setEnabled(false);
    connect(a_ttRemoveBtn, TQT_SIGNAL(clicked()), parent(), TQT_SLOT(slotRemoveAttachment()));
    topL->addWidget(a_ttRemoveBtn, 1,1);

    a_ttEditBtn=new TQPushButton(i18n("&Properties"), a_ttWidget);
    a_ttEditBtn->setEnabled(false);
    connect(a_ttEditBtn, TQT_SIGNAL(clicked()), parent(), TQT_SLOT(slotAttachmentProperties()));
    topL->addWidget(a_ttEditBtn, 2,1, Qt::AlignTop);

    topL->setRowStretch(2,1);
    topL->setColStretch(0,1);
  }

  if(!v_iewOpen) {
    v_iewOpen=true;
    a_ttWidget->show();

    KConfig *conf=knGlobals.config();
    conf->setGroup("POSTNEWS");

    TQValueList<int> lst=conf->readIntListEntry("Att_Splitter");
    if(lst.count()!=2)
      lst << 267 << 112;
    setSizes(lst);

    lst=conf->readIntListEntry("Att_Headers");
    if(lst.count()==5) {
      TQValueList<int>::Iterator it=lst.begin();

      TQHeader *h=a_ttView->header();
      for(int i=0; i<5; i++) {
        h->resizeSection(i,(*it));
        ++it;
      }
    }
  }
}


void KNComposer::ComposerView::hideAttachmentView()
{
  if(v_iewOpen) {
    a_ttWidget->hide();
    v_iewOpen=false;
  }
}


void KNComposer::ComposerView::showExternalNotification()
{
  e_dit->setReadOnly(true);
  n_otification->show();
}


void KNComposer::ComposerView::hideExternalNotification()
{
  e_dit->setReadOnly(false);
  n_otification->hide();
}


//=====================================================================================

#include <kcursor.h>
KNComposer::Editor::Editor(KNComposer::ComposerView *_composerView, KNComposer *_composer, TQWidget *parent, char *name)
    : KEdit(parent, name), m_composer( _composer ), m_composerView(_composerView)
{
  setOverwriteEnabled(true);
  spell = 0L;
  installEventFilter(this);
  KCursor::setAutoHideCursor( this, true, true );
  m_bound = TQRegExp( TQString::fromLatin1("[\\s\\W]") );
}


KNComposer::Editor::~Editor()
{
    removeEventFilter(this);
    delete spell;
}

//-----------------------------------------------------------------------------
bool KNComposer::Editor::eventFilter(TQObject*o, TQEvent* e)
{
  if (o == this)
    KCursor::autoHideEventFilter(o, e);

  if (e->type() == TQEvent::KeyPress)
  {
    TQKeyEvent *k = (TQKeyEvent*)e;
    // ---sven's Arrow key navigation start ---
    // Key Up in first line takes you to Subject line.
    if (k->key() == Key_Up && k->state() != ShiftButton && currentLine() == 0
      && lineOfChar(0, currentColumn()) == 0)
    {
      deselect();
      m_composerView->focusNextPrevEdit(0, false); //take me up
      return true;
    }
    // ---sven's Arrow key navigation end ---

    if (k->key() == Key_Backtab && k->state() == ShiftButton)
    {
      deselect();
      m_composerView->focusNextPrevEdit(0, false);
      return true;
    }
  } else if ( e->type() == TQEvent::ContextMenu ) {
    TQContextMenuEvent *event = (TQContextMenuEvent*) e;

    int para = 1, charPos, firstSpace, lastSpace;

    //Get the character at the position of the click
    charPos = charAt( viewportToContents(event->pos() ), &para );
    TQString paraText = text( para );

    if( !paraText.at(charPos).isSpace() )
    {
      //Get word right clicked on
      firstSpace = paraText.findRev( m_bound, charPos ) + 1;
      lastSpace = paraText.find( m_bound, charPos );
      if( lastSpace == -1 )
        lastSpace = paraText.length();
      TQString word = paraText.mid( firstSpace, lastSpace - firstSpace );
      //Continue if this word was misspelled
      if( !word.isEmpty() && m_replacements.contains( word ) )
      {
        KPopupMenu p;
        p.insertTitle( i18n("Suggestions") );

        //Add the suggestions to the popup menu
        TQStringList reps = m_replacements[word];
        if( reps.count() > 0 )
        {
          int listPos = 0;
          for ( TQStringList::Iterator it = reps.begin(); it != reps.end(); ++it ) {
            p.insertItem( *it, listPos );
            listPos++;
          }
        }
        else
        {
          p.insertItem( i18n( "No Suggestions" ), -2 );
        }

        //Execute the popup inline
        int id = p.exec( mapToGlobal( event->pos() ) );

        if( id > -1 )
        {
          //Save the cursor position
          int parIdx = 1, txtIdx = 1;
          getCursorPosition(&parIdx, &txtIdx);
          setSelection(para, firstSpace, para, lastSpace);
          insert(m_replacements[word][id]);
          // Restore the cursor position; if the cursor was behind the
          // misspelled word then adjust the cursor position
          if ( para == parIdx && txtIdx >= lastSpace )
            txtIdx += m_replacements[word][id].length() - word.length();
          setCursorPosition(parIdx, txtIdx);
        }
        //Cancel original event
        return true;
      }
    }
  }

  return KEdit::eventFilter(o, e);
}

void KNComposer::Editor::slotAddSuggestion( const TQString &text, const TQStringList &lst, unsigned int )
{
  m_replacements[text] = lst;
}

// expand tabs to avoid the "tab-damage",
// auto-wraped paragraphs have to split (code taken from KEdit::saveText)
TQStringList KNComposer::Editor::processedText()
{
  TQStringList ret;
  int lines = numLines()-1;
  if (lines < 0)
    return ret;

  if (wordWrap() == NoWrap) {
    for (int i = 0; i <= lines; i++)
      ret.append(textLine(i));
  } else {
    for (int i = 0; i <= lines; i++) {
      int lines_in_parag = linesOfParagraph(i);

      if (lines_in_parag == 1) {
        ret.append(textLine(i));
      } else {
        TQString parag_text = textLine(i);
        int pos = 0;
        int last_pos = 0;
        int current_line = 0;
        while (current_line+1 < lines_in_parag) {
          while (lineOfChar(i, pos) == current_line) pos++;
          ret.append(parag_text.mid(last_pos, pos - last_pos - 1));
          current_line++;
          last_pos = pos;
        }
        // add last line
        ret.append(parag_text.mid(pos));
      }
    }
  }

  TQString replacement;
  int tabPos;
  for (TQStringList::Iterator it = ret.begin(); it != ret.end(); ++it ) {
    while ((tabPos=(*it).find('\t'))!=-1) {
      replacement.fill(TQChar(' '), 8-(tabPos%8));
      (*it).replace(tabPos, 1, replacement);
    }
  }

  return ret;
}


void KNComposer::Editor::slotPasteAsQuotation()
{
  TQString s = TQApplication::clipboard()->text();
  if (!s.isEmpty()) {
    for (int i=0; (uint)i<s.length(); i++) {
      if ( s[i] < ' ' && s[i] != '\n' && s[i] != '\t' )
        s[i] = ' ';
    }
    s.prepend("> ");
    s.replace(TQRegExp("\n"),"\n> ");
    insert(s);
  }
}


void KNComposer::Editor::slotFind()
{
  search();
}

void KNComposer::Editor::slotSearchAgain()
{
    repeatSearch();
}

void KNComposer::Editor::slotReplace()
{
  replace();
}


void KNComposer::Editor::slotAddQuotes()
{
  if (hasMarkedText()) {
    TQString s = markedText();
    s.prepend("> ");
    s.replace(TQRegExp("\n"),"\n> ");
    insert(s);
  } else {
    int l = currentLine();
    int c = currentColumn();
    TQString s = textLine(l);
    s.prepend("> ");
    insertLine(s,l);
    removeLine(l+1);
    setCursorPosition(l,c+2);
  }
}


void KNComposer::Editor::slotRemoveQuotes()
{
  if (hasMarkedText()) {
    TQString s = markedText();
    if (s.left(2) == "> ")
      s.remove(0,2);
    s.replace(TQRegExp("\n> "),"\n");
    insert(s);
  } else {
    int l = currentLine();
    int c = currentColumn();
    TQString s = textLine(l);
    if (s.left(2) == "> ") {
      s.remove(0,2);
      insertLine(s,l);
      removeLine(l+1);
      setCursorPosition(l,c-2);
    }
  }
}


void KNComposer::Editor::slotAddBox()
{
    if (hasMarkedText()) {
    TQString s = markedText();
    s.prepend(",----[  ]\n");
    s.replace(TQRegExp("\n"),"\n| ");
    s.append("\n`----");
    insert(s);
  } else {
    int l = currentLine();
    int c = currentColumn();
    TQString s = TQString::fromLatin1(",----[  ]\n| %1\n`----").arg(textLine(l));
    insertLine(s,l);
    removeLine(l+3);
    setCursorPosition(l+1,c+2);
  }
}


void KNComposer::Editor::slotRemoveBox()
{
  if (hasMarkedText()) {
    TQString s = TQString::fromLatin1("\n") + markedText() + TQString::fromLatin1("\n");
    s.replace(TQRegExp("\n,----[^\n]*\n"),"\n");
    s.replace(TQRegExp("\n| "),"\n");
    s.replace(TQRegExp("\n`----[^\n]*\n"),"\n");
    s.remove(0,1);
    s.truncate(s.length()-1);
    insert(s);
  } else {
    int l = currentLine();
    int c = currentColumn();

    TQString s = textLine(l);   // test if we are in a box
    if (!((s.left(2) == "| ")||(s.left(5)==",----")||(s.left(5)=="`----")))
      return;

    setAutoUpdate(false);

    // find & remove box begin
    int x = l;
    while ((x>=0)&&(textLine(x).left(5)!=",----"))
      x--;
    if ((x>=0)&&(textLine(x).left(5)==",----")) {
      removeLine(x);
      l--;
      for (int i=x;i<=l;i++) {     // remove quotation
        s = textLine(i);
        if (s.left(2) == "| ") {
          s.remove(0,2);
          insertLine(s,i);
          removeLine(i+1);
        }
      }
    }

    // find & remove box end
    x = l;
    while ((x<numLines())&&(textLine(x).left(5)!="`----"))
      x++;
    if ((x<numLines())&&(textLine(x).left(5)=="`----")) {
      removeLine(x);
      for (int i=l+1;i<x;i++) {     // remove quotation
        s = textLine(i);
        if (s.left(2) == "| ") {
          s.remove(0,2);
          insertLine(s,i);
          removeLine(i+1);
        }
      }
    }

    setCursorPosition(l,c-2);

    setAutoUpdate(true);
    repaint(false);
  }
}


void KNComposer::Editor::slotRot13()
{
  if (hasMarkedText())
    insert(KNHelper::rot13(markedText()));
}


void KNComposer::Editor::contentsDragEnterEvent(TQDragEnterEvent *ev)
{
  if (KURLDrag::canDecode(ev))
    emit(sigDragEnterEvent(ev));
  else
    KEdit::dragEnterEvent(ev);
}


void KNComposer::Editor::contentsDropEvent(TQDropEvent *ev)
{
  if (KURLDrag::canDecode(ev))
    emit(sigDropEvent(ev));
  else
    KEdit::dropEvent(ev);
}

void KNComposer::Editor::keyPressEvent ( TQKeyEvent *e)
{
    if( e->key() == Key_Return ) {
        int line, col;
        getCursorPosition( &line, &col );
        TQString lineText = text( line );
        // returns line with additional trailing space (bug in Qt?), cut it off
        lineText.truncate( lineText.length() - 1 );
        // special treatment of quoted lines only if the cursor is neither at
        // the begin nor at the end of the line
        if( ( col > 0 ) && ( col < int( lineText.length() ) ) ) {
            bool isQuotedLine = false;
            uint bot = 0; // bot = begin of text after quote indicators
            while( bot < lineText.length() ) {
                if( ( lineText[bot] == '>' ) || ( lineText[bot] == '|' ) ) {
                    isQuotedLine = true;
                    ++bot;
                }
                else if( lineText[bot].isSpace() ) {
                    ++bot;
                }
                else {
                    break;
                }
            }

            KEdit::keyPressEvent( e );

            // duplicate quote indicators of the previous line before the new
            // line if the line actually contained text (apart from the quote
            // indicators) and the cursor is behind the quote indicators
            if( isQuotedLine
                && ( bot != lineText.length() )
                && ( col >= int( bot ) ) ) {
                TQString newLine = text( line + 1 );
                // remove leading white space from the new line and instead
                // add the quote indicators of the previous line
                unsigned int leadingWhiteSpaceCount = 0;
                while( ( leadingWhiteSpaceCount < newLine.length() )
                       && newLine[leadingWhiteSpaceCount].isSpace() ) {
                    ++leadingWhiteSpaceCount;
                }
                newLine = newLine.replace( 0, leadingWhiteSpaceCount,
                                           lineText.left( bot ) );
                removeParagraph( line + 1 );
                insertParagraph( newLine, line + 1 );
                // place the cursor at the begin of the new line since
                // we assume that the user split the quoted line in order
                // to add a comment to the first part of the quoted line
                setCursorPosition( line + 1 , 0 );
            }
        }
        else
            KEdit::keyPressEvent( e );
    }
    else
        KEdit::keyPressEvent( e );
}


void KNComposer::Editor::contentsContextMenuEvent( TQContextMenuEvent */*e*/ )
{
    TQString selectWord = selectWordUnderCursor();
    TQPopupMenu* popup = 0L;
    if ( selectWord.isEmpty())
    {
        popup = m_composer ? m_composer->popupMenu( "edit" ): 0;
        if ( popup )
            popup->popup(TQCursor::pos());
    }
    else
    {
        spell = new KSpell(this, i18n("Spellcheck"), this, TQT_SLOT(slotSpellStarted(KSpell *)));
        TQStringList l = KSpellingHighlighter::personalWords();
        for ( TQStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
            spell->addPersonal( *it );
        }
        connect(spell, TQT_SIGNAL(death()), this, TQT_SLOT(slotSpellFinished()));
        connect(spell, TQT_SIGNAL(done(const TQString&)), this, TQT_SLOT(slotSpellDone(const TQString&)));
        connect(spell, TQT_SIGNAL(misspelling (const TQString &, const TQStringList &, unsigned int)),
                this, TQT_SLOT(slotMisspelling (const TQString &, const TQStringList &, unsigned int)));
    }
}

void KNComposer::Editor::slotSpellStarted( KSpell *)
{
    spell->check( selectWordUnderCursor(),false );
}


void KNComposer::Editor::slotSpellDone(const TQString &/*newtext*/)
{
    spell->cleanUp();
}

void KNComposer::Editor::slotSpellFinished()
{
  KSpell::spellStatus status=spell->status();
  delete spell;
  spell=0;

  if(status==KSpell::Error) {
    KMessageBox::error(this, i18n("ISpell could not be started.\n"
    "Please make sure you have ISpell properly configured and in your PATH."));
  }
  else if(status==KSpell::Crashed) {

    KMessageBox::error(this, i18n("ISpell seems to have crashed."));
  }
}

void KNComposer::Editor::cut()
{
    KEdit::cut();
    m_composer->v_iew->restartBackgroundSpellCheck();
}

void KNComposer::Editor::clear()
{
    KEdit::clear();
    m_composer->v_iew->restartBackgroundSpellCheck();
}

void KNComposer::Editor::del()
{
    KEdit::del();
    m_composer->v_iew->restartBackgroundSpellCheck();
}


void KNComposer::Editor::slotMisspelling (const TQString &, const TQStringList &lst, unsigned int)
{
    int countAction = m_composer->listOfResultOfCheckWord( lst , selectWordUnderCursor());
    if ( countAction>0 )
    {
        TQPopupMenu* popup = m_composer ? m_composer->popupMenu( "edit_with_spell" ): 0;
        if ( popup )
            popup->popup(TQCursor::pos());
    }
    else
    {
        TQPopupMenu* popup = m_composer ? m_composer->popupMenu( "edit" ): 0;
        if ( popup )
            popup->popup(TQCursor::pos());
    }
}

void KNComposer::Editor::slotCorrectWord()
{
    removeSelectedText();
    KAction * act = (KAction *)(sender());
    int line, col;
    getCursorPosition(&line,&col);



    insertAt( act->text(), line, col );

    //insert( act->text() );
}

//=====================================================================================


KNComposer::AttachmentView::AttachmentView(TQWidget *parent, char *name)
 : KListView(parent, name)
{
  setFrameStyle(TQFrame::WinPanel | TQFrame::Sunken);  // match the TQMultiLineEdit style
  addColumn(i18n("File"), 115);
  addColumn(i18n("Type"), 91);
  addColumn(i18n("Size"), 55);
  addColumn(i18n("Description"), 110);
  addColumn(i18n("Encoding"), 60);
  header()->setClickEnabled(false);
  setAllColumnsShowFocus(true);
}


KNComposer::AttachmentView::~AttachmentView()
{
}


void KNComposer::AttachmentView::keyPressEvent(TQKeyEvent *e)
{
  if(!e)
    return; // subclass bug

  if( (e->key()==Key_Delete) && (currentItem()) )
    emit(delPressed(currentItem()));
  else
    KListView::keyPressEvent(e);
}


//=====================================================================================


KNComposer::AttachmentViewItem::AttachmentViewItem(KListView *v, KNAttachment *a) :
  KListViewItem(v), attachment(a)
{
  setText(0, a->name());
  setText(1, a->mimeType());
  setText(2, a->contentSize());
  setText(3, a->description());
  setText(4, a->encoding());
}



KNComposer::AttachmentViewItem::~AttachmentViewItem()
{
  delete attachment;
}


//=====================================================================================


KNComposer::AttachmentPropertiesDlg::AttachmentPropertiesDlg(KNAttachment *a, TQWidget *p, const char *n) :
  KDialogBase(p, n, true, i18n("Attachment Properties"), Help|Ok|Cancel, Ok), a_ttachment(a),
  n_onTextAsText(false)
{
  //init GUI
  TQWidget *page=new TQWidget(this);
  setMainWidget(page);
  TQVBoxLayout *topL=new TQVBoxLayout(page);

  //file info
  TQGroupBox *fileGB=new TQGroupBox(i18n("File"), page);
  TQGridLayout *fileL=new TQGridLayout(fileGB, 3,2, 15,5);

  fileL->addRowSpacing(0, fontMetrics().lineSpacing()-9);
  fileL->addWidget(new TQLabel(i18n("Name:"), fileGB) ,1,0);
  fileL->addWidget(new TQLabel(TQString("<b>%1</b>").arg(a->name()), fileGB), 1,1, Qt::AlignLeft);
  fileL->addWidget(new TQLabel(i18n("Size:"), fileGB), 2,0);
  fileL->addWidget(new TQLabel(a->contentSize(), fileGB), 2,1, Qt::AlignLeft);

  fileL->setColStretch(1,1);
  topL->addWidget(fileGB);

  //mime info
  TQGroupBox *mimeGB=new TQGroupBox(i18n("Mime"), page);
  TQGridLayout *mimeL=new TQGridLayout(mimeGB, 4,2, 15,5);

  mimeL->addRowSpacing(0, fontMetrics().lineSpacing()-9);
  m_imeType=new KLineEdit(mimeGB);
  m_imeType->setText(a->mimeType());
  mimeL->addWidget(m_imeType, 1,1);
  mimeL->addWidget(new TQLabel(m_imeType, i18n("&Mime-Type:"), mimeGB), 1,0);

  d_escription=new KLineEdit(mimeGB);
  d_escription->setText(a->description());
  mimeL->addWidget(d_escription, 2,1);
  mimeL->addWidget(new TQLabel(d_escription, i18n("&Description:"), mimeGB), 2,0);

  e_ncoding=new TQComboBox(false, mimeGB);
  e_ncoding->insertItem("7Bit");
  e_ncoding->insertItem("8Bit");
  e_ncoding->insertItem("quoted-printable");
  e_ncoding->insertItem("base64");
  if(a->isFixedBase64()) {
    e_ncoding->setCurrentItem(3);
    e_ncoding->setEnabled(false);
  }
  else
    e_ncoding->setCurrentItem(a->cte());
  mimeL->addWidget(e_ncoding, 3,1);
  mimeL->addWidget(new TQLabel(e_ncoding, i18n("&Encoding:"), mimeGB), 3,0);

  mimeL->setColStretch(1,1);
  topL->addWidget(mimeGB);

  //connections
  connect(m_imeType, TQT_SIGNAL(textChanged(const TQString&)),
    this, TQT_SLOT(slotMimeTypeTextChanged(const TQString&)));

  //finish GUI
  setFixedHeight(sizeHint().height());
  KNHelper::restoreWindowSize("attProperties", this, TQSize(300,250));
  setHelp("anc-knode-editor-advanced");
}


KNComposer::AttachmentPropertiesDlg::~AttachmentPropertiesDlg()
{
  KNHelper::saveWindowSize("attProperties", this->size());
}


void KNComposer::AttachmentPropertiesDlg::apply()
{
  a_ttachment->setDescription(d_escription->text());
  a_ttachment->setMimeType(m_imeType->text());
  a_ttachment->setCte(e_ncoding->currentItem());
}


void KNComposer::AttachmentPropertiesDlg::accept()
{
  if(m_imeType->text().find('/')==-1) {
    KMessageBox::sorry(this, i18n("You have set an invalid mime-type.\nPlease change it."));
    return;
  }
  else if(n_onTextAsText && m_imeType->text().find("text/", 0, false)!=-1 &&
       KMessageBox::warningContinueCancel(this,
       i18n("You have changed the mime-type of this non-textual attachment\nto text. This might cause an error while loading or encoding the file.\nProceed?")
       ) == KMessageBox::Cancel) return;

  KDialogBase::accept();
}


void KNComposer::AttachmentPropertiesDlg::slotMimeTypeTextChanged(const TQString &text)
{
    enableButtonOK( !text.isEmpty() );
  if(text.left(5)!="text/") {
    n_onTextAsText=a_ttachment->isFixedBase64();
    e_ncoding->setCurrentItem(3);
    e_ncoding->setEnabled(false);
  }
  else {
    e_ncoding->setCurrentItem(a_ttachment->cte());
    e_ncoding->setEnabled(true);
  }
}


//--------------------------------

#include "kncomposer.moc"

// kate: space-indent on; indent-width 2;
