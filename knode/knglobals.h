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
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNGLOBALS_H
#define KNGLOBALS_H

#include <kconfig.h>
#include "resource.h"

#include <kdepimmacros.h>

class KInstance;
class KNConfigManager;
class KNNetAccess;
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
class KNArticleWidget;


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
    KXMLGUIClient         *guiClient;
    KNArticleWidget       *artWidget;
    KNArticleFactory      *artFactory;
    Kpgp::Module          *pgp;
    KConfig               *config();
    KInstance             *instance;

    KNConfigManager       *configManager();
    KNNetAccess           *netAccess();
    KNAccountManager      *accountManager();
    KNGroupManager        *groupManager();
    KNArticleManager      *articleManager();
    KNFilterManager       *filterManager();
    KNFolderManager       *folderManager();
    KNScoringManager      *scoringManager();
    KNMemoryManager       *memoryManager();

    /** forwarded to top->setStatusMsg() if available */
    void setStatusMsg(const QString& text = QString::null, int id = SB_MAIN);

private:
    KSharedConfig::Ptr c_onfig;

    KNNetAccess           *mNetAccess;
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
