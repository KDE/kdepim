// -*- c-basic-offset: 2 -*-
/*
    knarticlewindow.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
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
#include <kconfig.h>
#include <kaccel.h>
#include <kaction.h>

#include "knarticle.h"
#include "articlewidget.h"
#include "utilities.h"
#include "knglobals.h"
#include "knmainwidget.h"
#include "knarticlewindow.h"

using namespace KNode;

QPtrList<KNArticleWindow> KNArticleWindow::instances;


bool KNArticleWindow::closeAllWindowsForCollection(KNArticleCollection *col, bool force)
{
  QPtrList<KNArticleWindow> list=instances;
  for(KNArticleWindow *i=list.first(); i; i=list.next())
    if(i->artW->article() && i->artW->article()->collection()==col) {
      if (force)
        i->close();
      else
        return false;
    }
  return true;
}


bool KNArticleWindow::closeAllWindowsForArticle(KNArticle *art, bool force)
{
  QPtrList<KNArticleWindow> list=instances;
  for(KNArticleWindow *i=list.first(); i; i=list.next())
    if(i->artW->article() && i->artW->article() == art) {
      if (force)
        i->close();
      else
        return false;
    }
  return true;
}


bool KNArticleWindow::raiseWindowForArticle(KNArticle *art)
{
  for(KNArticleWindow *i=instances.first(); i; i=instances.next())
    if(i->artW->article() && i->artW->article() ==art) {
      KWin::activateWindow(i->winId());
      return true;
    }
  return false;
}


bool KNArticleWindow::raiseWindowForArticle(const QCString &mid)
{
  for(KNArticleWindow *i=instances.first(); i; i=instances.next())
    if(i->artW->article() && i->artW->article()->messageID()->as7BitString(false)==mid) {
      KWin::activateWindow(i->winId());
      return true;
    }

  return false;
}


//==================================================================================

KNArticleWindow::KNArticleWindow(KNArticle *art)
  : KMainWindow(0, "articleWindow")
{
  if(knGlobals.instance)
    setInstance(knGlobals.instance);

  if(art)
    setCaption(art->subject()->asUnicodeString());

  artW = new ArticleWidget( this, this, actionCollection() );
  artW->setArticle(art);
  setCentralWidget(artW);

  instances.append(this);

  // file menu
  KStdAction::close( this, SLOT(close()), actionCollection() );

  // settings menu
  KStdAction::preferences(knGlobals.top, SLOT(slotSettings()), actionCollection());

  KAccel *accel = new KAccel( this );
  artW->setCharsetKeyboardAction()->plugAccel( accel );

  setupGUI( ToolBar|Keys|Create, "knreaderui.rc");

  KConfig *conf = knGlobals.config();
  conf->setGroup("articleWindow_options");
  resize(500,400);    // default optimized for 800x600
  applyMainWindowSettings(conf);
}


KNArticleWindow::~KNArticleWindow()
{
  instances.removeRef(this);
  KConfig *conf = knGlobals.config();
  conf->setGroup("articleWindow_options");
  saveMainWindowSettings(conf);
}

//--------------------------------

#include "knarticlewindow.moc"
