/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>
   Kevin Ottens <ervin ipsquad net>
   Marcin Przylucki <marcin.przylucki@kdemail.net>
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "obeximpl.h"

#include <kdebug.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kdesktopfile.h>

#include <qapplication.h>
#include <qeventloop.h>
#include <qdir.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <kmimetype.h>

#include <sys/stat.h>

#include "kmobiletools_cfg.h"
#include "devicesconfig.h"


#include "obexwrapper.h"

OBEXImpl::OBEXImpl() : QObject()
    , obexWrapper(0)
{
    KGlobal::dirs()->addResourceType("obex_entries",KStandardDirs::kde_default("data") + "obexview");
    obexWrapper = new OBEXWrapper();
}

void OBEXImpl::setHostConfig(const QString &device, int port, transport i_transport )
{
    obexWrapper->disconnectClient();
    kDebug() << "OBEXImpl::setHostConfig(" << device << ", " << port << ", " << (int) i_transport << ")" << endl;
    kDebug() << "Using transport " << i_transport << endl;
    obexWrapper->setupParameters(i_transport, port, device, UUID_FBS, sizeof(UUID_FBS) , 1, 1);
    obexWrapper->connectClient();
}

bool OBEXImpl::listDirectory(const KUrl &url, Q3ValueList<KIO::UDSEntry> &list)
{
    kDebug() << "OBEXImpl::listDirectory: " << url << endl;
    kDebug() << "OBEXImpl::listDirectory---- directory path only: " << url.path() << endl;

    QString neededPath = url.path();
    
    /* Getting file list from device to "OBEXWrapper::files" class member*/
    if ( !obexWrapper->fetchFileList( neededPath )) return false;

    Q3ValueList<stat_entry_t> obexfiles = obexWrapper->getFiles();
    Q3ValueListIterator<stat_entry_t> fileInfo;

    for(fileInfo=obexfiles.begin(); fileInfo!=obexfiles.end(); ++fileInfo)
    {
        KIO::UDSEntry entry;
        createEntry( entry, url, *fileInfo);
        list.append(entry);
    }

    return true;
}


static void addAtom(KIO::UDSEntry &entry, unsigned int ID, long l,
                    const QString &s = QString() )
{
    KIO::UDSAtom atom;
    atom.m_uds = ID;
    atom.m_long = l;
    atom.m_str = s;
    entry.append(atom);
}


void OBEXImpl::createTopLevelEntry(KIO::UDSEntry &entry) const
{
    entry.clear();
    addAtom(entry, KIO::UDS_NAME, 0, ".");
    addAtom(entry, KIO::UDS_FILE_TYPE, S_IFDIR);
    addAtom(entry, KIO::UDS_ACCESS, 0777);
    addAtom(entry, KIO::UDS_MIME_TYPE, 0, "inode/directory");
    addAtom(entry, KIO::UDS_ICON_NAME, 0, "obex");
    addAtom(entry, KIO::UDS_USER, 0, "root");
    addAtom(entry, KIO::UDS_GROUP, 0, "root");
}

void OBEXImpl::createEntry(KIO::UDSEntry &entry,
                        const KUrl &url,
                        stat_entry_t &file)
{
    QString filename=file.name;
    
    QString directory = /*url.host() +*/ url.path(+1);
    
    KMimeType mimetype=(* KMimeType::findByURL( directory+filename, file.mode, false,  true));
    KDesktopFile desktop(directory+file.name, true);
    kDebug() << "KDesktopFile:: " << desktop.fileName() << endl;

    entry.clear();

    addAtom(entry, KIO::UDS_NAME, 0, file.name);
    addAtom(entry, KIO::UDS_SIZE, file.size);
    //  addAtom(entry, KIO::UDS_URL, 0, "obex:/"+directory + filename);
    //  kDebug() << "*******debug UDS_URL: obex:/" << directory << filename << endl;
    //  addAtom(entry, KIO::UDS_FILE_TYPE, (S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH) );
    addAtom(entry, KIO::UDS_FILE_TYPE, file.mode );
    addAtom(entry, KIO::UDS_CREATION_TIME, file.mtime );
    addAtom(entry, KIO::UDS_MIME_TYPE, 0, mimetype.name() );

    kDebug() << directory+file.name << " mime type: " << mimetype.name() << "; file attributes: " << file.mode << endl;

    QString icon = desktop.readIcon();
    QString empty_icon = desktop.readEntry("EmptyIcon");

    if (!empty_icon.isEmpty())
    {
        KUrl url = desktop.readURL();

        m_lastListingEmpty = true;

        KIO::ListJob *job = KIO::listDir(url, false, false);
        connect( job, SIGNAL( entries(KIO::Job *,
                const KIO::UDSEntryList &) ),
                this, SLOT( slotEntries(KIO::Job *,
                            const KIO::UDSEntryList &) ) );
        connect( job, SIGNAL( result(KIO::Job *) ),
                this, SLOT( slotResult(KIO::Job *) ) );
        qApp->eventLoop()->enterLoop();

        if (m_lastListingEmpty) icon = empty_icon;
    }

    addAtom(entry, KIO::UDS_ICON_NAME, 0, icon);
}


void OBEXImpl::slotEntries(KIO::Job *job, const KIO::UDSEntryList &list)
{
    if (list.size()>0)
    {
        job->kill(true);
        m_lastListingEmpty = false;
        qApp->eventLoop()->exitLoop();
    }
}

void OBEXImpl::slotResult(KIO::Job *)
{
    qApp->eventLoop()->exitLoop();
}


#include "obeximpl.moc"


/*!
    \fn OBEXImpl::fetchFilesList()
*/

void OBEXImpl::fetchFilesList( const QString &path )
{
    obexWrapper->fetchFileList( path );
}

/*!
    \fn OBEXImpl::getFile(const KUrl &url)
*/

int OBEXImpl::getFile(const KUrl &url)
{
    kDebug() << "kio_obex::getFile " << url.path() << endl;
    return obexWrapper->getFile( url.path());
    return 0;
}

/*!
    \fn OBEXImpl::statEntry(const KUrl &url, QValueList<KIO::UDSEntry> &list)
*/

bool OBEXImpl::statEntry(const KUrl &url, KIO::UDSEntry &entry)
{
    kDebug() << "Stat for " << url.path() << endl;

    QString neededPath = url.path().latin1();
    stat_entry_t* currentEntry;
    
    if( !wrapper()->connectClient() ) return false;
    currentEntry = obexftp_stat( wrapper()->getClient(), neededPath);
    if ( !currentEntry ) return false;

    entry.clear();
    createEntry( entry, url, *currentEntry );

    return true;
}
