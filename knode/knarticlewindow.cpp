/*
    knarticlewindow.cpp

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

#include <kwin.h>

#include <kstdaction.h>
#include <klocale.h>
#include <kedittoolbar.h>
#include <kkeydialog.h>
#include <kconfig.h>

#include "kngroup.h"
#include "knmime.h"
#include "knarticlewidget.h"
#include "utilities.h"
#include "knglobals.h"
#include "knode.h"
#include "knarticlewindow.h"

QList<KNArticleWindow> KNArticleWindow::instances;


void KNArticleWindow::closeAllWindowsForCollection(KNArticleCollection *col)
{
  QList<KNArticleWindow> list=instances;
  for(KNArticleWindow *i=list.first(); i; i=list.next())
    if(i->artW->article()->collection()==col)
      i->close();
}


void KNArticleWindow::closeAllWindowsForArticle(KNArticle *art)
{
  QList<KNArticleWindow> list=instances;
  for(KNArticleWindow *i=list.first(); i; i=list.next())
    if(i->artW->article()==art)
      i->close();
}


bool KNArticleWindow::raiseWindowForArticle(KNArticle *art)
{
  bool ret=false;
  for(KNArticleWindow *i=instances.first(); i; i=instances.next())
    if(i->artW->article()==art) {
      KWin::setActiveWindow(i->winId());
      ret = true;
      break;
    }
  return ret;
}


bool KNArticleWindow::raiseWindowForArticle(const QCString &mid)
{
  bool ret=false;
  for(KNArticleWindow *i=instances.first(); i; i=instances.next())
    if(i->artW->article()->messageID()->as7BitString(false)==mid) {
      KWin::setActiveWindow(i->winId());
      ret = true;
      break;
    }
  return ret;
}


//==================================================================================

KNArticleWindow::KNArticleWindow(KNArticle *art)
  : KMainWindow(0, "articleWindow")
{
  instances.append(this);

  if(art)
    setCaption(art->subject()->asUnicodeString());

  artW=new KNArticleWidget(actionCollection(),this);
  artW->setArticle(art);
  setCentralWidget(artW);

  // file menu
  KStdAction::close(this, SLOT(slotFileClose()),actionCollection());

  // settings menu
  a_ctShowToolbar = KStdAction::showToolbar(this, SLOT(slotToggleToolBar()), actionCollection());
  KStdAction::keyBindings(this, SLOT(slotConfKeys()), actionCollection());
  KStdAction::configureToolbars(this, SLOT(slotConfToolbar()), actionCollection());
  KStdAction::preferences(knGlobals.top, SLOT(slotSettings()), actionCollection());

  a_ccel=new KAccel(this);
  artW->setCharsetKeyboardAction()->plugAccel(a_ccel);

  createGUI("knreaderui.rc");
  QPopupMenu *pop = static_cast<QPopupMenu *>(factory()->container("body_popup", this));
  if (!pop) pop = new QPopupMenu(this);
  artW->setBodyPopup(pop);

  KConfig *conf = KGlobal::config();
  conf->setGroup("articleWindow_options");
  resize(500,400);    // default optimized for 800x600
  applyMainWindowSettings(conf);
  a_ctShowToolbar->setChecked(!toolBar()->isHidden());
}


KNArticleWindow::~KNArticleWindow()
{
  instances.removeRef(this);
  KConfig *conf = KGlobal::config();
  conf->setGroup("articleWindow_options");
  saveMainWindowSettings(conf);
}


void KNArticleWindow::slotFileClose()
{
  close();
}


void KNArticleWindow::slotToggleToolBar()
{
  if(toolBar()->isVisible())
    toolBar()->hide();
  else
    toolBar()->show();
}


void KNArticleWindow::slotConfKeys()
{
  KActionCollection coll(*actionCollection());

  // hack, remove actions which cant have a shortcut
  coll.take(artW->setCharsetAction());

  KKeyDialog::configureKeys(&coll, xmlFile(), true, this);
}

    
void KNArticleWindow::slotConfToolbar()
{
  KEditToolbar *dlg = new KEditToolbar(guiFactory(),this);
  if (dlg->exec()) {
    createGUI("knreaderui.rc");
    QPopupMenu *pop = static_cast<QPopupMenu *>(factory()->container("body_popup", this));
    if (!pop) pop = new QPopupMenu(this);
    artW->setBodyPopup(pop);
  }
  delete dlg;
}


//--------------------------------

#include "knarticlewindow.moc"
