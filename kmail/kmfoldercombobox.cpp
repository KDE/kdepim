/* kmail folder-list combo-box */
/* Author: Ronen Tzur <rtzur@shani.net> */

#include <config.h>

#include "kmfoldercombobox.h"
#include "kmfoldermgr.h"

//-----------------------------------------------------------------------------

KMFolderComboBox::KMFolderComboBox( TQWidget *parent, char *name )
  : TQComboBox( parent, name )
{
  init();
}


//-----------------------------------------------------------------------------

KMFolderComboBox::KMFolderComboBox( bool rw, TQWidget *parent, char *name )
  : TQComboBox( rw, parent, name )
{
  init();
}


//-----------------------------------------------------------------------------

void KMFolderComboBox::init()
{
  mSpecialIdx = -1;
  mOutboxShown = true;
  mImapShown = true;
  refreshFolders();
  connect( this, TQT_SIGNAL( activated(int) ),
      this, TQT_SLOT( slotActivated(int) ) );
  connect( kmkernel->folderMgr(), TQT_SIGNAL(changed()),
      this, TQT_SLOT(refreshFolders()) );
  connect( kmkernel->dimapFolderMgr(), TQT_SIGNAL(changed()),
      this, TQT_SLOT(refreshFolders()) );
  if (mImapShown)
    connect( kmkernel->imapFolderMgr(), TQT_SIGNAL(changed()),
        this, TQT_SLOT(refreshFolders()) );
}


//-----------------------------------------------------------------------------

void KMFolderComboBox::showOutboxFolder(bool shown)
{
  mOutboxShown = shown;
  refreshFolders();
}

//-----------------------------------------------------------------------------

void KMFolderComboBox::showImapFolders(bool shown)
{
  mImapShown = shown;
  refreshFolders();
  if (shown)
    connect( kmkernel->imapFolderMgr(), TQT_SIGNAL(changed()),
        this, TQT_SLOT(refreshFolders()) );
  else
    disconnect( kmkernel->imapFolderMgr(), TQT_SIGNAL(changed()),
        this, TQT_SLOT(refreshFolders()) );
}

//-----------------------------------------------------------------------------

void KMFolderComboBox::createFolderList(TQStringList *names,
                                        TQValueList<TQGuardedPtr<KMFolder> > *folders)
{
  kmkernel->folderMgr()->createI18nFolderList( names, folders );
  if ( !mOutboxShown ) {
    TQValueList< TQGuardedPtr<KMFolder> >::iterator folderIt = folders->begin();
    TQStringList::iterator namesIt = names->begin();
    for ( ; folderIt != folders->end(); ++folderIt, ++namesIt ) {
      KMFolder *folder = *folderIt;
      if ( folder == kmkernel->outboxFolder() )
        break;
    }
    if ( folderIt != folders->end() ) {
      folders->remove( folderIt );
      names->remove( namesIt );
    }
  }

  if (mImapShown)
    kmkernel->imapFolderMgr()->createI18nFolderList( names, folders );

  kmkernel->dimapFolderMgr()->createI18nFolderList( names, folders );
}

//-----------------------------------------------------------------------------

void KMFolderComboBox::refreshFolders()
{
  TQStringList names;
  TQValueList<TQGuardedPtr<KMFolder> > folders;
  createFolderList( &names, &folders );

  KMFolder *folder = getFolder();
  this->clear();
  insertStringList( names );
  setFolder( folder );
}

//-----------------------------------------------------------------------------

void KMFolderComboBox::setFolder( KMFolder *aFolder )
{
  TQStringList names;
  TQValueList<TQGuardedPtr<KMFolder> > folders;
  createFolderList( &names, &folders );

  int idx = folders.findIndex( aFolder );
  if (idx == -1)
    idx = folders.findIndex( kmkernel->draftsFolder() );
  setCurrentItem( idx >= 0 ? idx : 0 );

  mFolder = aFolder;
}

void KMFolderComboBox::setFolder( const TQString &idString )
{
  KMFolder * folder = kmkernel->findFolderById( idString );
  if (!folder && !idString.isEmpty())
  {
     if (mSpecialIdx >= 0)
        removeItem(mSpecialIdx);
     mSpecialIdx = count();
     insertItem(idString, -1);
     setCurrentItem(mSpecialIdx);

     mFolder = 0;
     return;
  }
  setFolder( folder );
}

//-----------------------------------------------------------------------------

KMFolder *KMFolderComboBox::getFolder()
{
  if (mFolder)
    return mFolder;

  TQStringList names;
  TQValueList<TQGuardedPtr<KMFolder> > folders;
  createFolderList( &names, &folders );

  if (currentItem() == mSpecialIdx)
     return 0;

  TQString text = currentText();
  int idx = 0;
  TQStringList::Iterator it;
  for ( it = names.begin(); it != names.end(); ++it ) {
    if ( ! (*it).compare( text ) )
      return *folders.at( idx );
    idx++;
  }

  return kmkernel->draftsFolder();
}

//-----------------------------------------------------------------------------

void KMFolderComboBox::slotActivated(int index)
{
  TQStringList names;
  TQValueList<TQGuardedPtr<KMFolder> > folders;
  createFolderList( &names, &folders );

  if (index == mSpecialIdx)
  {
     mFolder = 0;
  }
  else
  {
     mFolder = *folders.at( index );
  }
}

//-----------------------------------------------------------------------------

#include "kmfoldercombobox.moc"
