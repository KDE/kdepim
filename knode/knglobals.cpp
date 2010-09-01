/*
    knglobals.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2004 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "knglobals.h"


#include <kconfig.h>
#include <kstaticdeleter.h>

#include "knconfigmanager.h"
#include "knnetaccess.h"
#include "knaccountmanager.h"
#include "kngroupmanager.h"
#include "knarticlemanager.h"
#include "knfiltermanager.h"
#include "knfoldermanager.h"
#include "knscoring.h"
#include "knmemorymanager.h"
#include "knmainwidget.h"
#include "knwidgets.h"

KNGlobals::KNGlobals() :
  mNetAccess( 0 ),
  mCfgManager( 0 ),
  mAccManager( 0 ),
  mGrpManager( 0 ),
  mArtManager( 0 ),
  mFilManager( 0 ),
  mFolManager( 0 ),
  mMemManager( 0 )
{
}

KNGlobals::~KNGlobals( )
{
#if 0
// hmm.. something in here is causing an 'impossible' crash. let's ignore the cleanup then.
  delete mNetAccess;
  delete mCfgManager;
  delete mAccManager;
  delete mGrpManager;
  delete mArtManager;
  delete mFilManager;
  delete mFolManager;
  delete mMemManager;
#endif
}

KConfig* KNGlobals::config()
{
  if (!c_onfig) {
      c_onfig = KSharedConfig::openConfig( "knoderc" );
  }
  return c_onfig;
}

KNConfigManager* KNGlobals::configManager()
{
  if (!mCfgManager)
    mCfgManager = new KNConfigManager();
  return mCfgManager;
}

KNNetAccess* KNGlobals::netAccess()
{
  if(!mNetAccess)
    mNetAccess = new KNNetAccess();
  return mNetAccess;
}

KNAccountManager* KNGlobals::accountManager()
{
  if(!mAccManager)
    mAccManager = new KNAccountManager(groupManager());
  return mAccManager;
}

KNGroupManager* KNGlobals::groupManager()
{
  if(!mGrpManager)
    mGrpManager = new KNGroupManager();
  return mGrpManager;
}

KNArticleManager* KNGlobals::articleManager()
{
  if(!mArtManager)
    mArtManager = new KNArticleManager();
  return mArtManager;
}

KNFilterManager* KNGlobals::filterManager()
{
  if (!mFilManager)
    mFilManager = new KNFilterManager();
  return mFilManager;
}

KNFolderManager* KNGlobals::folderManager()
{
  if(!mFolManager)
    mFolManager = new KNFolderManager(articleManager());
  return mFolManager;
}

KNScoringManager* KNGlobals::mScoreManager = 0;

KNScoringManager* KNGlobals::scoringManager()
{
  static  KStaticDeleter<KNScoringManager> sd;
  if (!mScoreManager)
    sd.setObject(mScoreManager, new KNScoringManager());
  return mScoreManager;
}

KNMemoryManager* KNGlobals::memoryManager()
{
  if(!mMemManager)
    mMemManager = new KNMemoryManager();
  return mMemManager;
}


void KNGlobals::setStatusMsg(const TQString &text, int id)
{
  if(top)
    top->setStatusMsg(text, id);
}
