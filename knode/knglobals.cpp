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

#include "knglobals.h"

#include <kapplication.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kstaticdeleter.h>

#include "knconfigmanager.h"
#include "knaccountmanager.h"
#include "kngroupmanager.h"
#include "knarticlemanager.h"
#include "knfiltermanager.h"
#include "knfoldermanager.h"
#include "knscoring.h"
#include "knmemorymanager.h"
#include "knmainwidget.h"
#include "scheduler.h"
#include "settings.h"

KNGlobals* KNGlobals::mSelf = 0;
static KStaticDeleter<KNGlobals> staticKNGlobalsDeleter;

KNGlobals::KNGlobals() :
  mInstance( 0 ),
  mScheduler( 0 ),
  mCfgManager( 0 ),
  mAccManager( 0 ),
  mGrpManager( 0 ),
  mArtManager( 0 ),
  mFilManager( 0 ),
  mFolManager( 0 ),
  mScoreManager( 0 ),
  mMemManager( 0 ),
  mSettings( 0 )
{
  kDebug(5003) << k_funcinfo << endl;

  // find knode icons even when running in kontact
  kapp->iconLoader()->addAppDir("knode");
}

KNGlobals::~KNGlobals( )
{
  kDebug(5003) << k_funcinfo << endl;
  delete mScoreManager;
  delete mSettings;
}

KNGlobals * KNGlobals::self()
{
  if ( !mSelf )
    mSelf = staticKNGlobalsDeleter.setObject( mSelf, new KNGlobals() );
  return mSelf;
}


KInstance *KNGlobals::instance() const
{
  if ( mInstance )
    return mInstance;
  return KApplication::kApplication();
}


KConfig* KNGlobals::config()
{
  if (!c_onfig) {
      c_onfig = KSharedConfig::openConfig( "knoderc" );
  }
  return c_onfig.data();
}

KNConfigManager* KNGlobals::configManager()
{
  if (!mCfgManager)
    mCfgManager = new KNConfigManager();
  return mCfgManager;
}

KNode::Scheduler* KNGlobals::scheduler()
{
  if ( !mScheduler )
    mScheduler = new KNode::Scheduler();
  return mScheduler;
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

KNScoringManager* KNGlobals::scoringManager()
{
  if (!mScoreManager)
    mScoreManager = new KNScoringManager();
  return mScoreManager;
}

KNMemoryManager* KNGlobals::memoryManager()
{
  if(!mMemManager)
    mMemManager = new KNMemoryManager();
  return mMemManager;
}


void KNGlobals::setStatusMsg(const QString &text, int id)
{
  if(top)
    top->setStatusMsg(text, id);
}

KNode::Settings * KNGlobals::settings( )
{
  if ( !mSettings ) {
    mSettings = new KNode::Settings();
    mSettings->readConfig();
  }
  return mSettings;
}
