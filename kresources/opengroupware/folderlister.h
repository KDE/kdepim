/*
    This file is part of libkcal.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
    
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
    
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KPIM_FOLDERLISTER_H
#define KPIM_FOLDERLISTER_H

#include <kurl.h>

#include <qvaluelist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdom.h>
#include <qobject.h>

namespace KIO {
class DavJob;
class Job;
}

class KConfig;

namespace KPIM {

class FolderLister : public QObject
{
    Q_OBJECT
  public:
    enum Type { AddressBook, Calendar };
    enum FolderType { ContactsFolder, CalendarFolder, TasksFolder, 
                      JournalsFolder, AllIncidencesFolder, 
                      MailFolder, MemoFolder, Folder, Unknown };
  
    class Entry
    {
      public:
        Entry() : active( false ) {}
      
        typedef QValueList<Entry> List;
      
        QString id;
        QString name;
        FolderType type;
        bool active;
    };

    FolderLister( Type );
  
    /** Initialize the retrieval with given root URL */
    virtual void retrieveFolders( const KURL & );
  
    void setFolders( const Entry::List & );
    Entry::List folders() const { return mFolders; }

    QStringList activeFolderIds() const;
    bool isActive( const QString &id ) const;

    void setWriteDestinationId( const QString & );
    QString writeDestinationId() const { return mWriteDestinationId; }

    void readConfig( const KConfig * );
    void writeConfig( KConfig * );
    
  signals:
    void foldersRead();

  protected slots:
    void slotListJobResult( KIO::Job * );

  protected:
    /** Retrieve information about the folder u. If it has sub-folders, it
        descends into the hierarchy */
    virtual void doRetrieveFolder( const KURL &u );
    /** Adjusts the given url, e.g. changes http:// -> webdav://, etc. */
    virtual KURL adjustUrl( const KURL &u );
    /** Creates the job to retrieve information about the folder at the given
        url. It is expected that the job retrieves at least the following props:
          DAV:displayname, DAV:resourcetype, DAV:hassubs
    */
    virtual KIO::DavJob *createJob( const KURL &url );
    /** Returns the type of folder retrieved in the dom node. Typically, you'll
        compare the DAV:resourcetype property with some values. */
    virtual FolderType getFolderType( const QDomNode &folderNode ) = 0;
    /** Interprets the results returned by the liste job (created by 
        createJob(url) ). Typically, this adds an Entry to the mFolders list if
        the job describes a folder of the appropriate type. If the folder has 
        subfolders, just call doRetrieveFolder(url) recursively. */
    virtual void interpretFolderResult( KIO::Job *job );
    /** List of folders that will always be included (subfolders won't!). Usually
        this is not needed as you should traverse the whole folder tree starting
        from the user's root dir. */
    virtual Entry::List defaultFolders();
    /** Type of this folder lister (i.e. AddressBook or Calendar) */
    Type getType() const { return mType; }

  private:
    Type mType;
    QStringList mUrls;
    QStringList mProcessedUrls;
    Entry::List mFolders;
    QString mWriteDestinationId;
    QString mUser;
    QString mPassword;
};

}

#endif
