/*
    This file is part of libkcal.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#include <qobject.h>

namespace KIO {
class Job;
}

class KConfig;

namespace KPIM {

class GroupwareDataAdaptor;

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

    void setAdaptor( KPIM::GroupwareDataAdaptor *adaptor );
    GroupwareDataAdaptor* adaptor() const { return mAdaptor; }

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
    /** Adds the folder with the given url and display name to the folder 
     *  tree (if is has an appropriate type) */ 
    virtual void processFolderResult( const QString &href, 
                                      const QString &displayName, 
                                      KPIM::FolderLister::FolderType  type );
    /** Retrieve information about the folder u. If it has sub-folders, it
        descends into the hierarchy */
    virtual void doRetrieveFolder( const KURL &u );
    /** A subitem was detected. If it's a folder, we need to descend, otherwise
        we just add the url to the list of processed URLs. */
    void folderSubitemRetrieved( const KURL &url, bool isFolder );

  protected:
    /** Creates the job to retrieve information about the folder at the given
        url. It's results will be interpreted by interpretFolderResult
    */
    virtual KIO::Job *createListFoldersJob( const KURL &url );
    /** Interprets the results returned by the liste job (created by
     *  createJob(url) ). The default implementation calls 
     *  interpretFolderListJob of the GroupwareDataAdaptor. Typically, 
     *  this adds an Entry to the mFolders list if the job describes a 
     *  folder of the appropriate type, by calling processsFolderResult.
     *  If the folder has subfolders, just call doRetrieveFolder(url) 
     *  recursively. */
    virtual void interpretListFoldersJob( KIO::Job *job );
    /** List of folders that will always be included (subfolders won't!). 
     *  Usually this is not needed as you should traverse the whole folder 
     *  tree starting from the user's root dir. */
    virtual Entry::List defaultFolders();
    /** Type of this folder lister (i.e. AddressBook or Calendar) */
    Type getType() const { return mType; }


  protected:
    Type mType;
    QStringList mUrls;
    QStringList mProcessedUrls;
    Entry::List mFolders;
    GroupwareDataAdaptor *mAdaptor;
  private:
    // TODO: We need multiple destinations for Events, Tasks and Journals
    QString mWriteDestinationId;
};

}

#endif
