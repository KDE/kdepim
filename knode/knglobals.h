/***************************************************************************
                     knglobals.h - description
 copyright            : (C) 1999 by Christian Thurner
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

#ifndef KNGLOBALS_H
#define KNGLOBALS_H

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
class KNMainWindow;
class KNodeView;

// idea: Previously the manager classes were available
//       via KNodeApp. Now they can be accessed directly,
//       this removes many header dependencies.
//       (knode.h isn't include everywhere)
class KNGlobals {

  public:

    QWidget               *topWidget;    // topWidget == top, used for message boxes,
    KNMainWindow          *top;          // no need to include knode.h everywhere
    KNConfigManager       *cfgManager;
    KNNetAccess           *netAccess;
    KNProgress            *progressBar;
    KNAccountManager      *accManager;
    KNGroupManager        *grpManager;
    KNArticleManager      *artManager;
    KNArticleFactory      *artFactory;
    KNFolderManager       *folManager;
    KNFilterManager       *filManager;
    KNodeView             *view;

};


extern KNGlobals knGlobals;

#endif








