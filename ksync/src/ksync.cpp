/*
    This file is part of ksync.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qdir.h>
#include <qprinter.h>
#include <qpainter.h>

#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstdaction.h>

#include "ksync.h"
#include "ksync.moc"
#include "ksyncview.h"
#include <kstatusbar.h>

#define ID_STATUS_MSG 1

KSync::KSync(QWidget* , const char* name):KMainWindow(0, name)
{
  config=kapp->config();

  initStatusBar();
  initActions();
  initView();
	
  readOptions();

  // disable actions at startup
  fileSave->setEnabled(false);
  fileSaveAs->setEnabled(false);
  filePrint->setEnabled(false);
  editCut->setEnabled(false);
  editCopy->setEnabled(false);
  editPaste->setEnabled(false);
  setAutoSaveSettings();
}

void KSync::initActions()
{
  fileNewWindow = new KAction(i18n("New &Window"), 0, 0, this, SLOT(slotFileNewWindow()), actionCollection(),"file_new_window");
  fileNew = KStdAction::openNew(this, SLOT(slotFileNew()), actionCollection());
  fileOpen = KStdAction::open(this, SLOT(slotFileOpen()), actionCollection());
  fileOpenRecent = KStdAction::openRecent(this, SLOT(slotFileOpenRecent(const KURL&)), actionCollection());
  fileSave = KStdAction::save(this, SLOT(slotFileSave()), actionCollection());
  fileSaveAs = KStdAction::saveAs(this, SLOT(slotFileSaveAs()), actionCollection());
  fileClose = KStdAction::close(this, SLOT(slotFileClose()), actionCollection());
  filePrint = KStdAction::print(this, SLOT(slotFilePrint()), actionCollection());
  fileQuit = KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection());
  editCut = KStdAction::cut(this, SLOT(slotEditCut()), actionCollection());
  editCopy = KStdAction::copy(this, SLOT(slotEditCopy()), actionCollection());
  editPaste = KStdAction::paste(this, SLOT(slotEditPaste()), actionCollection());
  createStandardStatusBarAction();
  setStandardToolBarMenuEnabled(true);
    
  fileNewWindow->setToolTip(i18n("Opens a new application window"));
  fileNew->setToolTip(i18n("Creates a new document"));
  fileOpen->setToolTip(i18n("Opens an existing document"));
  fileOpenRecent->setToolTip(i18n("Opens a recently used file"));
  fileSave->setToolTip(i18n("Saves the actual document"));
  fileSaveAs->setToolTip(i18n("Saves the actual document as..."));
  fileClose->setToolTip(i18n("Closes the actual document"));
  filePrint ->setToolTip(i18n("Prints out the actual document"));
  fileQuit->setToolTip(i18n("Quits the application"));
  editCut->setToolTip(i18n("Cuts the selected section and puts it to the clipboard"));
  editCopy->setToolTip(i18n("Copies the selected section to the clipboard"));
  editPaste->setToolTip(i18n("Pastes the clipboard contents to actual position"));

  // use the absolute path to your ksyncui.rc file for testing purpose in createGUI();
  createGUI();

}


void KSync::initStatusBar()
{
  statusBar()->insertItem(i18n("Ready."), ID_STATUS_MSG);
}

void KSync::initView()
{
  mView = new KSyncView(this);
  setCentralWidget(mView);	
//  setCaption(doc->URL().fileName(),false);
}

void KSync::openDocumentFile(const KURL& url)
{
  slotStatusMsg(i18n("Opening file..."));

//  doc->openDocument( url);
  fileOpenRecent->addURL( url );
  slotStatusMsg(i18n("Ready."));
}


void KSync::saveOptions()
{	
  config->setGroup("General Options");
  fileOpenRecent->saveEntries(config,"Recent Files");

  mView->writeConfig(config);
}


void KSync::readOptions()
{
  config->setGroup("General Options");

  // initialize the recent file list
  fileOpenRecent->loadEntries(config,"Recent Files");
  mView->readConfig(config);
}

void KSync::saveProperties(KConfig *)
{
#if 0
  if(doc->URL().fileName()!=i18n("Untitled") && !doc->isModified())
  {
    // saving to tempfile not necessary

  }
  else
  {
    KURL url=doc->URL();	
    _cfg->writePathEntry("filename", url.url());
    _cfg->writeEntry("modified", doc->isModified());
    QString tempname = kapp->tempSaveName(url.url());
    QString tempurl= KURL::encode_string(tempname);
    KURL _url(tempurl);
    doc->saveDocument(_url);
  }
#endif
}


void KSync::readProperties(KConfig *)
{
#if 0
  QString filename = _cfg->readPathEntry("filename");
  KURL url(filename);
  bool modified = _cfg->readBoolEntry("modified", false);
  if(modified)
  {
    bool canRecover;
    QString tempname = kapp->checkRecoverFile(filename, canRecover);
    KURL _url(tempname);
  	
    if(canRecover)
    {
      doc->openDocument(_url);
      doc->setModified();
      setCaption(_url.fileName(),true);
      QFile::remove(tempname);
    }
  }
  else
  {
    if(!filename.isEmpty())
    {
      doc->openDocument(url);
      setCaption(url.fileName(),false);
    }
  }
#endif
}

bool KSync::queryClose()
{
//  return doc->saveModified();
  return true;
}

bool KSync::queryExit()
{
  saveOptions();
  return true;
}

void KSync::slotFileNewWindow()
{
  slotStatusMsg(i18n("Opening a new application window..."));
	
  KSync *new_window= new KSync();
  new_window->show();

  slotStatusMsg(i18n("Ready."));
}

void KSync::slotFileNew()
{
  slotStatusMsg(i18n("Creating new document..."));

#if 0
  if(!doc->saveModified())
  {
     // here saving wasn't successful

  }
  else
  {	
    doc->newDocument();		
    setCaption(doc->URL().fileName(), false);
  }
#endif

  slotStatusMsg(i18n("Ready."));
}

void KSync::slotFileOpen()
{
  slotStatusMsg(i18n("Opening file..."));

#if 0	
  if(!doc->saveModified())
  {
     // here saving wasn't successful

  }
  else
  {	
    KURL url=KFileDialog::getOpenURL(QString::null,
        i18n("*|All Files"), this, i18n("Open File"));
    if(!url.isEmpty())
    {
      doc->openDocument(url);
      setCaption(url.fileName(), false);
      fileOpenRecent->addURL( url );
    }
  }
#endif

  slotStatusMsg(i18n("Ready."));
}

void KSync::slotFileOpenRecent(const KURL&)
{
  slotStatusMsg(i18n("Opening file..."));

#if 0	
  if(!doc->saveModified())
  {
     // here saving wasn't successful
  }
  else
  {
    doc->openDocument(url);
    setCaption(url.fileName(), false);
  }
#endif

  slotStatusMsg(i18n("Ready."));
}

void KSync::slotFileSave()
{
  slotStatusMsg(i18n("Saving file..."));
	
//  doc->saveDocument(doc->URL());

  slotStatusMsg(i18n("Ready."));
}

void KSync::slotFileSaveAs()
{
  slotStatusMsg(i18n("Saving file with a new filename..."));

  KURL url=KFileDialog::getSaveURL(QDir::currentDirPath(),
        i18n("*|All Files"), this, i18n("Save As"));
  if(!url.isEmpty())
  {
//    doc->saveDocument(url);
    fileOpenRecent->addURL(url);
//    setCaption(url.fileName(),doc->isModified());
  }

  slotStatusMsg(i18n("Ready."));
}

void KSync::slotFileClose()
{
  slotStatusMsg(i18n("Closing file..."));
	
  close();

  slotStatusMsg(i18n("Ready."));
}

void KSync::slotFilePrint()
{
  slotStatusMsg(i18n("Printing..."));

  QPrinter printer;
  if (printer.setup(this))
  {
    mView->print(&printer);
  }

  slotStatusMsg(i18n("Ready."));
}

void KSync::slotFileQuit()
{
  slotStatusMsg(i18n("Exiting..."));
  saveOptions();
  // close the first window, the list makes the next one the first again.
  // This ensures that queryClose() is called on each window to ask for closing
  KMainWindow* w;
  if(memberList)
  {
    for(w=memberList->first(); w!=0; w=memberList->first())
    {
      // only close the window if the closeEvent is accepted. If the user presses Cancel on the saveModified() dialog,
      // the window and the application stay open.
      if(!w->close())
	break;
    }
  }	
  slotStatusMsg(i18n("Ready."));
}

void KSync::slotEditCut()
{
  slotStatusMsg(i18n("Cutting selection..."));

  slotStatusMsg(i18n("Ready."));
}

void KSync::slotEditCopy()
{
  slotStatusMsg(i18n("Copying selection to clipboard..."));

  slotStatusMsg(i18n("Ready."));
}

void KSync::slotEditPaste()
{
  slotStatusMsg(i18n("Inserting clipboard contents..."));

  slotStatusMsg(i18n("Ready."));
}

void KSync::slotStatusMsg(const QString &text)
{
  statusBar()->clear();
  statusBar()->changeItem(text, ID_STATUS_MSG);
}

