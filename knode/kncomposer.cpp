/***************************************************************************
                          kncomposer.cpp  -  description
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

#include <qlabel.h>
#include <qlayout.h>
#include <qvgroupbox.h>
#include <qheader.h>
#include <qvbox.h>
#include <qtextcodec.h>

#include <klocale.h>
#include <kglobalsettings.h>
#include <kglobal.h>
#include <kcharsets.h>
#include <kmessagebox.h>
#include <kabapi.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>

#include "kngroupmanager.h"
#include "kngroupselectdialog.h"
#include "knstringsplitter.h"
#include "utilities.h"
#include "knglobals.h"
#include "kncomposer.h"
#include "knode.h"
#include "knmime.h"
#include "knconfigmanager.h"
#include "knaccountmanager.h"
#include "knnntpaccount.h"



KNComposer::KNComposer(KNLocalArticle *a, const QString &text, const QString &sig, bool firstEdit)
    : KMainWindow(0,"composerWindow"), r_esult(CRsave), a_rticle(a), s_ignature(sig), e_xternalEdited(false),
      e_xternalEditor(0), e_ditorTempfile(0), s_pellChecker(0), a_ttChanged(false)
{
  d_elAttList.setAutoDelete(true);
    
  //init v_iew
  v_iew=new ComposerView(this);
  setCentralWidget(v_iew);

  //statusbar
  KStatusBar *sb=statusBar();
  sb->insertItem(QString::null, 1,1);
  sb->setItemAlignment (1,AlignLeft | AlignVCenter);
  sb->insertItem(QString::null, 2,0);
  sb->setItemAlignment (2,AlignCenter | AlignVCenter);
  sb->insertItem(QString::null,3,0);
  sb->setItemAlignment (3,AlignCenter | AlignVCenter);
  connect(v_iew->e_dit, SIGNAL(CursorPositionChanged()), SLOT(slotUpdateStatusBar()));

  //------------------------------- <Actions> --------------------------------------

  //file menu
  new KAction(i18n("&Send Now"),"mail_send", CTRL + Key_Return , this,
    SLOT(slotSendNow()), actionCollection(), "send_now");

  new KAction(i18n("Send &Later"), "queue", 0, this,
    SLOT(slotSendLater()), actionCollection(), "send_later");

  new KAction(i18n("Save As &Draft"),"filesave", 0 , this,
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

  KStdAction::cut(v_iew->e_dit, SLOT(cut()), actionCollection());

  KAction *copy = KStdAction::copy(v_iew->e_dit, SLOT(copy()), actionCollection());
  copy->setEnabled(false);
  connect(v_iew->e_dit, SIGNAL(copyAvailable(bool)), copy, SLOT(setEnabled(bool)));

  KStdAction::paste(v_iew->e_dit, SLOT(paste()), actionCollection());

  KStdAction::selectAll(v_iew->e_dit, SLOT(selectAll()), actionCollection());

  KStdAction::find(this, SLOT(slotFind()), actionCollection());

  KStdAction::findNext(this, SLOT(slotFindNext()), actionCollection());

  KStdAction::replace(this, SLOT(slotReplace()), actionCollection());

  a_ctExternalEditor = new KAction(i18n("Start &External Editor"), "run", 0, this,
                       SLOT(slotExternalEditor()), actionCollection(), "external_editor");

  a_ctSpellCheck = KStdAction::spelling (this, SLOT(slotSpellcheck()), actionCollection(), "spellcheck");

  a_ctSetCharset = new KSelectAction(i18n("&Charset"), 0, actionCollection(), "setcharset");
  a_ctSetCharset->setItems(KNMimeBase::availableCharsets());
  connect(a_ctSetCharset, SIGNAL(activated(const QString&)),
    this, SLOT(slotSetCharset(const QString&)));


  //attach menu
  new KAction(i18n("Append &Signature"), "signature", 0 , this, SLOT(slotAppendSig()),
                   actionCollection(), "append_signature");

  new KAction(i18n("&Insert File"), 0, this, SLOT(slotInsertFile()),
                   actionCollection(), "insert_file");

  new KAction(i18n("Attach &File"), "attach", 0, this, SLOT(slotAttachFile()),
                   actionCollection(), "attach_file");

  a_ctRemoveAttachment      = new KAction(i18n("&Remove"), 0, this,
                              SLOT(slotRemoveAttachment()), actionCollection(), "remove_attachment");

  a_ctAttachmentProperties  = new KAction(i18n("&Properties"), 0, this,
                              SLOT(slotAttachmentProperties()), actionCollection(), "attachment_properties");


  //settings menu
  a_ctShowToolbar = KStdAction::showToolbar(this, SLOT(slotToggleToolBar()), actionCollection());
  a_ctShowStatusbar = KStdAction::showStatusbar(this, SLOT(slotToggleStatusBar()), actionCollection());

  KStdAction::keyBindings(this, SLOT(slotConfKeys()), actionCollection());

  KStdAction::configureToolbars(this, SLOT(slotConfToolbar()), actionCollection());

  KStdAction::preferences(knGlobals.top, SLOT(slotSettings()), actionCollection());

  createGUI("kncomposerui.rc");

  //---------------------------------- </Actions> ----------------------------------------


  //attachment popup
  a_ttPopup=static_cast<QPopupMenu*> (factory()->container("attachment_popup", this));
  if(!a_ttPopup) a_ttPopup = new QPopupMenu();
  slotAttachmentSelected(0);

  //init
  initData(text);

  //apply configuration
  setConfig();

  if(firstEdit)    // now we place the cusor at the end of the quoted text
    v_iew->e_dit->setCursorPosition(v_iew->e_dit->numLines()-1,0);
  else
    v_iew->e_dit->setCursorPosition(0,0);
  v_iew->e_dit->setFocus();

  if(firstEdit && knGlobals.cfgManager->postNewsComposer()->appendOwnSignature())
    slotAppendSig();

  v_iew->e_dit->setModified(false);

  KConfig *conf = KGlobal::config();
  conf->setGroup("composerWindow_options");
  resize(535,450);    // default optimized for 800x600
  applyMainWindowSettings(conf);
  a_ctShowToolbar->setChecked(!toolBar()->isHidden());
  a_ctShowStatusbar->setChecked(!statusBar()->isHidden());

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


void KNComposer::setConfig()
{
  v_iew->e_dit->setWordWrap(knGlobals.cfgManager->postNewsComposer()->wordWrap()?
                            QMultiLineEdit::FixedColumnWidth : QMultiLineEdit::NoWrap);
  v_iew->e_dit->setWrapColumnOrWidth(knGlobals.cfgManager->postNewsComposer()->maxLineLength());

  QFont fnt=knGlobals.cfgManager->appearance()->composerFont();
  KGlobal::charsets()->setQFont(fnt, c_harset);
  v_iew->e_dit->setFont(fnt);

  fnt=font();
  KGlobal::charsets()->setQFont(fnt, c_harset);
  v_iew->s_ubject->setFont(fnt);
  v_iew->t_o->setFont(fnt);

  slotUpdateStatusBar();
}


bool KNComposer::hasValidData()
{
  if(r_esult==CRdel || r_esult==CRdelAsk)
    return true;

  if( v_iew->s_ubject->text().isEmpty() ||
      v_iew->g_roups->text().isEmpty() &&
      v_iew->t_o->text().isEmpty() ) {
    KMessageBox::sorry(this, i18n("Please enter a subject and at least one\nnewsgroup or mail-address!"));
    return false;
  }

  //GNKSA checks
  bool empty = true;
  bool longLine = false;
  QString line;

  for(int i=0; i<v_iew->e_dit->numLines(); i++) {
    line=v_iew->e_dit->textLine(i);
    if(line == "-- ")
      break;
    if(!line.isEmpty())
      empty = false;
    if(line.length()>80) {
      longLine = true;
      break;
    }
  }

  if(empty) {
    KMessageBox::sorry(this, i18n("You can't post an empty message!"));
    return false;
  }

  if(longLine)
    return  (KMessageBox::warningYesNo( this, i18n("Your article contains lines longer than 80 characters.\nDo you want to re-edit the article or send it anyway?"),
                                              QString::null, i18n("&Send"),i18n("edit article","&Edit")) == KMessageBox::Yes);
  return true;
}


void KNComposer::applyChanges()
{

  KNMimeContent *text=0;
  KNAttachment *a=0;

  QFont::CharSet cs=KNMimeBase::stringToCharset(c_harset);


  //Subject
  a_rticle->subject()->fromUnicodeString(v_iew->s_ubject->text(), cs);

  //Newsgroups
  a_rticle->newsgroups()->fromUnicodeString(v_iew->g_roups->text(), QFont::ISO_8859_1);
  a_rticle->setDoPost(v_iew->g_roupsCB->isChecked());

  //To
  a_rticle->to()->fromUnicodeString(v_iew->t_o->text(), cs);
  a_rticle->setDoMail(v_iew->t_oCB->isChecked());

  //Followup-To
  if( a_rticle->doPost() && v_iew->f_up2CB->isChecked() && !v_iew->f_up2->currentText().isEmpty())
    a_rticle->followUpTo()->fromUnicodeString(v_iew->f_up2->currentText(), QFont::ISO_8859_1);
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
    text=new KNMimeContent();
    KNHeaders::ContentType *type=text->contentType();
    KNHeaders::CTEncoding *enc=text->contentTransferEncoding();
    KNConfig::PostNewsTechnical *pnt=knGlobals.cfgManager->postNewsTechnical();
    type->setMimeType("text/plain");
    enc->setCte((KNHeaders::contentEncoding)(pnt->encoding())); //default encoding for textual contents
    enc->setDecoded(true);
    text->assemble();
    a_rticle->addContent(text, true);
  }


  //set text
  text->contentType()->setCharset(c_harset);

  //auto-wrapped lines in v_iew->e_dit don't get an "\n", so we have to
  //assemble the text line by line
  QString tmp;
  for(int i=0; i < v_iew->e_dit->numLines(); i++)
    tmp+=v_iew->e_dit->textLine(i)+"\n";
  text->fromUnicodeString(tmp);

  //text is set and all attached contents have been assembled => now set lines
  a_rticle->lines()->setNumberOfLines(a_rticle->lineCount());

  a_rticle->assemble();
  a_rticle->updateListItem();
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

  d_oneSuccess = true;
  emit composerDone(this);
  if(d_oneSuccess)
    e->accept();
  else
    e->ignore();
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
  v_iew->g_roupsCB->setChecked(a_rticle->doPost());
  slotGroupsCheckBoxToggled(a_rticle->doPost());

  //To
  v_iew->t_o->setText(a_rticle->to()->asUnicodeString());
  v_iew->t_oCB->setChecked(a_rticle->doMail());
  slotToCheckBoxToggled(a_rticle->doMail());

  //Followup-To
  KNHeaders::FollowUpTo *fup2=a_rticle->followUpTo(false);
  if(fup2 && !fup2->isEmpty()) {
    v_iew->f_up2->lineEdit()->setText(fup2->asUnicodeString());
    v_iew->f_up2CB->setChecked(true);
    slotFupCheckBoxToggled(true);
  }
  else {
    v_iew->f_up2CB->setChecked(false);
    slotFupCheckBoxToggled(false);
  }

  KNMimeContent *textContent=a_rticle->textContent();

  if(text.isEmpty()) {
    QString decoded;
    if(textContent) {
      textContent->decodedText(decoded);
      v_iew->e_dit->setText(decoded);
    }
  }
  else
    v_iew->e_dit->setText(text);

  if(textContent)
    c_harset=textContent->contentType()->charset();
  else
    c_harset=knGlobals.cfgManager->postNewsTechnical()->charset();

  a_ctSetCharset->setCurrentItem(KNMimeBase::indexForCharset(c_harset));

  if(a_rticle->contentType()->isMultipart()) {
    v_iew->showAttachmentView();
    KNMimeContent::List attList;
    AttachmentViewItem *item=0;
    attList.setAutoDelete(false);
    a_rticle->attachments(&attList);
    for(KNMimeContent *c=attList.first(); c; c=attList.next())
      item=new AttachmentViewItem(v_iew->a_ttView, new KNAttachment(c));
  }
}


// inserts at cursor position if clear is false, replaces content otherwise
void KNComposer::insertFile(QString fileName, bool clear)
{
  QString temp;
  QFile file(fileName);
  bool ok=true;
  QTextCodec *codec=KGlobal::charsets()->codecForName(c_harset, ok);
  if(!ok) codec=KGlobal::charsets()->codecForName(KGlobal::locale()->charset());
  QTextStream ts(&file);
  ts.setCodec(codec);

  if(!file.open(IO_ReadOnly)) {
    if(clear)                // ok, not pretty, assuming that we load a tempfile when clear==true (external editor)
      displayTempFileError();
    else
      displayExternalFileError();
  }
  else {
    while(!file.atEnd())
      temp+=ts.readLine()+"\n";
  }

  if(clear) {
    v_iew->e_dit->setText(temp);
  }
  else {
    int editLine,editCol;
    v_iew->e_dit->getCursorPosition(&editLine, &editCol);
    v_iew->e_dit->insertAt(temp, editLine, editCol);
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


void KNComposer::slotSetCharset(const QString &s)
{
  if(s.isEmpty())
    return;

  c_harset=s.latin1();
  setConfig(); //adjust fonts
}


void KNComposer::slotFind()
{
  v_iew->e_dit->search();
}


void KNComposer::slotFindNext()
{
  v_iew->e_dit->repeatSearch();
}


void KNComposer::slotReplace()
{
  v_iew->e_dit->replace();
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
    displayTempFileError();
    e_ditorTempfile->unlink();
    delete e_ditorTempfile;
    e_ditorTempfile=0;
    return;
  }

  bool ok=true;
  QTextCodec *codec=KGlobal::charsets()->codecForName(c_harset, ok);

  if(!ok) codec=KGlobal::charsets()->codecForName(KGlobal::locale()->charset());

  e_ditorTempfile->file()->writeBlock(codec->fromUnicode(v_iew->e_dit->text()));
  e_ditorTempfile->close();

  if(e_ditorTempfile->status()!=0) {
    displayTempFileError();
    e_ditorTempfile->unlink();
    delete e_ditorTempfile;
    e_ditorTempfile=0;
    return;
  }

  e_xternalEditor=new KProcess();

  KNStringSplitter split;       // construct command line...
  split.init(editorCommand.latin1(), " ");
  bool filenameAdded=false;
  bool splitOk=split.first();
  while(splitOk) {
    if(split.string()=="%f") {
      (*e_xternalEditor) << e_ditorTempfile->name();
      filenameAdded=true;
    }
    else
      (*e_xternalEditor) << split.string();
    splitOk=split.next();
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

  connect(s_pellChecker, SIGNAL(death()), this, SLOT(slotSpellFinished()));
  connect(s_pellChecker, SIGNAL(done(const QString&)), this, SLOT(slotSpellDone(const QString&)));
  connect(s_pellChecker, SIGNAL(misspelling (QString, QStringList *, unsigned)),
          v_iew->e_dit, SLOT(misspelling (QString, QStringList *, unsigned)));
  connect(s_pellChecker, SIGNAL(corrected (QString, QString, unsigned)),
          v_iew->e_dit, SLOT(corrected (QString, QString, unsigned)));
}


void KNComposer::slotAppendSig()
{
  if(!s_ignature.isEmpty()) {
    v_iew->e_dit->append(s_ignature);
    v_iew->e_dit->setModified(true);
  }
}


void KNComposer::slotInsertFile()
{
  KURL url = KFileDialog::getOpenURL(QString::null, QString::null, this, i18n("Insert File"));
  QString fileName;

  if (url.isEmpty())
    return;

  if (url.isLocalFile())
    fileName = url.path();
  else
    if (!KIO::NetAccess::download(url, fileName))
      return;

  insertFile(fileName);

  if (!url.isLocalFile()) {
    KIO::NetAccess::removeTempFile(fileName);
  }
}


void KNComposer::slotAttachFile()
{
  QString path=KFileDialog::getOpenFileName(QString::null, QString::null, this, i18n("Attach File"));   // this needs to be network-transparent (CG)

  if(path.isEmpty()) // no file choosen
    return;

  if(QFile::exists(path)) {
    if (!v_iew->v_iewOpen) {
      saveWindowSize("composer", size());
      v_iew->showAttachmentView();
    }
    (void) new AttachmentViewItem(v_iew->a_ttView, new KNAttachment(path));
    a_ttChanged=true;
  }
  else
    displayExternalFileError();
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
      saveWindowSize("composerAtt", size());
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


void KNComposer::slotToggleToolBar()
{
  if(toolBar()->isVisible())
    toolBar()->hide();
  else
    toolBar()->show();
}


void KNComposer::slotToggleStatusBar()
{
  if (statusBar()->isVisible())
    statusBar()->hide();
  else
    statusBar()->show();
}


void KNComposer::slotUpdateStatusBar()
{
  statusBar()->changeItem(i18n(" Charset: %1 ").arg(c_harset), 1);
  statusBar()->changeItem(i18n(" Column: %1 ").arg(v_iew->e_dit->currentColumn()), 2);
  statusBar()->changeItem(i18n(" Line: %1 ").arg(v_iew->e_dit->currentLine()), 3);
}


void KNComposer::slotConfKeys()
{
  KKeyDialog::configureKeys(actionCollection(), xmlFile(), true, this);
}


void KNComposer::slotConfToolbar()
{
  KEditToolbar *dlg = new KEditToolbar(guiFactory(),this);
  if(dlg->exec()) {
    createGUI("kncomposerui.rc");
    a_ttPopup=static_cast<QPopupMenu*> (factory()->container("attachment_popup", this));
    if(!a_ttPopup) a_ttPopup = new QPopupMenu();
  }
  delete dlg;
}


//-------------------------------- </Actions> -----------------------------------


void KNComposer::slotSubjectChanged(const QString &t)
{
  if(!t.isEmpty()) setCaption(t);
  else setCaption(i18n("No subject"));
}


void KNComposer::slotGroupsChanged(const QString &t)
{
  KNStringSplitter split;
  bool splitOk;
  QString currText=v_iew->f_up2->currentText();

  v_iew->f_up2->clear();

  split.init(t.latin1(), ",");
  splitOk=split.first();
  if(splitOk)
    v_iew->f_up2CB->setChecked(true);

  while(splitOk) {
    v_iew->f_up2->insertItem(QString::fromLatin1(split.string()));
    splitOk=split.next();
  }

  if(!currText.isEmpty())
   v_iew->f_up2->lineEdit()->setText(currText);
}


void KNComposer::slotToBtnClicked()
{
  KabAPI *kab;
  AddressBook::Entry entry;
  KabKey key;
  QString path;

  kab=new KabAPI(this);
  if(kab->init()!=AddressBook::NoError) {
    KMessageBox::error(this, i18n("Cannot initialize the adressbook"));
    delete kab;
    return;
  }

  if(kab->exec()) {
    if(kab->getEntry(entry, key)!=AddressBook::NoError) {
      KMessageBox::error(this, i18n("Cannot read adressbook-entry"));
    }
    else {
      QString to=v_iew->t_o->text();
      if(!to.isEmpty()) to+=",";
      to+=entry.emails.first().latin1();
      v_iew->t_o->setText(to);
    }
  }
  delete kab;
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
    KMessageBox::error(this, i18n("You have no valid news-account configured!"));
    v_iew->g_roups->clear();
    v_iew->g_roupsCB->setChecked(false);
    return;
  }

  if(id==-1)
    a_rticle->setServerId(nntp->id());

  QCString grps=v_iew->g_roups->text().latin1();
  KNGroupSelectDialog *dlg=new KNGroupSelectDialog(this, nntp, grps);

  connect(dlg, SIGNAL(loadList(KNNntpAccount*)),
    knGlobals.grpManager, SLOT(slotLoadGroupList(KNNntpAccount*)));
  connect(knGlobals.grpManager, SIGNAL(newListReady(KNGroupListData*)),
    dlg, SLOT(slotReceiveList(KNGroupListData*)));

  if(dlg->exec())
    v_iew->g_roups->setText(dlg->selectedGroups());

  delete dlg;
}


void KNComposer::slotToCheckBoxToggled(bool b)
{
  v_iew->t_o->setEnabled(b);
  v_iew->t_oBtn->setEnabled(b);
}


void KNComposer::slotGroupsCheckBoxToggled(bool b)
{
  v_iew->g_roups->setEnabled(b);
  v_iew->g_roupsBtn->setEnabled(b);
  if(!b) {
    v_iew->f_up2CB->setChecked(false);
    v_iew->f_up2CB->setEnabled(false);
  }
  else {
    v_iew->f_up2CB->setEnabled(true);
    slotGroupsChanged(v_iew->g_roups->text());
  }
}


void KNComposer::slotFupCheckBoxToggled(bool b)
{
  v_iew->f_up2->setEnabled(b);
}


void KNComposer::slotEditorFinished(KProcess *)
{
  if(e_xternalEditor->normalExit()) {
    insertFile(e_ditorTempfile->name(), true);
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
  s_pellChecker->check(v_iew->e_dit->text());
}


void KNComposer::slotSpellDone(const QString &newtext)
{
  a_ctExternalEditor->setEnabled(true);
  a_ctSpellCheck->setEnabled(true);
  v_iew->e_dit->spellcheck_stop();
  if(s_pellChecker->dlgResult()==0)
    v_iew->e_dit->setText(newtext);
  s_pellChecker->cleanUp();

  delete s_pellChecker;
  s_pellChecker=0;
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


//=====================================================================================


KNComposer::ComposerView::ComposerView(QWidget *p, const char *n)
  : QSplitter(QSplitter::Vertical, p, n), a_ttWidget(0), a_ttView(0), v_iewOpen(false)
{
  QWidget *main=new QWidget(this);

  //headers
  QFrame *hdrFrame=new QFrame(main);
  hdrFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);
  QGridLayout *hdrL=new QGridLayout(hdrFrame, 4,3, 10,10);
  hdrL->setColStretch(1,1);

  //subject
  s_ubject=new QLineEdit(hdrFrame);
  QLabel *l=new QLabel(s_ubject, i18n("S&ubject:"), hdrFrame);
  hdrL->addWidget(l, 0,0);
  hdrL->addMultiCellWidget(s_ubject, 0,0, 1,2);
  connect(s_ubject, SIGNAL(textChanged(const QString&)),
    parent(), SLOT(slotSubjectChanged(const QString&)));

  //To
  t_oCB=new QCheckBox(i18n("&To:"), hdrFrame);
  hdrL->addWidget(t_oCB, 1,0);
  t_o=new QLineEdit(hdrFrame);
  hdrL->addWidget(t_o, 1,1);
  t_oBtn=new QPushButton(i18n("&Browse..."), hdrFrame);
  hdrL->addWidget(t_oBtn, 1,2);
  connect(t_oCB, SIGNAL(toggled(bool)), parent(), SLOT(slotToCheckBoxToggled(bool)));
  connect(t_oBtn, SIGNAL(clicked()), parent(), SLOT(slotToBtnClicked()));

  //Newsgroups
  g_roupsCB=new QCheckBox(i18n("&Groups:"), hdrFrame);
  hdrL->addWidget(g_roupsCB, 2,0);
  g_roups=new QLineEdit(hdrFrame);
  hdrL->addWidget(g_roups, 2,1);
  g_roupsBtn=new QPushButton(i18n("Br&owse..."), hdrFrame);
  hdrL->addWidget(g_roupsBtn, 2,2);
  connect(g_roupsCB, SIGNAL(toggled(bool)), parent(), SLOT(slotGroupsCheckBoxToggled(bool)));
  connect(g_roups, SIGNAL(textChanged(const QString&)),
    parent(), SLOT(slotGroupsChanged(const QString&)));
  connect(g_roupsBtn, SIGNAL(clicked()), parent(), SLOT(slotGroupsBtnClicked()));

  //Followup-To
  f_up2CB=new QCheckBox(i18n("&Followup-To:"), hdrFrame);
  hdrL->addWidget(f_up2CB, 3,0);
  f_up2=new QComboBox(true, hdrFrame);
  hdrL->addMultiCellWidget(f_up2, 3,3, 1,2);
  connect(f_up2CB, SIGNAL(toggled(bool)), parent(), SLOT(slotFupCheckBoxToggled(bool)));

  //Editor
  e_dit=new Editor(main);
  e_dit->setMinimumHeight(50);

  QVBoxLayout *notL=new QVBoxLayout(e_dit);
  notL->addStretch(1);
  n_otification=new QGroupBox(2, Qt::Horizontal, e_dit);
  l=new QLabel(i18n("You are currently editing the article body\nin an external editor. To continue you have\nto close the external editor."), n_otification);
  c_ancelEditorBtn=new QPushButton(i18n("&Kill external editor"), n_otification);
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
  installEventFilter(this);
}


KNComposer::Editor::~Editor()
{
  removeEventFilter(this);
}


// expand tabs to avoid the "tab-damage"
QString KNComposer::Editor::textLine(int line) const
{
  QString temp=KEdit::textLine(line);
  QString replacement;
  int tabPos;

  while((tabPos=temp.find('\t'))!=-1) {
    replacement.fill(QChar(' '), 8-(tabPos%8));
    temp.replace(tabPos, 1, replacement);
  }
  return temp;
}


// don't use Tab for focus handling
bool KNComposer::Editor::eventFilter(QObject*, QEvent* e)
{
  if (e->type() == QEvent::KeyPress) {
    QKeyEvent *k = static_cast<QKeyEvent*>(e);
    if (k->key()==Key_Tab) {
      int col, row;
      getCursorPosition(&row, &col);
      insertAt("\t", row, col);
      return true;
    }
  }
  return false;
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
  QGridLayout *fileL=new QGridLayout(fileGB, 2,2, 20,10);

  fileL->addWidget(new QLabel(i18n("Name:"), fileGB) ,0,0);
  fileL->addWidget(new QLabel(QString("<b>%1</b>").arg(a->name()), fileGB), 0,1, Qt::AlignLeft);
  fileL->addWidget(new QLabel(i18n("Size:"), fileGB), 1,0);
  fileL->addWidget(new QLabel(a->contentSize(), fileGB), 1,1, Qt::AlignLeft);

  fileL->setColStretch(1,1);
  topL->addWidget(fileGB);


  //mime info
  QGroupBox *mimeGB=new QGroupBox(i18n("Mime"), page);
  QGridLayout *mimeL=new QGridLayout(mimeGB, 3,2, 20,10);

  mimeL->addWidget(new QLabel(i18n("Mime-Type:"), mimeGB), 0,0);
  m_imeType=new QLineEdit(mimeGB);
  mimeL->addWidget(m_imeType, 0,1);
  m_imeType->setText(a->mimeType());

  mimeL->addWidget(new QLabel(i18n("Description:"), mimeGB), 1,0);
  d_escription=new QLineEdit(mimeGB);
  mimeL->addWidget(d_escription, 1,1);
  d_escription->setText(a->description());

  mimeL->addWidget(new QLabel(i18n("Encoding:"), mimeGB), 2,0);
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
  mimeL->addWidget(e_ncoding, 2,1);

  mimeL->setColStretch(1,1);
  topL->addWidget(mimeGB);


  //connections
  connect(m_imeType, SIGNAL(textChanged(const QString&)),
    this, SLOT(slotMimeTypeTextChanged(const QString&)));


  //finish GUI
  setFixedHeight(sizeHint().height());
  restoreWindowSize("attProperties", this, QSize(300,250));
  setHelp("anc-knode-editor-advanced");
}


KNComposer::AttachmentPropertiesDlg::~AttachmentPropertiesDlg()
{
  saveWindowSize("attProperties", this->size());
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
