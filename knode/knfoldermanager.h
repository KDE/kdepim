/*
    knfoldermanager.h

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

#ifndef KNFOLDERMANAGER_H
#define KNFOLDERMANAGER_H

#include <qglobal.h>
#if QT_VERSION >= 290
#  include <qptrlist.h>
#else
#  include <qlist.h>
#  define QPtrList QList
#  define QPtrListIterator QListIterator
#endif

class KNListView;
class KNFolder;
class KNArticleManager;
class KNCleanUp;


class KNFolderManager
{
  public:
    KNFolderManager(KNListView *v, KNArticleManager *a);
    ~KNFolderManager();

    //folder access
    void setCurrentFolder(KNFolder *f);
    KNFolder* currentFolder()             { return c_urrentFolder; }
    bool hasCurrentFolder()               { return (c_urrentFolder!=0); }
    KNFolder* folder(int id);

    //standard folders
    KNFolder* root()                      { return f_List.at(0); }
    KNFolder* drafts()                    { return f_List.at(1); }
    KNFolder* outbox()                    { return f_List.at(2); }
    KNFolder* sent()                      { return f_List.at(3); }

    //header loading
    bool loadHeaders(KNFolder *f);
    bool unloadHeaders(KNFolder *f, bool force=true);
    bool loadDrafts()                     { return loadHeaders(drafts()); }
    bool loadOutbox()                     { return loadHeaders(outbox()); }
    bool loadSent()                       { return loadHeaders(sent()); }

    // returns the new folder
    KNFolder* newFolder(KNFolder *p);
    bool deleteFolder(KNFolder *f);
    void emptyFolder(KNFolder *f);
    bool moveFolder(KNFolder *f, KNFolder *p);

    //unsent articles
    int unsentForAccount(int accId);

    //compacting
    void compactFolder(KNFolder *f);
    void compactAll(KNCleanUp *cup);
    void compactAll();

    // import + export
    void importFromMBox(KNFolder *f);
    void exportToMBox(KNFolder *f);

    //synchronization
    void syncFolders();   

  protected:
    int loadCustomFolders();
    void showListItems();
    void createListItem(KNFolder *f);
        
    KNFolder  *c_urrentFolder;
    QPtrList<KNFolder> f_List;
    int l_astId;
    KNListView *v_iew;
    KNArticleManager *a_rtManager;

};

#endif
