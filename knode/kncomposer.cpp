/*
    kncomposer.cpp

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

#include <qheader.h>
#include <qtextcodec.h>
#include <qclipboard.h>
#include <qdragobject.h>
#include <qapplication.h>
#include <kspelldlg.h>

#include <kaccel.h>
#include <kcharsets.h>
#include <kmessagebox.h>
#include <kabc/addresseedialog.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
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
#include <syntaxhighlighter.h>
using Syntaxhighlighter::DictSpellChecker;
using Syntaxhighlighter::SpellChecker;
#include <kapplication.h>
#include "kngroupselectdialog.h"
#include "utilities.h"
#include "knglobals.h"
#include "kncomposer.h"
#include "knode.h"
#include "knconfigmanager.h"
#include "knaccountmanager.h"
#include "knnntpaccount.h"
#include "knpgp.h"
#include "knarticlefactory.h"
#include <kstatusbar.h>
#include <klocale.h>
#include <qpopupmenu.h>
#include <spellingfilter.h>


KNComposer::KNComposer(KNLocalArticle *a, const QString &text, const QString &sig, const QString &unwraped, bool firstEdit, bool dislikesCopies, bool createCopy)
    : KMainWindow(0,"composerWindow"), r_esult(CRsave), a_rticle(a), s_ignature(sig), u_nwraped(unwraped),
      n_eeds8Bit(true), v_alidated(false), a_uthorDislikesMailCopies(dislikesCopies), e_xternalEdited(false), e_xternalEditor(0),
      e_ditorTempfile(0), s_pellChecker(0), a_ttChanged(false)
{
    mSpellingFilter = 0;
  d_elAttList.setAutoDelete(true);

  // activate dnd of attachments...
  setAcceptDrops(true);

  //init v_iew
  v_iew=new ComposerView(this);
  setCentralWidget(v_iew);

  connect(v_iew->c_ancelEditorBtn, SIGNAL(clicked()), SLOT(slotCancelEditor()));
  connect(v_iew->e_dit, SIGNAL(sigDragEnterEvent(QDragEnterEvent *)), SLOT(slotDragEnterEvent(QDragEnterEvent *)));
  connect(v_iew->e_dit, SIGNAL(sigDropEvent(QDropEvent *)), SLOT(slotDropEvent(QDropEvent *)));

  //statusbar
  KStatusBar *sb=statusBar();
  sb->insertItem(QString::null, 1,1);                 // type
  sb->setItemAlignment (1,AlignLeft | AlignVCenter);
  sb->insertItem(QString::null, 2,1);                 // charset
  sb->setItemAlignment (2,AlignLeft | AlignVCenter);
  sb->insertItem(QString::null, 3,0);                 // column
  sb->setItemAlignment (3,AlignCenter | AlignVCenter);
  sb->insertItem(QString::null, 4,0);                 // column
  sb->setItemAlignment (4,AlignCenter | AlignVCenter);
  sb->insertItem(QString::null, 5,0);                 // line
  sb->setItemAlignment (5,AlignCenter | AlignVCenter);
  connect(v_iew->e_dit, SIGNAL(CursorPositionChanged()), SLOT(slotUpdateCursorPos()));
  connect(v_iew->e_dit, SIGNAL(toggle_overwrite_signal()), SLOT(slotUpdateStatusBar()));

  //------------------------------- <Actions> --------------------------------------

  //file menu
  new KAction(i18n("&Send Now"),"mail_send", CTRL + Key_Return , this,
    SLOT(slotSendNow()), actionCollection(), "send_now");

  new KAction(i18n("Send &Later"), "queue", 0, this,
    SLOT(slotSendLater()), actionCollection(), "send_later");

  new KAction(i18n("Save as &Draft"),"filesave", 0 , this,
    SLOT(slotSaveAsDraft()), actionCollection(), "save_as_draft");

  new KAction(i18n("D&elete"),"editdelete", 0 , this,
    SLOT(slotArtDelete()), actionCollection(), "art_delete");

  KStdAction::close(this, SLOT(close()),actionCollection());

  //edit menu
  KAction *undo = KStdAction::undo(v_iew->e_dit, SLOT(undo()), actionCollection());
  undo->setEnabled(false);
  connect(v_iew->e_dit, SIGNAL(undoAvailable(bool)), undo, SLOT(setEnabled(bool)));

  KAction *redo = KStdAction::redo(v_iew->e_dit, SLOT(redo()), actionCollection());
  redo->setEnabled(false);
  connect(v_iew->e_dit, SIGNAL(redoAvailable(bool)), redo, SLOT(setEnabled(bool)));

  KAction *cut =KStdAction::cut(v_iew->e_dit, SLOT(cut()), actionCollection());

  connect(v_iew->e_dit, SIGNAL(copyAvailable(bool)), cut , SLOT(setEnabled(bool)));


  KAction *copy = KStdAction::copy(v_iew->e_dit, SLOT(copy()), actionCollection());
  copy->setEnabled(false);
  connect(v_iew->e_dit, SIGNAL(copyAvailable(bool)), copy, SLOT(setEnabled(bool)));

  KStdAction::paste(v_iew->e_dit, SLOT(paste()), actionCollection());

  new KAction(i18n("Paste as &Quotation"), 0, v_iew->e_dit,
                   SLOT(slotPasteAsQuotation()), actionCollection(), "paste_quoted");

  KStdAction::selectAll(v_iew->e_dit, SLOT(selectAll()), actionCollection());

  KStdAction::find(v_iew->e_dit, SLOT(slotFind()), actionCollection());

  KStdAction::replace(v_iew->e_dit, SLOT(slotReplace()), actionCollection());

  //attach menu
  new KAction(i18n("Append &Signature"), 0 , this, SLOT(slotAppendSig()),
                   actionCollection(), "append_signature");

  new KAction(i18n("&Insert File..."), 0, this, SLOT(slotInsertFile()),
                   actionCollection(), "insert_file");

  new KAction(i18n("Insert File (in a &box)..."), 0, this, SLOT(slotInsertFileBoxed()),
                   actionCollection(), "insert_file_boxed");

  new KAction(i18n("Attach &File..."), "attach", 0, this, SLOT(slotAttachFile()),
                   actionCollection(), "attach_file");

  a_ctPGPsign = new KToggleAction(i18n("Sign Article with &PGP"),
		   "signature", 0,
                   actionCollection(), "sign_article");

  a_ctRemoveAttachment = new KAction(i18n("&Remove"), 0, this,
                                    SLOT(slotRemoveAttachment()), actionCollection(), "remove_attachment");

  a_ctAttachmentProperties  = new KAction(i18n("&Properties..."), 0, this,
                                          SLOT(slotAttachmentProperties()), actionCollection(), "attachment_properties");

  //options menu

  a_ctDoPost = new KToggleAction(i18n("Send &News Article"), "filenew", 0 , this,
                   SLOT(slotToggleDoPost()), actionCollection(), "send_news");

  a_ctDoMail = new KToggleAction(i18n("Send E&mail"), "mail_generic" , 0 , this,
                   SLOT(slotToggleDoMail()), actionCollection(), "send_mail");

  a_ctSetCharset = new KSelectAction(i18n("Set &Charset"), 0, actionCollection(), "set_charset");
  a_ctSetCharset->setItems(knGlobals.cfgManager->postNewsTechnical()->composerCharsets());
  a_ctSetCharset->setShortcutConfigurable(false);
  connect(a_ctSetCharset, SIGNAL(activated(const QString&)),
  this, SLOT(slotSetCharset(const QString&)));

  a_ctSetCharsetKeyb = new KAction(i18n("Set Charset"), 0, this,
                                   SLOT(slotSetCharsetKeyboard()), actionCollection(), "set_charset_keyboard");


  a_ctWordWrap  = new KToggleAction(i18n("&Word Wrap"), 0 , this,
                      SLOT(slotToggleWordWrap()), actionCollection(), "toggle_wordwrap");

  //tools menu

  new KAction(i18n("Add &Quote Characters"), 0, v_iew->e_dit,
              SLOT(slotAddQuotes()), actionCollection(), "tools_quote");

  new KAction(i18n("&Remove Quote Characters"), 0, v_iew->e_dit,
              SLOT(slotRemoveQuotes()), actionCollection(), "tools_unquote");

  new KAction(i18n("Add &Box"), 0, v_iew->e_dit,
              SLOT(slotAddBox()), actionCollection(), "tools_box");

  new KAction(i18n("Re&move Box"), 0, v_iew->e_dit,
              SLOT(slotRemoveBox()), actionCollection(), "tools_unbox");

  KAction *undoRewrap = new KAction(i18n("Get &Original Text (not re-wrapped)"), 0, this,
                                    SLOT(slotUndoRewrap()), actionCollection(), "tools_undoRewrap");
  undoRewrap->setEnabled(!u_nwraped.isNull());

  KAction *rot13 = new KAction(i18n("S&cramble (Rot 13)"), "encrypted", 0, v_iew->e_dit,
                               SLOT(slotRot13()), actionCollection(), "tools_rot13");
  rot13->setEnabled(false);
  connect(v_iew->e_dit, SIGNAL(copyAvailable(bool)), rot13, SLOT(setEnabled(bool)));

  a_ctExternalEditor = new KAction(i18n("Start &External Editor"), "run", 0, this,
                       SLOT(slotExternalEditor()), actionCollection(), "external_editor");

  a_ctSpellCheck = KStdAction::spelling (this, SLOT(slotSpellcheck()), actionCollection());

  //settings menu
  createStandardStatusBarAction();
  setStandardToolBarMenuEnabled(true);

  KStdAction::keyBindings(this, SLOT(slotConfKeys()), actionCollection());

  KStdAction::configureToolbars(this, SLOT(slotConfToolbar()), actionCollection());

  KStdAction::preferences(knGlobals.top, SLOT(slotSettings()), actionCollection());

  a_ccel=new KAccel(this);
  a_ctSetCharsetKeyb->plugAccel(a_ccel);

  createGUI("kncomposerui.rc");

  //---------------------------------- </Actions> ----------------------------------------

  //editor popup
  e_ditPopup=static_cast<QPopupMenu*> (factory()->container("edit", this));
  if(!e_ditPopup) e_ditPopup = new QPopupMenu();
  v_iew->e_dit->installRBPopup(e_ditPopup);

  //attachment popup
  a_ttPopup=static_cast<QPopupMenu*> (factory()->container("attachment_popup", this));
  if(!a_ttPopup) a_ttPopup = new QPopupMenu();
  slotAttachmentSelected(0);

  //init
  initData(text);

  //apply configuration
  setConfig(false);

  if (firstEdit) {   // now we place the cursor at the end of the quoted text / below the attribution line
    if (knGlobals.cfgManager->postNewsComposer()->cursorOnTop())
      v_iew->e_dit->setCursorPosition(1,0);
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

  if(firstEdit && knGlobals.cfgManager->postNewsComposer()->appendOwnSignature())
    slotAppendSig();

  if (createCopy && (m_ode==news)) {
    a_ctDoMail->setChecked(true);
    slotToggleDoMail();
  }

  v_iew->e_dit->setModified(false);

  // restore window & toolbar configuration
  KConfig *conf = KGlobal::config();
  conf->setGroup("composerWindow_options");
  resize(535,450);    // default optimized for 800x600
  applyMainWindowSettings(conf);

  // starting the external editor
  if(knGlobals.cfgManager->postNewsComposer()->useExternalEditor())
    slotExternalEditor();
}


KNComposer::~KNComposer()
{
  delete s_pellChecker;
  delete e_xternalEditor;  // this also kills the editor process if it's still running
  if(e_ditorTempfile) {
    e_ditorTempfile->unlink();
    delete e_ditorTempfile;
  }
  KConfig *conf = KGlobal::config();
  conf->setGroup("composerWindow_options");
  saveMainWindowSettings(conf);
}


void KNComposer::setConfig(bool onlyFonts)
{
  if (!onlyFonts) {
    v_iew->e_dit->setWordWrap(knGlobals.cfgManager->postNewsComposer()->wordWrap()?
                              QMultiLineEdit::FixedColumnWidth : QMultiLineEdit::NoWrap);
    v_iew->e_dit->setWrapColumnOrWidth(knGlobals.cfgManager->postNewsComposer()->maxLineLength());
    a_ctWordWrap->setChecked(knGlobals.cfgManager->postNewsComposer()->wordWrap());

    Kpgp::Module *pgp = Kpgp::Module::getKpgp();
    a_ctPGPsign->setEnabled(pgp->usePGP());
  }

  QFont fnt=knGlobals.cfgManager->appearance()->composerFont();
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
    QString s = v_iew->e_dit->textLine(0);
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
    KMessageBox::sorry(this, i18n("Please enter a subject!"));
    return false;
  }
  if (!n_eeds8Bit && !KMime::isUsAscii(v_iew->s_ubject->text()))
    n_eeds8Bit=true;

  if (m_ode != mail) {
    if (v_iew->g_roups->text().isEmpty()) {
      KMessageBox::sorry(this, i18n("Please enter a newsgroup!"));
      return false;
    }

    int groupCount = QStringList::split(',',v_iew->g_roups->text()).count();
    int fupCount = QStringList::split(',',v_iew->f_up2->currentText()).count();
    bool followUp = !v_iew->f_up2->currentText().isEmpty();

    if (groupCount>12) {
      KMessageBox::sorry(this, i18n("You are crossposting to more than 12 newsgroups.\nPlease remove all newsgroups in which your article is off-topic!"));
      return false;
    }

    if (groupCount>5)
      if (!(KMessageBox::warningYesNo( this, i18n("You are crossposting to more than five newsgroups.\nPlease reconsider whether this is really useful\nand remove groups in which your article is off-topic.\nDo you want to re-edit the article or send it anyway?"),
                                       QString::null, i18n("&Send"),i18n("edit article","&Edit")) == KMessageBox::Yes))
        return false;

    if (!followUp && (groupCount>2))
      if (!(KMessageBox::warningYesNo( this, i18n("You are crossposting to more than two newsgroups.\nPlease use the \"Followup-To\" header to direct\nthe replies to your article into one group.\nDo you want to re-edit the article or send it anyway?"),
                                       QString::null, i18n("&Send"),i18n("edit article","&Edit")) == KMessageBox::Yes))
        return false;

    if (fupCount>12) {
      KMessageBox::sorry(this, i18n("You are directing replies to more than 12 newsgroups.\nPlease remove some newsgroups from the \"Followup-To\" header!"));
      return false;
    }

    if (fupCount>5)
      if (!(KMessageBox::warningYesNo( this, i18n("You are directing replies to more than five newsgroups.\nPlease reconsider whether this is really useful.\nDo you want to re-edit the article or send it anyway?"),
                                       QString::null, i18n("&Send"),i18n("edit article","&Edit")) == KMessageBox::Yes))
        return false;
  }

  if (m_ode != news) {
    if (v_iew->t_o->text().isEmpty() ) {
      KMessageBox::sorry(this, i18n("Please enter the email address!"));
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
  QStringList text = v_iew->e_dit->processedText();

  for (QStringList::Iterator it = text.begin(); it != text.end(); ++it) {

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
    KMessageBox::sorry(this, i18n("Your message contains characters that aren't included\nin the \"us-ascii\" character set. Please choose\na suitable character set in the \"Options\" menu!"));
    return false;
  }

  if (empty) {
    KMessageBox::sorry(this, i18n("You can't post an empty message!"));
    return false;
  }

  if ((textLines>1)&&(notQuoted==1)) {
    if (hasAttributionLine)
      if (!(KMessageBox::warningYesNo( this, i18n("Your article seems to consist entirely of quoted text.\nDo you want to re-edit the article or send it anyway?"),
                                       QString::null, i18n("&Send"),i18n("edit article","&Edit")) == KMessageBox::Yes))
        return false;
  } else {
    if (notQuoted==0) {
      KMessageBox::sorry(this, i18n("You can't post an article consisting\n"
			      "entirely of quoted text!"));
      return false;
    }
  }

  if (longLine)
    if (!(KMessageBox::warningYesNo( this,
          i18n("Your article contains lines longer than 80 characters.\n"
	       "Do you want to re-edit the article or send it anyway?"),
          QString::null, i18n("&Send"),
	  i18n("edit article","&Edit")) == KMessageBox::Yes))
      return false;

  if (sigLength>8) {
    if (!(KMessageBox::warningYesNo( this, i18n("Your signature is more than 8 lines long.\nYou should shorten it to match the widely accepted limit of 4 lines.\nDo you want to re-edit the article or send it anyway?"),
                                     QString::null, i18n("&Send"),i18n("edit article","&Edit")) == KMessageBox::Yes))
      return false;
  } else
    if (sigLength>4)
       KMessageBox::information(this, i18n("Your signature exceeds the widely accepted limit of 4 lines.\nPlease consider shortening your signature.\nOtherwise, you will probably annoy your readers"),
                                QString::null,"longSignatureWarning");

  // check if article can be signed
  if ( a_ctPGPsign->isChecked() ) {
    // try to get the signing key
    QCString signingKey = knGlobals.cfgManager->identity()->signingKey();
    KNNntpAccount *acc = knGlobals.accManager->account( a_rticle->serverId() );
    if ( acc ) {
      KMime::Headers::Newsgroups *grps = a_rticle->newsgroups();
      KNGroup *grp = knGlobals.grpManager->group( grps->firstGroup(), acc );
      if ( grp && grp->identity() && grp->identity()->hasSigningKey() )
        signingKey = grp->identity()->signingKey();
      else if ( acc->identity() && acc->identity()->hasSigningKey() )
        signingKey = acc->identity()->signingKey();
    }

    // the article can only be signed if we have a key
    if (signingKey.isEmpty()) {
          if ( KMessageBox::warningContinueCancel( this,
                   i18n("You haven't configured your preferred "
                        "signing key yet.\n"
                        "Please specify it in the global "
                        "identity configuration,\n"
                        "in the account properties or in the "
                        "group properties!\n"
                        "The article will be sent unsigned." ),
                   QString::null, i18n( "Send Unsigned" ),
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
    a_rticle->newsgroups()->fromUnicodeString(v_iew->g_roups->text().remove(QRegExp("\\s")), KMime::Headers::Latin1);
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

    QListViewItemIterator it(v_iew->a_ttView);
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

  if(!d_elAttList.isEmpty())
      for(KNAttachment *a=d_elAttList.first(); a; a=d_elAttList.next())
        if(a->isAttached())
          a->detach(a_rticle);

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
  KNConfig::PostNewsTechnical *pnt=knGlobals.cfgManager->postNewsTechnical();
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
  QString tmp;
  QStringList textLines = v_iew->e_dit->processedText();
  for (QStringList::Iterator it = textLines.begin(); it != textLines.end(); ++it)
    tmp += *it + "\n";

  // Sign article if needed
  if ( a_ctPGPsign->isChecked() ) {
      // first get the signing key
      QCString signingKey = knGlobals.cfgManager->identity()->signingKey();
      KNNntpAccount *acc = knGlobals.accManager->account( a_rticle->serverId() );
      if ( acc ) {
          KMime::Headers::Newsgroups *grps = a_rticle->newsgroups();
          KNGroup *grp = knGlobals.grpManager->group( grps->firstGroup(), acc );
          if ( grp && grp->identity() && grp->identity()->hasSigningKey() )
              signingKey = grp->identity()->signingKey();
          else if ( acc->identity() && acc->identity()->hasSigningKey() )
              signingKey = acc->identity()->signingKey();
      }
      // now try to sign the article
      if (!signingKey.isEmpty()) {
          QString tmpText = tmp;
          Kpgp::Block block;
          bool ok=true;
          QTextCodec *codec=KGlobal::charsets()->codecForName(c_harset, ok);
          if(!ok) // no suitable codec found => try local settings and hope the best ;-)
              codec=KGlobal::locale()->codecForEncoding();

          block.setText( codec->fromUnicode(tmpText) );
          kdDebug(5003) << "signing article from " << article()->from()->email() << endl;
          if( block.clearsign( signingKey, codec->name() ) == Kpgp::Ok ) {
              QCString result = block.text();
              tmp = codec->toUnicode(result.data(), result.length() );
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


void KNComposer::closeEvent(QCloseEvent *e)
{
  if(!v_iew->e_dit->isModified() && !a_ttChanged) {  // nothing to save, don't show nag screen
    if(a_rticle->id()==-1)
      r_esult=CRdel;
    else
      r_esult=CRcancel;
  }
  else {
    switch ( KMessageBox::warningYesNoCancel( this, i18n("Do you want to save this article in the draft folder?"),
                                              QString::null, i18n("&Save"), i18n("&Discard"))) {
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


void KNComposer::initData(const QString &text)
{
  //Subject
  if(a_rticle->subject()->isEmpty())
    slotSubjectChanged(QString::null);
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
  QString s;

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
    c_harset=knGlobals.cfgManager->postNewsTechnical()->charset();

  a_ctSetCharset->setCurrentItem(knGlobals.cfgManager->postNewsTechnical()->indexForCharset(c_harset));

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
void KNComposer::insertFile(QFile *file, bool clear, bool box, QString boxTitle)
{
  QString temp;
  bool ok=true;
  QTextCodec *codec=KGlobal::charsets()->codecForName(c_harset, ok);
  QTextStream ts(file);
  ts.setCodec(codec);

  if (box)
    temp = QString::fromLatin1(",----[ %1 ]\n").arg(boxTitle);

  if (box && (v_iew->e_dit->wordWrap()!=QMultiLineEdit::NoWrap)) {
    int wrapAt = v_iew->e_dit->wrapColumnOrWidth();
    QStringList lst;
    QString line;
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
    temp += QString::fromLatin1("`----");

  if(clear)
    v_iew->e_dit->setText(temp);
  else
    v_iew->e_dit->insert(temp);
}


// ask for a filename, handle network urls
void KNComposer::insertFile(bool clear, bool box)
{
  KNLoadHelper helper(this);
  QFile *file = helper.getFile(i18n("Insert File"));
  KURL url;
  QString boxName;

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


void KNComposer::slotSendNow()
{
  r_esult=CRsendNow;
  emit composerDone(this);
}


void KNComposer::slotSendLater()
{
  r_esult=CRsendLater;
  emit composerDone(this);
}


void KNComposer::slotSaveAsDraft()
{
  r_esult=CRsave;
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
      d_elAttList.append(it->attachment);
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
      if (!(KMessageBox::warningContinueCancel(this, i18n("The poster doesn't want a mail copy of your reply (Mail-Copies-To: nobody).\nPlease respect his request."),
                                               QString::null, i18n("&Send Copy")) == KMessageBox::Continue)) {
        a_ctDoMail->setChecked(false); //revert
        return;
      }
    }

    if (knGlobals.cfgManager->postNewsTechnical()->useExternalMailer()) {
      QString s = v_iew->e_dit->textLine(0);
      if (!s.contains(i18n("<posted & mailed>")))
        v_iew->e_dit->insertAt(i18n("<posted & mailed>\n\n"),0,0);
      QString tmp;
      QStringList textLines = v_iew->e_dit->processedText();
      for (QStringList::Iterator it = textLines.begin(); it != textLines.end(); ++it) {
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


void KNComposer::slotSetCharset(const QString &s)
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
  v_iew->e_dit->setWordWrap(a_ctWordWrap->isChecked()? QMultiLineEdit::FixedColumnWidth : QMultiLineEdit::NoWrap);
}


void KNComposer::slotUndoRewrap()
{
  if (KMessageBox::warningContinueCancel( this, i18n("This will replace all text you have written!"),
                                         QString::null, QString::null) == KMessageBox::Continue) {
    v_iew->e_dit->setText(u_nwraped);
    slotAppendSig();
  }
}

void KNComposer::slotExternalEditor()
{
  if(e_xternalEditor)   // in progress...
    return;

  QString editorCommand=knGlobals.cfgManager->postNewsComposer()->externalEditor();

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
  QTextCodec *codec=KGlobal::charsets()->codecForName(c_harset, ok);

  QString tmp;
  QStringList textLines = v_iew->e_dit->processedText();
  for (QStringList::Iterator it = textLines.begin(); it != textLines.end();) {
    tmp += *it;
    ++it;
    if (it != textLines.end())
      tmp+="\n";
  }

  QCString local = codec->fromUnicode(tmp);
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
  QStringList command = QStringList::split(' ',editorCommand);
  bool filenameAdded=false;
  for ( QStringList::Iterator it = command.begin(); it != command.end(); ++it ) {
    if ((*it).contains("%f")) {
      (*it).replace(QRegExp("%f"),e_ditorTempfile->name());
      filenameAdded=true;
    }
    (*e_xternalEditor) << (*it);
  }
  if(!filenameAdded)    // no %f in the editor command
    (*e_xternalEditor) << e_ditorTempfile->name();

  connect(e_xternalEditor, SIGNAL(processExited(KProcess *)),this, SLOT(slotEditorFinished(KProcess *)));
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

  a_ctExternalEditor->setEnabled(false);
  a_ctSpellCheck->setEnabled(false);

  s_pellChecker = new KSpell(this, i18n("Spellcheck"), this, SLOT(slotSpellStarted(KSpell *)));
  QStringList l = SpellChecker::personalWords();
  for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
      s_pellChecker->addPersonal( *it );
  }
  connect(s_pellChecker, SIGNAL(death()), this, SLOT(slotSpellFinished()));
  connect(s_pellChecker, SIGNAL(done(const QString&)), this, SLOT(slotSpellDone(const QString&)));
  connect(s_pellChecker, SIGNAL(misspelling (const QString &, const QStringList &, unsigned int)),
          v_iew->e_dit, SLOT(misspelling (const QString &, const QStringList &, unsigned int)));
  connect(s_pellChecker, SIGNAL(corrected (const QString &, const QString &, unsigned int)),
          v_iew->e_dit, SLOT(corrected (const QString &, const QString &, unsigned int)));
}

void KNComposer::slotUpdateStatusBar()
{
  QString typeDesc;
  switch (m_ode) {
    case news:  typeDesc = i18n("News Article");
                break;
    case mail:  typeDesc = i18n("Email");
                break;
    default  :  typeDesc = i18n("News Article & Email");
  }
  QString overwriteDesc;
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
  KKeyDialog::configureKeys(actionCollection(), xmlFile(), true, this);
}


void KNComposer::slotConfToolbar()
{
  KConfig *conf = KGlobal::config();
  conf->setGroup("composerWindow_options");
  saveMainWindowSettings(conf);
  KEditToolbar dlg(guiFactory(),this);
  connect(&dlg,SIGNAL( newToolbarConfig() ), this, SLOT( slotNewToolbarConfig() ));
  dlg.exec();
}

void KNComposer::slotNewToolbarConfig()
{
  createGUI("kncomposerui.rc");

  e_ditPopup=static_cast<QPopupMenu*> (factory()->container("edit", this));
  if(!e_ditPopup) e_ditPopup = new QPopupMenu();
  v_iew->e_dit->installRBPopup(e_ditPopup);
  a_ttPopup=static_cast<QPopupMenu*> (factory()->container("attachment_popup", this));
  if(!a_ttPopup) a_ttPopup = new QPopupMenu();

  KConfig *conf = KGlobal::config();
  conf->setGroup("composerWindow_options");
  applyMainWindowSettings(conf);
}

//-------------------------------- </Actions> -----------------------------------


void KNComposer::slotSubjectChanged(const QString &t)
{
  if(!t.isEmpty()) setCaption(t);
  else setCaption(i18n("No Subject"));
}


void KNComposer::slotGroupsChanged(const QString &t)
{
  KQCStringSplitter split;
  bool splitOk;
  QString currText=v_iew->f_up2->currentText();

  v_iew->f_up2->clear();

  split.init(t.latin1(), ",");
  splitOk=split.first();
  while(splitOk) {
    v_iew->f_up2->insertItem(QString::fromLatin1(split.string()));
    splitOk=split.next();
  }
  v_iew->f_up2->insertItem("");

  if(!currText.isEmpty())
    v_iew->f_up2->lineEdit()->setText(currText);
}


void KNComposer::slotToBtnClicked()
{
  KABC::Addressee a = KABC::AddresseeDialog::getAddressee( this );
  if ( a.isEmpty() ) return;

  QString to=v_iew->t_o->text();
  if(!to.isEmpty()) to+=", ";
  to+=a.fullEmail();
  v_iew->t_o->setText(to);
}


void KNComposer::slotGroupsBtnClicked()
{
  int id=a_rticle->serverId();
  KNNntpAccount *nntp=0;

  if(id!=-1)
    nntp=knGlobals.accManager->account(id);

  if(!nntp)
    nntp=knGlobals.accManager->first();

  if(!nntp) {
    KMessageBox::error(this, i18n("You have no valid news accounts configured!"));
    v_iew->g_roups->clear();
    return;
  }

  if(id==-1)
    a_rticle->setServerId(nntp->id());

  KNGroupSelectDialog *dlg=new KNGroupSelectDialog(this, nntp, v_iew->g_roups->text().remove(QRegExp("\\s")));

  connect(dlg, SIGNAL(loadList(KNNntpAccount*)),
    knGlobals.grpManager, SLOT(slotLoadGroupList(KNNntpAccount*)));
  connect(knGlobals.grpManager, SIGNAL(newListReady(KNGroupListData*)),
    dlg, SLOT(slotReceiveList(KNGroupListData*)));

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


void KNComposer::slotAttachmentPopup(QListViewItem *it, const QPoint &p, int)
{
  if(it)
    a_ttPopup->popup(p);
}


void KNComposer::slotAttachmentSelected(QListViewItem *it)
{
  if(v_iew->a_ttWidget) {
    v_iew->a_ttRemoveBtn->setEnabled((it!=0));
    v_iew->a_ttEditBtn->setEnabled((it!=0));
  }
}


void KNComposer::slotAttachmentEdit(QListViewItem *)
{
  slotAttachmentProperties();
}


void KNComposer::slotAttachmentRemove(QListViewItem *)
{
  slotRemoveAttachment();
}


//==============================================================================
// spellchecking code copied form kedit (Bernd Johannes Wuebben)
//==============================================================================


void KNComposer::slotSpellStarted( KSpell *)
{
  v_iew->e_dit->spellcheck_start();
  s_pellChecker->setProgressResolution(2);

  // read the quote indicator from the preferences
  KConfig *config=KGlobal::config();
  KConfigGroupSaver saver(config, "READNEWS");
  QString quotePrefix;
  quotePrefix = config->readEntry("quoteCharacters",">");
//todo fixme
//quotePrefix = mComposer->msg()->formatString(quotePrefix);

  kdDebug() << "spelling: new SpellingFilter with prefix=\"" << quotePrefix << "\"" << endl;
  mSpellingFilter = new SpellingFilter(v_iew->e_dit->text(), quotePrefix, SpellingFilter::FilterUrls,
    SpellingFilter::FilterEmailAddresses);

  s_pellChecker->check(mSpellingFilter->filteredText());
}

void KNComposer::slotSpellDone(const QString &newtext)
{
  a_ctExternalEditor->setEnabled(true);
  a_ctSpellCheck->setEnabled(true);
  v_iew->e_dit->spellcheck_stop();

  int dlgResult = s_pellChecker->dlgResult();
  if ( dlgResult == KS_CANCEL )
  {

      kdDebug() << "spelling: canceled - restoring text from SpellingFilter" << endl;
      kdDebug()<<" mSpellingFilter->originalText() :"<<mSpellingFilter->originalText()<<endl;
    v_iew->e_dit->setText(mSpellingFilter->originalText());

    //v_iew->e_dit->setModified(mWasModifiedBeforeSpellCheck);
  }
  s_pellChecker->cleanUp();
  DictSpellChecker::dictionaryChanged();

  delete s_pellChecker;
  s_pellChecker=0;
  delete mSpellingFilter;
  mSpellingFilter = 0;
}


void KNComposer::slotSpellFinished()
{
  a_ctExternalEditor->setEnabled(true);
  a_ctSpellCheck->setEnabled(true);
  KSpell::spellStatus status=s_pellChecker->status();
  delete s_pellChecker;
  s_pellChecker=0;

  if(status==KSpell::Error) {
    KMessageBox::error(this, i18n("ISpell could not be started.\n"
    "Please make sure you have ISpell properly configured and in your PATH."));
  }
  else if(status==KSpell::Crashed) {
    v_iew->e_dit->spellcheck_stop();
    KMessageBox::error(this, i18n("ISpell seems to have crashed."));
  }
}
#if 0
//-----------------------------------------------------------------------------
void KNComposer::slotSpellResult(const QString &)
{
  v_iew->e_dit->spellcheck_stop();

  int dlgResult = s_pellChecker->dlgResult();
  if ( dlgResult == KS_CANCEL )
  {

      kdDebug() << "spelling: canceled - restoring text from SpellingFilter" << endl;
      kdDebug()<<" mSpellingFilter->originalText() :"<<mSpellingFilter->originalText()<<endl;
    v_iew->e_dit->setText(mSpellingFilter->originalText());

    //v_iew->e_dit->setModified(mWasModifiedBeforeSpellCheck);
  }
  s_pellChecker->cleanUp();
  DictSpellChecker::dictionaryChanged();

  //emit v_iew->e_dit->spellcheck_done( dlgResult );
}
#endif

void KNComposer::slotDragEnterEvent(QDragEnterEvent *ev)
{
  QStringList files;
  ev->accept(QUriDrag::canDecode(ev));
}


void KNComposer::slotDropEvent(QDropEvent *ev)
{
  QStrList urls;
  char *s;

  if (!QUriDrag::decode(ev, urls))
    return;

  for (s = urls.first(); s != 0; s=urls.next()) {
    KURL url(s);
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


void KNComposer::dragEnterEvent(QDragEnterEvent *ev)
{
  slotDragEnterEvent(ev);
}


void KNComposer::dropEvent(QDropEvent *ev)
{
  slotDropEvent(ev);
}

//=====================================================================================


KNComposer::ComposerView::ComposerView(QWidget *p, const char *n)
  : QSplitter(QSplitter::Vertical, p, n), a_ttWidget(0), a_ttView(0), v_iewOpen(false)
{
  QWidget *main=new QWidget(this);

  //headers
  QFrame *hdrFrame=new QFrame(main);
  hdrFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);
  QGridLayout *hdrL=new QGridLayout(hdrFrame, 4,3, 7,5);
  hdrL->setColStretch(1,1);

  //To
  t_o=new KLineEdit(hdrFrame);
  l_to=new QLabel(t_o, i18n("T&o:"), hdrFrame);
  t_oBtn=new QPushButton(i18n("&Browse..."), hdrFrame);
  hdrL->addWidget(l_to, 0,0);
  hdrL->addWidget(t_o, 0,1);
  hdrL->addWidget(t_oBtn, 0,2);
  connect(t_oBtn, SIGNAL(clicked()), parent(), SLOT(slotToBtnClicked()));

  //Newsgroups
  g_roups=new KLineEdit(hdrFrame);
  l_groups=new QLabel(g_roups, i18n("&Groups:"), hdrFrame);
  g_roupsBtn=new QPushButton(i18n("B&rowse..."), hdrFrame);
  hdrL->addWidget(l_groups, 1,0);
  hdrL->addWidget(g_roups, 1,1);
  hdrL->addWidget(g_roupsBtn, 1,2);
  connect(g_roups, SIGNAL(textChanged(const QString&)),
          parent(), SLOT(slotGroupsChanged(const QString&)));
  connect(g_roupsBtn, SIGNAL(clicked()), parent(), SLOT(slotGroupsBtnClicked()));

  //Followup-To
  f_up2=new KComboBox(true, hdrFrame);
  l_fup2=new QLabel(f_up2, i18n("Follo&wup-To:"), hdrFrame);
  hdrL->addWidget(l_fup2, 2,0);
  hdrL->addMultiCellWidget(f_up2, 2,2, 1,2);

  //subject
  s_ubject=new KLineEdit(hdrFrame);
  QLabel *l=new QLabel(s_ubject, i18n("S&ubject:"), hdrFrame);
  hdrL->addWidget(l, 3,0);
  hdrL->addMultiCellWidget(s_ubject, 3,3, 1,2);
  connect(s_ubject, SIGNAL(textChanged(const QString&)),
          parent(), SLOT(slotSubjectChanged(const QString&)));

  //Editor
  e_dit=new Editor(main);
  e_dit->setMinimumHeight(50);

  KConfig *config = kapp->config();
  KConfigGroupSaver saver(config, "VISUAL_APPEARANCE");
  QColor defaultColor1( kapp->palette().active().text()); // defaults from kmreaderwin.cpp
  QColor defaultColor2( kapp->palette().active().text() );
  QColor defaultColor3( kapp->palette().active().text() );
  QColor defaultForeground( kapp->palette().active().text() );
  QColor col1 = config->readColorEntry( "ForegroundColor", &defaultForeground );
  QColor col2 = config->readColorEntry( "quote3Color", &defaultColor3 );
  QColor col3 = config->readColorEntry( "quote2Color", &defaultColor2 );
  QColor col4 = config->readColorEntry( "quote1Color", &defaultColor1 );
  QColor c = QColor("red");
  mSpellChecker = new DictSpellChecker(e_dit, /*active*/ true, /*autoEnabled*/ true,
    /*spellColor*/ config->readColorEntry("NewMessage", &c),
    /*colorQuoting*/ true, col1, col2, col3, col4);

  QVBoxLayout *notL=new QVBoxLayout(e_dit);
  notL->addStretch(1);
  n_otification=new QGroupBox(2, Qt::Horizontal, e_dit);
  l=new QLabel(i18n("You are currently editing the article body\nin an external editor. To continue, you have\nto close the external editor."), n_otification);
  c_ancelEditorBtn=new QPushButton(i18n("&Kill External Editor"), n_otification);
  n_otification->setFrameStyle(QFrame::Panel | QFrame::Raised);
  n_otification->setLineWidth(2);
  n_otification->hide();
  notL->addWidget(n_otification, 0, Qt::AlignHCenter);
  notL->addStretch(1);

  //finish GUI
  QVBoxLayout *topL=new QVBoxLayout(main, 4,4);
  topL->addWidget(hdrFrame);
  topL->addWidget(e_dit, 1);
}


KNComposer::ComposerView::~ComposerView()
{
  if(v_iewOpen) {
    KConfig *conf=KGlobal::config();
    conf->setGroup("POSTNEWS");

    conf->writeEntry("Att_Splitter",sizes());   // save splitter pos

    QValueList<int> lst;                        // save header sizes
    QHeader *h=a_ttView->header();
    for (int i=0; i<5; i++)
      lst << h->sectionSize(i);
    conf->writeEntry("Att_Headers",lst);
  }
  delete mSpellChecker;
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


void KNComposer::ComposerView::showAttachmentView()
{
  if(!a_ttWidget) {
    a_ttWidget=new QWidget(this);
    QGridLayout *topL=new QGridLayout(a_ttWidget, 3, 2, 4, 4);

    a_ttView=new AttachmentView(a_ttWidget);
    topL->addMultiCellWidget(a_ttView, 0,2, 0,0);

    //connections
    connect(a_ttView, SIGNAL(currentChanged(QListViewItem*)),
            parent(), SLOT(slotAttachmentSelected(QListViewItem*)));
    connect(a_ttView, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
            parent(), SLOT(slotAttachmentPopup(QListViewItem*, const QPoint&, int)));
    connect(a_ttView, SIGNAL(delPressed(QListViewItem*)),
            parent(), SLOT(slotAttachmentRemove(QListViewItem*)));
    connect(a_ttView, SIGNAL(doubleClicked(QListViewItem*)),
            parent(), SLOT(slotAttachmentEdit(QListViewItem*)));
    connect(a_ttView, SIGNAL(returnPressed(QListViewItem*)),
            parent(), SLOT(slotAttachmentEdit(QListViewItem*)));

    //buttons
    a_ttAddBtn=new QPushButton(i18n("A&dd"),a_ttWidget);
    connect(a_ttAddBtn, SIGNAL(clicked()), parent(), SLOT(slotAttachFile()));
    topL->addWidget(a_ttAddBtn, 0,1);

    a_ttRemoveBtn=new QPushButton(i18n("&Remove"), a_ttWidget);
    a_ttRemoveBtn->setEnabled(false);
    connect(a_ttRemoveBtn, SIGNAL(clicked()), parent(), SLOT(slotRemoveAttachment()));
    topL->addWidget(a_ttRemoveBtn, 1,1);

    a_ttEditBtn=new QPushButton(i18n("&Properties"), a_ttWidget);
    a_ttEditBtn->setEnabled(false);
    connect(a_ttEditBtn, SIGNAL(clicked()), parent(), SLOT(slotAttachmentProperties()));
    topL->addWidget(a_ttEditBtn, 2,1, Qt::AlignTop);

    topL->setRowStretch(2,1);
    topL->setColStretch(0,1);
  }

  if(!v_iewOpen) {
    v_iewOpen=true;
    a_ttWidget->show();

    KConfig *conf=KGlobal::config();
    conf->setGroup("POSTNEWS");

    QValueList<int> lst=conf->readIntListEntry("Att_Splitter");
    if(lst.count()!=2)
      lst << 267 << 112;
    setSizes(lst);

    lst=conf->readIntListEntry("Att_Headers");
    if(lst.count()==5) {
      QValueList<int>::Iterator it=lst.begin();

      QHeader *h=a_ttView->header();
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


KNComposer::Editor::Editor(QWidget *parent, char *name)
  : KEdit(parent, name)
{
  setOverwriteEnabled(true);
}


KNComposer::Editor::~Editor()
{}


// expand tabs to avoid the "tab-damage",
// auto-wraped paragraphs have to split (code taken from KEdit::saveText)
QStringList KNComposer::Editor::processedText()
{
  QStringList ret;
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
        QString parag_text = textLine(i);
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

  QString replacement;
  int tabPos;
  for (QStringList::Iterator it = ret.begin(); it != ret.end(); ++it ) {
    while ((tabPos=(*it).find('\t'))!=-1) {
      replacement.fill(QChar(' '), 8-(tabPos%8));
      (*it).replace(tabPos, 1, replacement);
    }
  }

  return ret;
}


void KNComposer::Editor::slotPasteAsQuotation()
{
  QString s = QApplication::clipboard()->text();
  if (!s.isEmpty()) {
    for (int i=0; (uint)i<s.length(); i++) {
      if ( s[i] < ' ' && s[i] != '\n' && s[i] != '\t' )
        s[i] = ' ';
    }
    s.prepend("> ");
    s.replace(QRegExp("\n"),"\n> ");
    insert(s);
  }
}


void KNComposer::Editor::slotFind()
{
  search();
}


void KNComposer::Editor::slotReplace()
{
  replace();
}


void KNComposer::Editor::slotAddQuotes()
{
  if (hasMarkedText()) {
    QString s = markedText();
    s.prepend("> ");
    s.replace(QRegExp("\n"),"\n> ");
    insert(s);
  } else {
    int l = currentLine();
    int c = currentColumn();
    QString s = textLine(l);
    s.prepend("> ");
    insertLine(s,l);
    removeLine(l+1);
    setCursorPosition(l,c+2);
  }
}


void KNComposer::Editor::slotRemoveQuotes()
{
  if (hasMarkedText()) {
    QString s = markedText();
    if (s.left(2) == "> ")
      s.remove(0,2);
    s.replace(QRegExp("\n> "),"\n");
    insert(s);
  } else {
    int l = currentLine();
    int c = currentColumn();
    QString s = textLine(l);
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
    QString s = markedText();
    s.prepend(",----[  ]\n");
    s.replace(QRegExp("\n"),"\n| ");
    s.append("\n`----");
    insert(s);
  } else {
    int l = currentLine();
    int c = currentColumn();
    QString s = QString::fromLatin1(",----[  ]\n| %1\n`----").arg(textLine(l));
    insertLine(s,l);
    removeLine(l+3);
    setCursorPosition(l+1,c+2);
  }
}


void KNComposer::Editor::slotRemoveBox()
{
  if (hasMarkedText()) {
    QString s = QString::fromLatin1("\n") + markedText() + QString::fromLatin1("\n");
    s.replace(QRegExp("\n,----[^\n]*\n"),"\n");
    s.replace(QRegExp("\n| "),"\n");
    s.replace(QRegExp("\n`----[^\n]*\n"),"\n");
    s.remove(0,1);
    s.truncate(s.length()-1);
    insert(s);
  } else {
    int l = currentLine();
    int c = currentColumn();

    QString s = textLine(l);   // test if we are in a box
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


void KNComposer::Editor::dragEnterEvent(QDragEnterEvent *ev)
{
  if (QUriDrag::canDecode(ev))
    emit(sigDragEnterEvent(ev));
  else
    KEdit::dragEnterEvent(ev);
}


void KNComposer::Editor::dropEvent(QDropEvent *ev)
{
  if (QUriDrag::canDecode(ev))
    emit(sigDropEvent(ev));
  else
    KEdit::dropEvent(ev);
}


//=====================================================================================


KNComposer::AttachmentView::AttachmentView(QWidget *parent, char *name)
 : QListView(parent, name)
{
  setFrameStyle(QFrame::WinPanel | QFrame::Sunken);  // match the QMultiLineEdit style
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


void KNComposer::AttachmentView::keyPressEvent(QKeyEvent *e)
{
  if(!e)
    return; // subclass bug

  if( (e->key()==Key_Delete) && (currentItem()) )
    emit(delPressed(currentItem()));
  else
    QListView::keyPressEvent(e);
}


//=====================================================================================


KNComposer::AttachmentViewItem::AttachmentViewItem(QListView *v, KNAttachment *a) :
  QListViewItem(v), attachment(a)
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


KNComposer::AttachmentPropertiesDlg::AttachmentPropertiesDlg(KNAttachment *a, QWidget *p, const char *n) :
  KDialogBase(p, n, true, i18n("Attachment Properties"), Help|Ok|Cancel, Ok), a_ttachment(a),
  n_onTextAsText(false)
{
  //init GUI
  QWidget *page=new QWidget(this);
  setMainWidget(page);
  QVBoxLayout *topL=new QVBoxLayout(page);

  //file info
  QGroupBox *fileGB=new QGroupBox(i18n("File"), page);
  QGridLayout *fileL=new QGridLayout(fileGB, 3,2, 15,5);

  fileL->addRowSpacing(0, fontMetrics().lineSpacing()-9);
  fileL->addWidget(new QLabel(i18n("Name:"), fileGB) ,1,0);
  fileL->addWidget(new QLabel(QString("<b>%1</b>").arg(a->name()), fileGB), 1,1, Qt::AlignLeft);
  fileL->addWidget(new QLabel(i18n("Size:"), fileGB), 2,0);
  fileL->addWidget(new QLabel(a->contentSize(), fileGB), 2,1, Qt::AlignLeft);

  fileL->setColStretch(1,1);
  topL->addWidget(fileGB);

  //mime info
  QGroupBox *mimeGB=new QGroupBox(i18n("Mime"), page);
  QGridLayout *mimeL=new QGridLayout(mimeGB, 4,2, 15,5);

  mimeL->addRowSpacing(0, fontMetrics().lineSpacing()-9);
  m_imeType=new KLineEdit(mimeGB);
  m_imeType->setText(a->mimeType());
  mimeL->addWidget(m_imeType, 1,1);
  mimeL->addWidget(new QLabel(m_imeType, i18n("&Mime-Type:"), mimeGB), 1,0);

  d_escription=new KLineEdit(mimeGB);
  d_escription->setText(a->description());
  mimeL->addWidget(d_escription, 2,1);
  mimeL->addWidget(new QLabel(d_escription, i18n("&Description:"), mimeGB), 2,0);

  e_ncoding=new QComboBox(false, mimeGB);
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
  mimeL->addWidget(new QLabel(e_ncoding, i18n("&Encoding:"), mimeGB), 3,0);

  mimeL->setColStretch(1,1);
  topL->addWidget(mimeGB);

  //connections
  connect(m_imeType, SIGNAL(textChanged(const QString&)),
    this, SLOT(slotMimeTypeTextChanged(const QString&)));

  //finish GUI
  setFixedHeight(sizeHint().height());
  KNHelper::restoreWindowSize("attProperties", this, QSize(300,250));
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
       KMessageBox::warningYesNo(this,
       i18n("You have changed the mime-type of this non-textual attachment\nto text. This might cause an error while loading or encoding the file.\nProceed?")
       ) == KMessageBox::No) return;

  KDialogBase::accept();
}


void KNComposer::AttachmentPropertiesDlg::slotMimeTypeTextChanged(const QString &text)
{
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
