// -*- c-basic-offset: 2 -*-
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

QValueList<KNArticleWindow*> KNArticleWindow::mInstances;


bool KNArticleWindow::closeAllWindowsForCollection( KNArticleCollection *col, bool force )
{
  QValueList<KNArticleWindow*> list = mInstances;
  for ( QValueList<KNArticleWindow*>::Iterator it = list.begin(); it != list.end(); ++it )
    if ( (*it)->artW->article() && (*it)->artW->article()->collection() == col ) {
      if ( force )
        (*it)->close();
      else
        return false;
    }
  return true;
}


bool KNArticleWindow::closeAllWindowsForArticle( KNArticle *art, bool force )
{
  QValueList<KNArticleWindow*> list = mInstances;
  for ( QValueList<KNArticleWindow*>::Iterator it = list.begin(); it != list.end(); ++it )
    if ( (*it)->artW->article() && (*it)->artW->article() == art ) {
      if ( force )
        (*it)->close();
      else
        return false;
    }
  return true;
}


bool KNArticleWindow::raiseWindowForArticle( KNArticle *art )
{
  for ( QValueList<KNArticleWindow*>::Iterator it = mInstances.begin(); it != mInstances.end(); ++it )
    if ( (*it)->artW->article() && (*it)->artW->article() == art ) {
      KWin::activateWindow( (*it)->winId() );
      return true;
    }
  return false;
}


bool KNArticleWindow::raiseWindowForArticle(const QCString &mid)
{
  for ( QValueList<KNArticleWindow*>::Iterator it = mInstances.begin(); it != mInstances.end(); ++it )
    if ( (*it)->artW->article() && (*it)->artW->article()->messageID()->as7BitString( false ) == mid ) {
      KWin::activateWindow( (*it)->winId() );
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

  mInstances.append( this );

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
  mInstances.remove( this );
  KConfig *conf = knGlobals.config();
  conf->setGroup("articleWindow_options");
  saveMainWindowSettings(conf);
}

//--------------------------------

#include "knarticlewindow.moc"
