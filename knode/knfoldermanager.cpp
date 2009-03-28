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

#include <QByteArray>
#include <QDir>
#include <QTextStream>

#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include "knglobals.h"
#include "knconfigmanager.h"
#include "knfolder.h"
#include "utilities.h"
#include "knfoldermanager.h"
#include "knarticlemanager.h"
#include "kncleanup.h"
#include "knmemorymanager.h"
#include "knmainwidget.h"


KNFolderManager::KNFolderManager(KNArticleManager *a) : a_rtManager(a)
{
  //standard folders
  QString dir( KStandardDirs::locateLocal( "data", "knode/folders/" ) );
  if (dir.isNull()) {
    KNHelper::displayInternalFileError();
    return;
  }

  KNFolder *f;

  f=new KNFolder(0, i18n("Local Folders"), "root");
  mFolderList.append( f );
  f->readInfo();

  f=new KNFolder(1, i18n("Drafts"), "drafts", root());
  mFolderList.append( f );
  f->readInfo();

  f=new KNFolder(2, i18n("Outbox"), "outbox", root());
  mFolderList.append( f );
  f->readInfo();

  f=new KNFolder(3, i18n("Sent"), "sent", root());
  mFolderList.append( f );
  f->readInfo();

  l_astId=3;

  //custom folders
  loadCustomFolders();

  setCurrentFolder(0);
}


KNFolderManager::~KNFolderManager()
{
  qDeleteAll( mFolderList );
}


void KNFolderManager::setCurrentFolder(KNFolder *f)
{
  c_urrentFolder=f;
  a_rtManager->setFolder(f);

  kDebug(5003) <<"KNFolderManager::setCurrentFolder() : folder changed";

  if(f && !f->isRootFolder()) {
    if( loadHeaders(f) )
      a_rtManager->showHdrs();
    else
      KMessageBox::error(knGlobals.topWidget, i18n("Cannot load index-file."));
  }
}


bool KNFolderManager::loadHeaders(KNFolder *f)
{
  if( !f || f->isRootFolder() )
    return false;

  if (f->isLoaded())
    return true;

  // we want to delete old stuff first => reduce vm fragmentation
  knGlobals.memoryManager()->prepareLoad(f);

  if (f->loadHdrs()) {
    knGlobals.memoryManager()->updateCacheEntry( f );
    return true;
  }

  return false;
}


bool KNFolderManager::unloadHeaders(KNFolder *f, bool force)
{
  if(!f || !f->isLoaded())
    return false;

  if (!force && (c_urrentFolder == f))
    return false;

  if (f->unloadHdrs(force))
    knGlobals.memoryManager()->removeCacheEntry(f);
  else
    return false;

  return true;
}


KNFolder* KNFolderManager::folder(int i)
{
  for ( List::Iterator it = mFolderList.begin(); it != mFolderList.end(); ++it )
    if ( (*it)->id() == i )
      return (*it);
  return 0;
}


KNFolder* KNFolderManager::newFolder(KNFolder *p)
{
  if (!p)
    p = root();
  KNFolder *f=new KNFolder(++l_astId, i18n("New folder"), p);
  mFolderList.append( f );
  emit folderAdded(f);
  return f;
}


bool KNFolderManager::deleteFolder(KNFolder *f)
{
  if(!f || f->isRootFolder() || f->isStandardFolder() || f->lockedArticles()>0)
    return false;

  List del;
  KNCollection *p;

  // find all subfolders of the folder we want to delete
  for ( List::Iterator it = mFolderList.begin(); it != mFolderList.end(); ++it ) {
    p = (*it)->parent();
    while ( p ) {
      if ( p == f ) {
        if ( (*it)->lockedArticles() > 0 )
          return false;
        del.append( (*it) );
        break;
      }
      p = p->parent();
    }
  }

  emit folderRemoved(f);

  del.append(f);
  for ( List::Iterator it = del.begin(); it != del.end(); ++it ) {
    if ( c_urrentFolder == (*it) )
      c_urrentFolder = 0;

    if ( unloadHeaders( (*it), true ) ) {
      (*it)->deleteFiles();
      mFolderList.removeAll( (*it) );
      delete (*it);
    } else
      return false;
  }

  return true;
}


void KNFolderManager::emptyFolder(KNFolder *f)
{
  if(!f || f->isRootFolder())
    return;
  knGlobals.memoryManager()->removeCacheEntry(f);
  f->deleteAll();
}


bool KNFolderManager::moveFolder(KNFolder *f, KNFolder *p)
{
  if(!f || p==f->parent()) // nothing to be done
    return true;

  // is "p" a child of "f" ?
  KNCollection *p2=p?p->parent():0;
  while(p2) {
    if(p2==f)
      break;
    p2=p2->parent();
  }

  if( (p2 && p2==f) || f==p || f->isStandardFolder() || f->isRootFolder()) //  no way ;-)
    return false;

  emit folderRemoved(f);

  // reparent
  f->setParent(p);
  f->saveInfo();

  emit folderAdded(f);

  if(c_urrentFolder==f)
    emit folderActivated(f);

  return true;
}


int KNFolderManager::unsentForAccount(int accId)
{
  int cnt=0;

  for ( List::Iterator it = mFolderList.begin(); it != mFolderList.end(); ++it ) {
    for ( int idx = 0; idx < (*it)->length(); ++idx ) {
      KNLocalArticle *a = (*it)->at( idx );
      if ( a->serverId() == accId && a->doPost() && !a->posted() )
        cnt++;
    }
  }

  return cnt;
}


void KNFolderManager::compactFolder(KNFolder *f)
{
  if(!f || f->isRootFolder())
    return;

  KNCleanUp cup;
  cup.compactFolder(f);
}


void KNFolderManager::compactAll(KNCleanUp *cup)
{
  for ( List::Iterator it = mFolderList.begin(); it != mFolderList.end(); ++it )
    if ( !(*it)->isRootFolder() && (*it)->lockedArticles() == 0 )
      cup->appendCollection( (*it) );
}


void KNFolderManager::compactAll()
{
  KNCleanUp *cup = new KNCleanUp();
  compactAll( cup );
  cup->start();

  knGlobals.configManager()->cleanup()->setLastCompactDate();
  delete cup;
}


void KNFolderManager::importFromMBox(KNFolder *f)
{
  if(!f || f->isRootFolder())
    return;

  f->setNotUnloadable(true);

  if (!f->isLoaded() && !loadHeaders(f)) {
    f->setNotUnloadable(false);
    return;
  }

  KNLoadHelper helper(knGlobals.topWidget);
  QFile *file = helper.getFile(i18n("Import MBox Folder"));
  KNLocalArticle::List list;
  KNLocalArticle *art;
  QByteArray str;
  int artStart=0, artEnd=0;
  bool done=true;

  if (file) {
    knGlobals.top->setCursorBusy(true);
    knGlobals.setStatusMsg(i18n(" Importing articles..."));
    knGlobals.top->secureProcessEvents();

    if (!file->atEnd()) {                // search for the first article...
      str = file->readLine();
      if ( str.left( 5 ) == "From " ) {
        artStart = file->pos();
        done = false;
      } else {
        artStart = KNHelper::findStringInFile( file, "\n\nFrom " );
        if (artStart != -1) {
          file->seek( artStart + 1 );
          str = file->readLine();
          artStart = file->pos();
          done = false;
        }
      }
    }

    knGlobals.top->secureProcessEvents();

    if (!done) {
      while (!file->atEnd()) {
        artEnd = KNHelper::findStringInFile( file, "\n\nFrom " );

        if (artEnd != -1) {
          file->seek( artStart );    // seek the first character of the article
          int size=artEnd-artStart;
          QByteArray buffer;
          buffer = file->read( size);

          if ( !buffer.isEmpty() ) {
            art = new KNLocalArticle(0);
            art->setEditDisabled(true);
            art->setContent( buffer );
            art->parse();
            list.append(art);
          }

          file->seek( artEnd + 1 );
          str = file->readLine();
          artStart = file->pos();
        } else {
          if ((int)file->size() > artStart) {
            file->seek( artStart );    // seek the first character of the article
            int size=file->size()-artStart;
            QByteArray buffer;
            buffer = file->read( size );

            if ( !buffer.isEmpty() ) {
              art = new KNLocalArticle(0);
              art->setEditDisabled(true);
              art->setContent( buffer );
              art->parse();
              list.append(art);
            }
          }
        }

        if (list.count()%75 == 0)
          knGlobals.top->secureProcessEvents();
      }
    }

    knGlobals.setStatusMsg(i18n(" Storing articles..."));
    knGlobals.top->secureProcessEvents();

    if (!list.isEmpty())
      knGlobals.articleManager()->moveIntoFolder(list, f);

    knGlobals.setStatusMsg( QString() );
    knGlobals.top->setCursorBusy(false);
  }

  f->setNotUnloadable(false);
}


void KNFolderManager::exportToMBox(KNFolder *f)
{
  if(!f || f->isRootFolder())
    return;

  f->setNotUnloadable(true);

  if (!f->isLoaded() && !loadHeaders(f)) {
    f->setNotUnloadable(false);
    return;
  }

  KNSaveHelper helper(f->name()+".mbox", knGlobals.topWidget);
  QFile *file = helper.getFile(i18n("Export Folder"));

  if (file) {
    knGlobals.top->setCursorBusy(true);
    knGlobals.setStatusMsg(i18n(" Exporting articles..."));
    knGlobals.top->secureProcessEvents();

    QTextStream ts(file);
    ts.setCodec( "ISO 8859-1" );
    KNLocalArticle *a;

    for(int idx=0; idx<f->length(); idx++) {
      a=f->at(idx);

      a->setNotUnloadable(true);

      if (a->hasContent() || knGlobals.articleManager()->loadArticle(a)) {
        ts << "From aaa@aaa Mon Jan 01 00:00:00 1997\n";
        a->toStream(ts, true);
        ts << "\n";
      }

      a->setNotUnloadable(false);

      if (idx%75 == 0)
        knGlobals.top->secureProcessEvents();
    }

    knGlobals.setStatusMsg( QString() );
    knGlobals.top->setCursorBusy(false);
  }
}


void KNFolderManager::syncFolders()
{
  QString dir( KStandardDirs::locateLocal( "data", "knode/folders/" ) );
  if (dir.isNull()) {
    KNHelper::displayInternalFileError();
    return;
  }

  //sync
  for ( List::Iterator it = mFolderList.begin(); it != mFolderList.end(); ++it ) {
    if ( !(*it)->isRootFolder() )
      (*it)->syncIndex();
    (*it)->saveInfo();
  }
}


int KNFolderManager::loadCustomFolders()
{
  int cnt=0;
  QString dir( KStandardDirs::locateLocal( "data", "knode/folders/" ) );
  KNFolder *f;

  if (dir.isNull()) {
    KNHelper::displayInternalFileError();
    return 0;
  }

  QDir d(dir);
  QStringList entries(d.entryList(QStringList("custom_*.info")));  // ignore info files of standard folders
  for(QStringList::Iterator it=entries.begin(); it != entries.end(); ++it) {
    f=new KNFolder();
    if(f->readInfo(d.absoluteFilePath(*it))) {
      if(f->id()>l_astId)
        l_astId=f->id();
      mFolderList.append( f );
      cnt++;
    }
    else
      delete f;
  }

  // set parents
  if(cnt>0) {
    for ( List::Iterator it = mFolderList.begin(); it != mFolderList.end(); ++it ) {
      if ( !(*it)->isRootFolder() ) {   // the root folder has no parent
        KNFolder *par = folder( (*it)->parentId() );
        if ( !par )
          par = root();
        (*it)->setParent( par );
      }
    }
  }

  return cnt;
}


#include "knfoldermanager.moc"
