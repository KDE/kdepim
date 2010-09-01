#ifndef kmfolderdir_h
#define kmfolderdir_h

#include <tqstring.h>
#include "kmfoldernode.h"
#include "kmfoldertype.h"

class KMFolder;
class KMFolderMgr;


/** KMail list that manages the contents of one directory that may
 * contain folders and/or other directories.
 */
class KMFolderDir: public KMFolderNode, public KMFolderNodeList
{
  Q_OBJECT

public:
  KMFolderDir( KMFolder * owner, KMFolderDir * parent = 0,
               const TQString& path = TQString::null,
               KMFolderDirType = KMStandardDir );
  virtual ~KMFolderDir();

  virtual bool isDir() const { return true; }

  /**
   * Adds the given subdirectory of this directory to the associated folder.
   */
  void addDirToParent( const TQString &dirName, KMFolder *parentFolder );

  /** Read contents of directory. */
  virtual bool reload();

  /** Return full pathname of this directory. */
  virtual TQString path() const;

  /** Returns the label of the folder for visualization. */
  TQString label() const;

  /** URL of the node for visualization purposes. */
  virtual TQString prettyURL() const;

  /** Create a mail folder in this directory with given name. If sysFldr==TRUE
   the folder is marked as a (KMail) system folder.
   Returns Folder on success. */
  virtual KMFolder* createFolder( const TQString& folderName,
                                  bool sysFldr=false,
                                  KMFolderType folderType=KMFolderTypeMbox );

  /** Returns folder with given name or zero if it does not exist */
  virtual KMFolderNode* hasNamedFolder(const TQString& name);

  /** Returns the folder manager that manages this folder */
  virtual KMFolderMgr* manager() const;

  /** Returns the folder whose children we are holding */
  KMFolder* owner() const { return mOwner; }

  virtual KMFolderDirType type() const { return mDirType; }

protected:
  KMFolder * mOwner;
  KMFolderDirType mDirType;
};


//-----------------------------------------------------------------------------

class KMFolderRootDir: public KMFolderDir
{
  Q_OBJECT

public:
  KMFolderRootDir( KMFolderMgr* manager,
                   const TQString& path=TQString::null,
                   KMFolderDirType dirType = KMStandardDir );
  virtual ~KMFolderRootDir();
  virtual TQString path() const;

  /** set the absolute path */
  virtual void setPath(const TQString&);

  virtual TQString prettyURL() const;

  void setBaseURL( const TQCString& baseURL );

  virtual KMFolderMgr* manager() const;

protected:
  TQString mPath;
  KMFolderMgr *mManager;
  TQCString mBaseURL;
};

#endif /*kmfolderdir_h*/

