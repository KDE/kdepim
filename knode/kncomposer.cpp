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
#include <qcheckbox.h>

#include <ktempfile.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>
#include <kabapi.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <keditcl.h>
#include <kspell.h>

#include "knappmanager.h"
#include "kngroupmanager.h"
#include "knsavedarticle.h"
#include "kngroupselectdialog.h"
#include "knstringsplitter.h"
#include "utilities.h"
#include "knglobals.h"
#include "knode.h"
#include "kncomposer.h"



// firstEdit==true: place the cursor at the end of the article
// n==0: eMail
KNComposer::KNComposer(KNSavedArticle *a, const QCString &sig, bool firstEdit, KNNntpAccount *n)//, int textEnc)
    : KMainWindow(0), spellChecker(0), r_esult(CRsave), a_rticle(a), nntp(n),
      externalEdited(false), attChanged(false), externalEditor(0), editorTempfile(0)//, textCTE(textEnc)
{
  if(!sig.isEmpty()) s_ignature=sig.copy();
    
  delAttList=new QList<KNAttachment>;
  delAttList->setAutoDelete(true);
    
  //init view
  view=new ComposerView(this, a_rticle->isMail());
  setCentralWidget(view);
  connect(view->subject, SIGNAL(textChanged(const QString&)),
          this, SLOT(slotSubjectChanged(const QString&)));
  if(!a_rticle->isMail()) {
    connect(view->fupCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotFupCheckToggled(bool)));
  }

  connect(view->dest, SIGNAL(textChanged(const QString&)),
          this, SLOT(slotDestinationChanged(const QString&)));
  connect(view->destButton, SIGNAL(clicked()),
          this, SLOT(slotDestButtonClicked()));
  connect(view->cancelEditorButton, SIGNAL(clicked()),
          this, SLOT(slotCancelEditor()));          
    
  // file menu
  new KAction(i18n("&Send Now"),"mail_send", 0 , this, SLOT(slotSendNow()),
              actionCollection(), "send_now");
  new KAction(i18n("Send &Later"), "queue", 0, this, SLOT(slotSendLater()),
              actionCollection(), "send_later");
  new KAction(i18n("Save As &Draft"),"filesave", 0 , this, SLOT(slotSaveAsDraft()),
              actionCollection(), "save_as_draft");
  new KAction(i18n("D&elete"),"editdelete", 0 , this, SLOT(slotArtDelete()),
              actionCollection(), "art_delete");
  KStdAction::close(this, SLOT(close()),actionCollection());

  // edit menu
  KAction *undo = KStdAction::undo(view->edit, SLOT(undo()), actionCollection());
  undo->setEnabled(false);
  connect(view->edit, SIGNAL(undoAvailable(bool)), undo, SLOT(setEnabled(bool)));
  KAction *redo = KStdAction::redo(view->edit, SLOT(redo()), actionCollection());
  redo->setEnabled(false);
  connect(view->edit, SIGNAL(redoAvailable(bool)), redo, SLOT(setEnabled(bool)));
  KStdAction::cut(view->edit, SLOT(cut()), actionCollection());
  KAction *copy = KStdAction::copy(view->edit, SLOT(copy()), actionCollection());
  copy->setEnabled(false);
  connect(view->edit, SIGNAL(copyAvailable(bool)), copy, SLOT(setEnabled(bool)));
  KStdAction::paste(view->edit, SLOT(paste()), actionCollection());
  KStdAction::selectAll(view->edit, SLOT(selectAll()), actionCollection());
  KStdAction::find(this, SLOT(slotFind()), actionCollection());
  KStdAction::findNext(this, SLOT(slotFindNext()), actionCollection());
  KStdAction::replace(this, SLOT(slotReplace()), actionCollection());
  actExternalEditor = new KAction(i18n("Start &External Editor"), 0, this, SLOT(slotExternalEditor()),
                                  actionCollection(), "external_editor");
  actSpellCheck = KStdAction::spelling (this, SLOT(slotSpellcheck()), actionCollection(), "spellcheck");
    
  // attach menu
  new KAction(i18n("Append &Signature"), "signature", 0 , this, SLOT(slotAppendSig()),
                   actionCollection(), "append_signature");
  new KAction(i18n("&Insert File"), 0, this, SLOT(slotInsertFile()),
                   actionCollection(), "insert_file");
  new KAction(i18n("Attach &File"), "attach", 0, this, SLOT(slotAttachFile()),
                   actionCollection(), "attach_file");
  actRemoveAttachment = new KAction(i18n("&Remove"), 0, this, SLOT(slotRemoveAttachment()),
                                    actionCollection(), "remove_attachment");
  actAttachmentProperties = new KAction(i18n("&Properties"), 0, this, SLOT(slotAttachmentProperties()),
                                        actionCollection(), "attachment_properties");

  // settings menu
  KStdAction::showToolbar(this, SLOT(slotToggleToolBar()), actionCollection());
  KStdAction::keyBindings(this, SLOT(slotConfKeys()), actionCollection());
  KStdAction::configureToolbars(this, SLOT(slotConfToolbar()), actionCollection());
  KStdAction::preferences(knGlobals.top, SLOT(slotSettings()), actionCollection());

  createGUI("kncomposerui.rc");

  // attachment popup
  attPopup=static_cast<QPopupMenu*> (factory()->container("attachment_popup", this));
  slotAttachmentSelected(0);

  //init data 
  initData();

  setConfig();

  if (firstEdit)    // now we place the cusor at the end of the quoted text
    view->edit->setCursorPosition(view->edit->numLines()-1,0);
  else
    view->edit->setCursorPosition(0,0);
  view->edit->setFocus();

  if (firstEdit && appSig) slotAppendSig();

  if (view->viewOpen)
    restoreWindowSize("composerAtt", this, QSize(535,450));  // optimized default for 800x600
  else
    restoreWindowSize("composer", this, QSize(535,450));     // optimized default for 800x600

  view->edit->setModified(false);
    
  if (useExternalEditor) slotExternalEditor();
}


KNComposer::~KNComposer()
{
  delete delAttList;
  delete spellChecker;
  delete externalEditor;  // this also kills the editor process if it's still running
  if (editorTempfile) {
    editorTempfile->unlink();
    delete editorTempfile;
  }
  if (view->viewOpen)
    saveWindowSize("composerAtt", size());
  else
    saveWindowSize("composer", size());
}


void KNComposer::setConfig()
{
  view->edit->setWordWrap(QMultiLineEdit::FixedColumnWidth);
  view->edit->setWrapColumnOrWidth(lineLen);

  if (knGlobals.appManager->useFonts())
    view->edit->setFont(knGlobals.appManager->font(KNAppManager::composer));
  else
    view->edit->setFont(KGlobalSettings::generalFont());
}


void KNComposer::closeEvent(QCloseEvent *e)
{
  if (!textChanged() && !attChanged) {  // nothing to save, don't show nag screen
    if (a_rticle->id()==-1) r_esult=CRdel;
      else r_esult=CRcancel;
  } else {
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
  
  doneSuccess = true;
  emit composerDone(this);
  if (doneSuccess)
    e->accept();
  else
    e->ignore();
}


void KNComposer::initData()
{
  KNMimeContent *text=a_rticle->textContent();

  d_estination=a_rticle->destination().copy();
  view->dest->setText(d_estination);
  if(!a_rticle->isMail()) slotDestinationChanged(d_estination);
  
  if(text) {
    int cnt = 0;
    for(char *line=text->firstBodyLine(); line; line=text->nextBodyLine()) {
      view->edit->insertLine(line);
      cnt++;
    }
    if (appSig && (cnt==0))
      view->edit->insertLine("");
  }
    
  if(a_rticle->subject().isEmpty()) slotSubjectChanged(QString::null);
  else view->subject->setText(a_rticle->subject());
  
  if(a_rticle->isMultipart()) {
    view->showAttachmentView();
    QList<KNMimeContent> attList;
    AttachmentViewItem *item=0;
    attList.setAutoDelete(false);
    a_rticle->attachments(&attList);
    for(KNMimeContent *c=attList.first(); c; c=attList.next())
      item=new AttachmentViewItem(view->attView, new KNAttachment(c));
  } 
}


bool KNComposer::hasValidData()
{
  if (view->subject->text().isEmpty() || d_estination.isEmpty()) {
    KMessageBox::sorry(this, i18n("Please enter a subject and at least one\nnewsgroup or mail-address!"));
    return false;
  }
  bool empty = true;
  bool longLine = false;
  QString line;
  for (int i=0;i<view->edit->numLines();i++) {
    line = view->edit->textLine(i);
    if (line == "-- ")
      break;
    if (!line.isEmpty())
      empty = false;
    if (line.length()>80) {
      longLine = true;
      break;
    }
  }
  if (empty) {
    KMessageBox::sorry(this, i18n("You can't post an empty message!"));
    return false;
  }
  if (longLine)
    return  (KMessageBox::warningYesNo( this, i18n("Your article contains lines longer than 80 characters.\nDo you want to re-edit the article or send it anyway?"),
                                              QString::null, i18n("&Send"),i18n("&Edit")) == KMessageBox::Yes);
  return true;
}



bool KNComposer::textChanged()
{
  return ( view->edit->isModified() );
}



// inserts at cursor position if clear is false, replaces content otherwise
void KNComposer::insertFile(QString fileName, bool clear)
{
  unsigned int len = QFileInfo(fileName).size();
  unsigned int readLen;
  QCString temp;
  QFile file(fileName);

  if (!file.open(IO_Raw|IO_ReadOnly)) {
    if (clear)                // ok, not pretty, assuming that we load a tempfile when clear==true (external editor)
      displayTempFileError();
    else
      displayExternalFileError();
  } else {
    temp.resize(len + 2);
    readLen = file.readBlock(temp.data(), len);
    if (temp[len-1]!='\n')
      temp[len++] = '\n';
    temp[len] = '\0';
  }

  if (clear) {
    view->edit->setText(temp);
  } else {
    int editLine,editCol;
    view->edit->getCursorPosition(&editLine, &editCol);
    view->edit->insertAt(temp, editLine, editCol);
  }
}



void KNComposer::applyChanges()
{
  KNMimeContent *text=0;
  KNAttachment *a=0;

  a_rticle->setSubject(view->subject->text().local8Bit());
  a_rticle->setDestination(d_estination);
  if(!a_rticle->isMail() && view->fupCheck->isChecked() && !view->fup2->currentText().isEmpty())
    a_rticle->setHeader(KNArticleBase::HTfup2, view->fup2->currentText().local8Bit());
  else
    a_rticle->removeHeader("Followup-To");


  if(attChanged && (view->attView)) {

    QListViewItemIterator it(view->attView);
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

    if(!delAttList->isEmpty())
      for(KNAttachment *a=delAttList->first(); a; a=delAttList->next())
        if(a->isAttached())
          a->detach(a_rticle);
  }

  text=a_rticle->textContent();
  if(!text) {
      text=new KNMimeContent();
      text->initContent();
      a_rticle->addContent(text, true);
    }

  text->mimeInfo()->setCTMediaType(KNArticleBase::MTtext);
  text->mimeInfo()->setCTSubType(KNArticleBase::STplain);

  text->mimeInfo()->setCTEncoding(); //default encoding for textual contents
  text->mimeInfo()->setDecoded(true);

  text->clearBody();
  for(int i=0; i<view->edit->numLines(); i++)
    text->addBodyLine(view->edit->textLine(i).local8Bit());
}



void KNComposer::slotDestinationChanged(const QString &t)
{
  d_estination=t.local8Bit();
  
  if(!a_rticle->isMail()) {
    KNStringSplitter split;
    bool splitOk;
    QString currText = view->fup2->currentText();

    view->fup2->clear();
    split.init(d_estination, ",");
    splitOk=split.first();
    if (splitOk)
      view->fupCheck->setChecked(true);
    while(splitOk) {
      view->fup2->insertItem(QString(split.string()));
      splitOk=split.next();
    }
    if (!currText.isEmpty())
      view->fup2->lineEdit()->setText(currText);
  }
}



void KNComposer::slotDestButtonClicked()
{
  KNGroupSelectDialog *gsdlg=0;
  KabAPI *kab;
  AddressBook::Entry entry;
  KabKey key;
  QString path;
    
  if(!a_rticle->isMail()) {
    gsdlg=new KNGroupSelectDialog(this, nntp, d_estination);
  
    connect(gsdlg, SIGNAL(loadList(KNNntpAccount*)), knGlobals.gManager, SLOT(slotLoadGroupList(KNNntpAccount*)));    
    connect(knGlobals.gManager, SIGNAL(newListReady(KNGroupListData*)), gsdlg, SLOT(slotReceiveList(KNGroupListData*)));
    
    if(gsdlg->exec()) {
      d_estination=gsdlg->selectedGroups();
      view->dest->setText(d_estination);
    }
    delete gsdlg;
  }
  else {
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
        if(!d_estination.isEmpty()) d_estination+=",";
        d_estination+=entry.emails.first().local8Bit();
        view->dest->setText(d_estination);
        //kdDebug(5003) << "KabApi::getEntry() : name: " << entry.lastname << ", key: " << () << endl;
      }
    }
    delete kab;     
  }
}


void KNComposer::slotFupCheckToggled(bool b)
{
  view->fup2->setEnabled(b);
}


void KNComposer::slotSubjectChanged(const QString &t)
{
  if(!t.isEmpty()) setCaption(t);
  else setCaption(i18n("No subject"));
}


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


void KNComposer::slotFind()
{
  view->edit->search();
}


void KNComposer::slotFindNext()
{
  view->edit->repeatSearch();
}


void KNComposer::slotReplace()
{
  view->edit->replace();
}


void KNComposer::slotExternalEditor()
{
  if (externalEditor)   // in progress...
    return;

  if (externalEditorCommand.isEmpty())
    KMessageBox::sorry(this, i18n("No editor configured.\nPlease do this in the settings dialog."));

  if (editorTempfile) {       // shouldn't happen...
    editorTempfile->unlink();
    delete editorTempfile;
    editorTempfile = 0;
  }

  editorTempfile = new KTempFile();

  if (editorTempfile->status()!=0) {
    displayTempFileError();
    editorTempfile->unlink();
    delete editorTempfile;
    editorTempfile = 0;
    return;
  }

  editorTempfile->file()->writeBlock(view->edit->text().local8Bit());
  editorTempfile->close();

  if (editorTempfile->status()!=0) {
    displayTempFileError();
    editorTempfile->unlink();
    delete editorTempfile;
    editorTempfile = 0;
    return;
  }

  externalEditor = new KProcess();

  KNStringSplitter split;       // construct command line...
  split.init(externalEditorCommand.local8Bit(), " "); 
  bool filenameAdded = false;
  bool splitOk=split.first();
  while(splitOk) {
    if (split.string()=="%f") {
      (*externalEditor) << editorTempfile->name();
      filenameAdded = true;
    } else
      (*externalEditor) << split.string();
    splitOk=split.next();
  }
  if (!filenameAdded)    // no %f in the editor command
    (*externalEditor) << editorTempfile->name();

  connect(externalEditor, SIGNAL(processExited(KProcess *)),this, SLOT(slotEditorFinished(KProcess *)));
  if (!externalEditor->start()) {
    KMessageBox::error(this, i18n("Unable to start external editor.\nPlease check your configuration in the settings dialog."));
    delete externalEditor;
    externalEditor = 0;
    editorTempfile->unlink();
    delete editorTempfile;
    editorTempfile = 0;
    return;
  }

  actExternalEditor->setEnabled(false);   // block other edit action while the editor is running...
  actSpellCheck->setEnabled(false);
  view->showExternalNotification();
}


void KNComposer::slotSpellcheck()
{
  if (spellChecker)    // in progress...
    return;

  actExternalEditor->setEnabled(false);
  actSpellCheck->setEnabled(false);

  spellChecker = new KSpell(this, i18n("Spellcheck"), this, SLOT(slotSpellStarted(KSpell *)));

  connect(spellChecker, SIGNAL(death()), this, SLOT(slotSpellFinished()));
  connect(spellChecker, SIGNAL(done(const QString&)), this, SLOT(slotSpellDone(const QString&)));
  connect(spellChecker, SIGNAL(misspelling (QString, QStringList *, unsigned)),
          view->edit, SLOT(misspelling (QString, QStringList *, unsigned)));
  connect(spellChecker, SIGNAL(corrected (QString, QString, unsigned)),
          view->edit, SLOT(corrected (QString, QString, unsigned)));
}


void KNComposer::slotAppendSig()
{
  view->edit->append(s_ignature);
  view->edit->setModified(true);
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

  if(!path.isEmpty()) {
    if (!view->viewOpen) {
      saveWindowSize("composer", size());
      view->showAttachmentView();
      restoreWindowSize("composerAtt", this, QSize(535,450));  // optimized default for 800x600
    }
    (void) new AttachmentViewItem(view->attView, new KNAttachment(path));
    attChanged=true;
  }
}


void KNComposer::slotRemoveAttachment()
{
  if(!view->viewOpen) return;

  if(view->attView->currentItem()) {
    AttachmentViewItem *it=static_cast<AttachmentViewItem*>(view->attView->currentItem());
    if(it->attachment->isAttached()) {
      delAttList->append(it->attachment);
      it->attachment=0;
    }
    delete it;

    if(view->attView->childCount()==0) {
      saveWindowSize("composerAtt", size());
      view->hideAttachmentView();
      restoreWindowSize("composer", this, QSize(535,450));     // optimized default for 800x600
    }

    attChanged=true;
  }
}


void KNComposer::slotAttachmentProperties()
{
  if(!view->viewOpen) return;

  if(view->attView->currentItem()) {
    AttachmentViewItem *it=static_cast<AttachmentViewItem*>(view->attView->currentItem());
    AttachmentPropertiesDlg *d=new AttachmentPropertiesDlg(this, it->attachment);
    if(d->exec()) {
      d->apply();
      it->setText(1, it->attachment->contentMimeType());
      it->setText(3, it->attachment->contentDescription());
      it->setText(4, it->attachment->contentEncoding());
    }
    delete d;
    attChanged=true;
  }

}


void KNComposer::slotAttachmentPopup(QListViewItem *it, const QPoint &p, int)
{
  if(it)
    attPopup->popup(p);
}


void KNComposer::slotAttachmentSelected(QListViewItem *it)
{
  if (view->attWidget) {
    view->attRemoveButton->setEnabled((it!=0));
    view->attEditButton->setEnabled((it!=0));
  }
}


void KNComposer::slotAttachmentEdit(QListViewItem *it)
{
  slotAttachmentProperties();
}


void KNComposer::slotAttachmentRemove(QListViewItem *it)
{
  slotRemoveAttachment();
}

    
void KNComposer::slotToggleToolBar()
{
  if(toolBar("mainToolBar")->isVisible())
    toolBar("mainToolBar")->hide();
  else
    toolBar("mainToolBar")->show();
}


void KNComposer::slotConfKeys()
{
  KKeyDialog::configureKeys(actionCollection(), xmlFile(), true, this);
}
  
    
void KNComposer::slotConfToolbar()
{
  KEditToolbar *dlg = new KEditToolbar(guiFactory(),this);
  if (dlg->exec()) {
    createGUI("kncomposerui.rc");
    attPopup=static_cast<QPopupMenu*> (factory()->container("attachment_popup", this));
  }
  delete dlg;
}


//==============================================================================
// spellchecking code copied form kedit (Bernd Johannes Wuebben)
//==============================================================================


void KNComposer::slotSpellStarted( KSpell *)
{
  view->edit->spellcheck_start();
  spellChecker->setProgressResolution(2);
  spellChecker->check(view->edit->text());
}



void KNComposer::slotSpellDone(const QString &newtext)
{
  actExternalEditor->setEnabled(true);
  actSpellCheck->setEnabled(true);
  view->edit->spellcheck_stop();
  if (spellChecker->dlgResult() == 0)
    view->edit->setText(newtext);
  spellChecker->cleanUp();
  delete spellChecker;
  spellChecker = 0;
}


void KNComposer::slotSpellFinished()
{
  actExternalEditor->setEnabled(true);
  actSpellCheck->setEnabled(true);
  KSpell::spellStatus status = spellChecker->status();
  delete spellChecker;
  spellChecker = 0;
  if (status == KSpell::Error) {
    KMessageBox::error(this, i18n("ISpell could not be started.\n"
    "Please make sure you have ISpell properly configured and in your PATH."));
  } else if (status == KSpell::Crashed) {
    view->edit->spellcheck_stop();
    KMessageBox::error(this, i18n("ISpell seems to have crashed."));
  }
}


void KNComposer::slotEditorFinished(KProcess *)
{
  if (externalEditor->normalExit()) {
    insertFile(editorTempfile->name(),true);
    externalEdited = true;
  }

  slotCancelEditor();   // cleanup...
}


void KNComposer::slotCancelEditor()
{
  delete externalEditor;  // this also kills the editor process if it's still running
  externalEditor = 0;
  editorTempfile->unlink();
  delete editorTempfile;
  editorTempfile = 0;

  actExternalEditor->setEnabled(true);
  actSpellCheck->setEnabled(true);
  view->hideExternalNotification();
}


//=====================================================================================
// handle Tabs... (expanding them in textLine(), etc.)

KNComposer::Editor::Editor(QWidget *parent=0, char *name=0)
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
  QString temp = KEdit::textLine(line);
  QString replacement;
  int tabPos;

  while ((tabPos = temp.find('\t'))!=-1) {
    replacement.fill(QChar(' '),8-(tabPos%8));
    temp.replace(tabPos,1,replacement);
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


KNComposer::ComposerView::ComposerView(QWidget *parent, bool mail)
  : QSplitter(QSplitter::Vertical, parent, 0), attWidget(0), attView(0), viewOpen(false)
{
  QLabel *l1, *l2;
  int frameLines=2;
  QWidget *editW=new QWidget(this);
  QFrame *fr1=new QFrame(editW);
  fr1->setFrameStyle(QFrame::Box | QFrame::Sunken);
  subject=new QLineEdit(fr1);
  l1=new QLabel(subject,i18n("S&ubject:"), fr1);
  dest=new QLineEdit(fr1);
  if (mail)
    l2=new QLabel(dest, i18n("&To:"), fr1);
  else
    l2=new QLabel(dest, i18n("&Groups:"), fr1);
  destButton=new QPushButton(i18n("&Browse..."), fr1);
  if(!mail) {
    fupCheck=new QCheckBox(i18n("Followup-&To:"), fr1);
    fupCheck->setMinimumSize(fupCheck->sizeHint());
    fup2=new QComboBox(true, fr1);
    fup2->setMinimumSize(fup2->sizeHint());
    fup2->setEnabled(false);
    frameLines=3;
  }
  
  edit=new Editor(editW);
  edit->setMinimumHeight(50);
  
  QVBoxLayout *l = new QVBoxLayout(edit);
  l->addStretch(1);
  notification=new QGroupBox(2,Qt::Horizontal,edit);
  new QLabel(i18n("You are currently editing the article body\nin an external editor. To continue you have\nto close the external editor."),notification);
  cancelEditorButton=new QPushButton(i18n("&Kill external editor"), notification);
  notification->setFrameStyle(QFrame::Panel | QFrame::Raised);
  notification->setLineWidth(2);
  notification->hide();
  l->addWidget(notification,0,Qt::AlignHCenter);
  l->addStretch(1);
    
  QGridLayout *frameL1=new QGridLayout(fr1, frameLines,3, 10,10);
  frameL1->addWidget(l1, 0,0);
  frameL1->addMultiCellWidget(subject, 0,0,1,2);
  frameL1->addWidget(l2, 1,0);
  frameL1->addWidget(dest, 1,1);
  frameL1->addWidget(destButton, 1,2);
  if(!mail) {
    frameL1->addWidget(fupCheck, 2,0);
    frameL1->addMultiCellWidget(fup2, 2,2,1,2);
  }
  frameL1->setColStretch(1,1);
    
  QVBoxLayout *topL=new QVBoxLayout(editW, 4,4);
  topL->addWidget(fr1);
  topL->addWidget(edit, 1);
}



KNComposer::ComposerView::~ComposerView()
{
  if (viewOpen) {
    KConfig *conf=KGlobal::config();
    conf->setGroup("POSTNEWS");

    conf->writeEntry("Att_Splitter",sizes());   // save splitter pos

    QValueList<int> lst;                        // save header sizes
    QHeader *h=attView->header();
    for (int i=0; i<5; i++)
      lst << h->sectionSize(i);
    conf->writeEntry("Att_Headers",lst);

    conf->sync();
  }
}


void KNComposer::ComposerView::showAttachmentView()
{
  if(!attWidget) {
    attWidget=new QWidget(this);
    QGridLayout *topL=new QGridLayout(attWidget, 3, 2, 4, 4);

    attView=new AttachmentView(attWidget);
    topL->addMultiCellWidget(attView,0,2,0,0);
    connect(attView, SIGNAL(currentChanged(QListViewItem*)),
            parent(), SLOT(slotAttachmentSelected(QListViewItem*)));
    connect(attView, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
            parent(), SLOT(slotAttachmentPopup(QListViewItem*, const QPoint&, int)));
    connect(attView, SIGNAL(delPressed(QListViewItem*)),
            parent(), SLOT(slotAttachmentRemove(QListViewItem*)));
    connect(attView, SIGNAL(doubleClicked(QListViewItem*)),
            parent(), SLOT(slotAttachmentEdit(QListViewItem*)));
    connect(attView, SIGNAL(returnPressed(QListViewItem*)),
            parent(), SLOT(slotAttachmentEdit(QListViewItem*)));

    QPushButton *btn = new QPushButton(i18n("A&dd"),attWidget);
    connect(btn, SIGNAL(clicked()), parent(), SLOT(slotAttachFile()));
    topL->addWidget(btn, 0,1);

    attRemoveButton = new QPushButton(i18n("&Remove"),attWidget);
    attRemoveButton->setEnabled(false);
    connect(attRemoveButton, SIGNAL(clicked()), parent(), SLOT(slotRemoveAttachment()));
    topL->addWidget(attRemoveButton, 1,1);

    attEditButton = new QPushButton(i18n("&Properties"),attWidget);
    attEditButton->setEnabled(false);
    connect(attEditButton, SIGNAL(clicked()), parent(), SLOT(slotAttachmentProperties()));
    topL->addWidget(attEditButton, 2,1, Qt::AlignTop);

    topL->setRowStretch(2,1);
    topL->setColStretch(0,1);
  }
  
  if (!viewOpen) {
    viewOpen = true;
    attWidget->show();

    KConfig *conf=KGlobal::config();
    conf->setGroup("POSTNEWS");

    QValueList<int> lst = conf->readIntListEntry("Att_Splitter");
    if (lst.count()!=2)
      lst << 267 << 112;
    setSizes(lst);

    lst = conf->readIntListEntry("Att_Headers");
    if (lst.count()==5) {
      QValueList<int>::Iterator it = lst.begin();

      QHeader *h=attView->header();
      for (int i=0; i<5; i++) {
         h->resizeSection(i,(*it));
         ++it;
      }
    }
  }
}



void KNComposer::ComposerView::hideAttachmentView()
{
  if (viewOpen) {
    KConfig *conf=KGlobal::config();
    conf->setGroup("POSTNEWS");

    conf->writeEntry("Att_Splitter",sizes());   // save splitter pos

    QValueList<int> lst;                        // save header sizes
    QHeader *h=attView->header();
    for (int i=0; i<5; i++)
      lst << h->sectionSize(i);
    conf->writeEntry("Att_Headers",lst);

    conf->sync();

    attWidget->hide();
    viewOpen = false;
  }
}



void KNComposer::ComposerView::showExternalNotification()
{
  edit->setReadOnly(true);
  notification->show();
}



void KNComposer::ComposerView::hideExternalNotification()
{
  edit->setReadOnly(false);
  notification->hide();
}



//===============================================================



bool KNComposer::appSig;
bool KNComposer::useExternalEditor;
int KNComposer::lineLen;
QString KNComposer::externalEditorCommand;

void KNComposer::readConfig()
{
  KConfig *conf=KGlobal::config();
  conf->setGroup("POSTNEWS");
  lineLen=conf->readNumEntry("maxLength", 72);
  appSig=conf->readBoolEntry("appSig", true);
  useExternalEditor=conf->readBoolEntry("useExternalEditor",false);
  externalEditorCommand=conf->readEntry("externalEditor","kwrite %f");
}


//=====================================================================================


KNComposer::AttachmentView::AttachmentView(QWidget *parent, char *name)
 : QListView(parent, name)
{
  setFrameStyle( QFrame::WinPanel | QFrame::Sunken );  // match the QMultiLineEdit style
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



void KNComposer::AttachmentView::keyPressEvent( QKeyEvent * e )
{
  if ( !e )
    return; // subclass bug

  if ((e->key() == Key_Delete)&&(currentItem()))
    emit( delPressed(currentItem()) );
  else
    QListView::keyPressEvent(e);
}



//===============================================================



KNComposer::AttachmentViewItem::AttachmentViewItem(QListView *v, KNAttachment *a) :
  QListViewItem(v), attachment(a)
{
  setText(0, a->contentName());
  setText(1, a->contentMimeType());
  setText(2, a->contentSize());
  setText(3, a->contentDescription());
  setText(4, a->contentEncoding());
}



KNComposer::AttachmentViewItem::~AttachmentViewItem()
{
  delete attachment;
}


//===================================================================


KNComposer::AttachmentPropertiesDlg::AttachmentPropertiesDlg(QWidget *p, KNAttachment *a) :
  KDialogBase(p, 0, true, i18n("Attachment Properties"), Help|Ok|Cancel, Ok), attachment(a),
  nonTextAsText(false)
{
  QWidget *page = new QWidget(this);
  setMainWidget(page);
  QVBoxLayout *topL = new QVBoxLayout(page);

  // file info ========================================================
  QGroupBox *fileGB=new QGroupBox(i18n("File"), page);
  QGridLayout *fileL = new QGridLayout(fileGB,2,2,20,10);

  fileL->addWidget(new QLabel(i18n("Name:"),fileGB),0,0);
  fileL->addWidget(new QLabel(QString("<b>%1</b>").arg(a->contentName()),fileGB),0,1,Qt::AlignLeft);
  fileL->addWidget(new QLabel(i18n("Size:"),fileGB),1,0);
  fileL->addWidget(new QLabel(a->contentSize(),fileGB),1,1,Qt::AlignLeft);

  fileL->setColStretch(1,1);
  topL->addWidget(fileGB);

  // mime info =======================================================
  QGroupBox *mimeGB=new QGroupBox(i18n("Mime"), page);
  QGridLayout *mimeL=new QGridLayout(mimeGB, 3,2, 20,10);

  mimeL->addWidget(new QLabel(i18n("Mime-Type:"), mimeGB), 0,0);
  mimeType=new QLineEdit(mimeGB);
  mimeL->addWidget(mimeType, 0,1);

  mimeL->addWidget(new QLabel(i18n("Description:"), mimeGB), 1,0);
  description=new QLineEdit(mimeGB);
  mimeL->addWidget(description, 1,1);

  mimeL->addWidget(new QLabel(i18n("Encoding:"), mimeGB), 2,0);
  encoding=new QComboBox(false, mimeGB);
  encoding->insertItem("7Bit");
  encoding->insertItem("8Bit");
  encoding->insertItem("quoted-printable");
  encoding->insertItem("base64");
  if(a->isFixedBase64()) {
    encoding->setCurrentItem(3);
    encoding->setEnabled(false);
  } else
    encoding->setCurrentItem(a->cte());
  mimeL->addWidget(encoding, 2,1);

  mimeL->setColStretch(1,1);
  topL->addWidget(mimeGB);

  setFixedHeight(sizeHint().height());

  mimeType->setText(a->contentMimeType());
  description->setText(a->contentDescription());

  connect(mimeType, SIGNAL(textChanged(const QString&)),
    this, SLOT(slotMimeTypeTextChanged(const QString&)));

  restoreWindowSize("attProperties", this, QSize(300,250));
}



KNComposer::AttachmentPropertiesDlg::~AttachmentPropertiesDlg()
{
  saveWindowSize("attProperties", this->size());
}



void KNComposer::AttachmentPropertiesDlg::apply()
{
  attachment->setContentDescription(description->text());
  attachment->setContentMimeType(mimeType->text());
  attachment->setCte(encoding->currentItem());
}



void KNComposer::AttachmentPropertiesDlg::accept()
{
  if(mimeType->text().find('/')==-1) {
    KMessageBox::sorry(this, i18n("You have set an invalid mime-type.\nPlease change it."));
    return;
  }
  else if(nonTextAsText && mimeType->text().find("text/", 0, false)!=-1 &&
       KMessageBox::warningYesNo(this,
       i18n("You have changed the mime-type of this non-textual attachment\nto text. This might cause an error while loading or encoding the file.\nProceed?")
       ) == KMessageBox::No) return;


  KDialogBase::accept();
}



void KNComposer::AttachmentPropertiesDlg::slotMimeTypeTextChanged(const QString &text)
{
  if(text.left(5)!="text/") {
    nonTextAsText=attachment->isFixedBase64();
    encoding->setCurrentItem(3);
    encoding->setEnabled(false);
  }
  else {
    encoding->setCurrentItem(attachment->cte());
    encoding->setEnabled(true);
  }
}


//--------------------------------

#include "kncomposer.moc"
