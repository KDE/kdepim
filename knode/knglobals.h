/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2006 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNGLOBALS_H
#define KNGLOBALS_H

#include "knode_export.h"
#include "resource.h"

#include <kconfig.h>
#include <kcomponentdata.h>

/**
  Keep compatibility with the old way.
  @deprecated Use KNGlobals::self().
*/
#define knGlobals (*KNGlobals::self())

class KComponentData;
class KNConfigManager;
class KNAccountManager;
class KNGroupManager;
class KNArticleManager;
class KNArticleFactory;
class KNFolderManager;
class QWidget;
class KNFilterManager;
class KNMainWidget;
class KNScoringManager;
class KNMemoryManager;
class KXMLGUIClient;
namespace KNode {
  class Scheduler;
  class Settings;
}


/** idea: Previously the manager classes were available
    via KNodeApp. Now they can be accessed directly,
    this removes many header dependencies.
    (knode.h isn't include everywhere) */
class KNODE_EXPORT KNGlobals
{
  friend class KNGlobalsPrivate;

  public:
    /** Return the KNGlobals instance. */
    static KNGlobals *self();

    /** topWidget == top, used for message boxes, */
    QWidget               *topWidget;
    /** no need to include knode.h everywhere */
    KNMainWidget          *top;
    /** Returns the KXMLGUIClient of the main window. */
    KXMLGUIClient         *guiClient;
    /** Returns the article factory. */
    KNArticleFactory      *artFactory;
    /** Returns KNode's main configuration. */
    KConfig               *config();
    /** Returns the current instance. */
    const KComponentData &componentData() const;
    /** Sets the current instance. */
    void setComponentData( const KComponentData &inst ) { mInstance = inst; }

    KNConfigManager       *configManager();
    /** Returns the scheduler. */
    KNode::Scheduler      *scheduler();
    /** Returns the account manager. */
    KNAccountManager      *accountManager();
    /** Returns the group manager. */
    KNGroupManager        *groupManager();
    /** Returns the article manager.  */
    KNArticleManager      *articleManager();
    /** Returns the filter manager. */
    KNFilterManager       *filterManager();
    /** Returns the folder manager. */
    KNFolderManager       *folderManager();
    /** Returns the scoring manager. */
    KNScoringManager      *scoringManager();
    /** Returns the memory manager. */
    KNMemoryManager       *memoryManager();
    /** Returns the KConfigXT generated settings object. */
    KNode::Settings *settings();

    /** forwarded to top->setStatusMsg() if available */
    void setStatusMsg(const QString& text = QString(), int id = SB_MAIN);

  private:
    /** Create a new KNGlobals object, should only be called by the friend class KNGlobalsPrivate. */
    KNGlobals();
    /** Destroy the KNGlobals. */
    ~KNGlobals();

    KSharedConfig::Ptr c_onfig;

    KComponentData mInstance;
    KNode::Scheduler      *mScheduler;
    KNConfigManager       *mCfgManager;
    KNAccountManager      *mAccManager;
    KNGroupManager        *mGrpManager;
    KNArticleManager      *mArtManager;
    KNFilterManager       *mFilManager;
    KNFolderManager       *mFolManager;
    KNScoringManager *mScoreManager;
    KNMemoryManager       *mMemManager;
    KNode::Settings *mSettings;
};

#endif
