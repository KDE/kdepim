/*
    knglobals.h

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

#ifndef KNGLOBALS_H
#define KNGLOBALS_H

#include <kconfig.h>
#include "resource.h"

#include <kdepimmacros.h>

class KInstance;
class KNConfigManager;
class KNProgress;
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
namespace Kpgp {
   class Module;
}
namespace KNode {
  class ArticleWidget;
  class Scheduler;
}


/** idea: Previously the manager classes were available
    via KNodeApp. Now they can be accessed directly,
    this removes many header dependencies.
    (knode.h isn't include everywhere) */
class KDE_EXPORT KNGlobals {
  public:
    /** topWidget == top, used for message boxes, */
    QWidget               *topWidget;
    /** no need to include knode.h everywhere */
    KNMainWidget          *top;
    /** Returns the KXMLGUIClient of the main window. */
    KXMLGUIClient         *guiClient;
    /** Returns the article widget of the main window. */
    KNode::ArticleWidget  *artWidget;
    /** Returns the article factory. */
    KNArticleFactory      *artFactory;
    Kpgp::Module          *pgp;
    KConfig               *config();
    KInstance             *instance;

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

    /** forwarded to top->setStatusMsg() if available */
    void setStatusMsg(const QString& text = QString::null, int id = SB_MAIN);

private:
    KSharedConfig::Ptr c_onfig;

    KNode::Scheduler      *mScheduler;
    KNConfigManager       *mCfgManager;
    KNAccountManager      *mAccManager;
    KNGroupManager        *mGrpManager;
    KNArticleManager      *mArtManager;
    KNFilterManager       *mFilManager;
    KNFolderManager       *mFolManager;
    static KNScoringManager  *mScoreManager;
    KNMemoryManager       *mMemManager;
};


extern KNGlobals knGlobals KDE_EXPORT;

#endif
