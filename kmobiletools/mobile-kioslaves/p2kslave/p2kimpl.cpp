/* This file is part of the KDE project
   Copyright (c) 2004 Kevin Ottens <ervin ipsquad net>
   Rearranged by Marco Gulino <marco@kmobiletools.org> for the mobile protocol

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "p2kimpl.h"

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
#include <qregexp.h>
#include <sys/stat.h>

#include "kmobiletools_cfg.h"
#include "devicesconfig.h"


#include "p2kwrapper.h"

P2KImpl::P2KImpl() : QObject()
{
	KGlobal::dirs()->addResourceType("p2k_entries",
		KStandardDirs::kde_default("data") + "p2kview");
///  p2kWrapper=new P2KWrapper( 0x22b8, 0x4902, 0x4901, "/dev/ttyACM0"); // motorola c650
///  p2kWrapper=new P2KWrapper( 0x22b8, 0x5802, 0x5801, "/dev/ttyACM0"); // motorola c350
 p2kWrapper=new P2KWrapper();
}

bool P2KImpl::listRoot(const KUrl &url, Q3ValueList<KIO::UDSEntry> &list)
{
    kDebug() << "P2KImpl::listRoot" << endl;
    if(p2kWrapper->connectPhone() ) kDebug() << "Files count: " << p2kWrapper->countFiles() << endl;
    QStringList names_found;
    QStringList dirList = p2kWrapper->getRoot();
    kDebug() << "P2KImpl::" << dirList << endl;
    QStringList::ConstIterator dirpath = dirList.begin();
    QStringList::ConstIterator end = dirList.end();
    for(; dirpath!=end; ++dirpath)
    {
        KIO::UDSEntry entry;
        entry.clear();
        createEntry( entry, url.url(-1), QString(*dirpath).replace("//", "/"), true );
        list.append(entry);
        names_found.append(*dirpath);
    }

    return true;
}

bool P2KImpl::listDirectory(const KUrl &url, Q3ValueList<KIO::UDSEntry> &list)
{
    if (!p2kWrapper->getFiles().count() ) p2kWrapper->fetchFileList();
    kDebug() << "P2KImpl::listDirectory: " << url << endl;
    kDebug() << "P2KImpl::listDirectory---- directory path only: " << url.path() << endl;
    QStringList names_found;
    QStringList dirList = p2kWrapper->getDirs();
//     dirList << "/b" << "/b/prova";
//     kDebug() << "Directory list: " << dirList << endl;
    QRegExp regexp;
    regexp.setPattern(QString("^").append(QRegExp::escape(url.path()) ) );
    names_found=dirList.grep(regexp);
    QString foundstr;
    for(QStringList::Iterator it=names_found.begin(); it!=names_found.end(); ++it)
    {
        if((*it)==url.path()) continue;
        foundstr=(*it).replace(regexp, "");
        if(foundstr[foundstr.length()-1]==QDir::separator()) foundstr=foundstr.left(foundstr.length()-2);
        if(foundstr[0]==QDir::separator()) foundstr=foundstr.mid(1);
//         kDebug() << "P2KImpl Listing directory `" << foundstr << "` in " << url.path() << endl;
        if(foundstr.contains(QDir::separator() )) continue;
//         foundstr=foundstr.remove(url.path());
        KIO::UDSEntry entry;
        createEntry( entry, url.url(), foundstr );
        list.append(entry);
    }
    Q3ValueList<p2k_fileInfo> p2kfiles=p2kWrapper->getFiles();
    Q3ValueListIterator<p2k_fileInfo> fileInfo;
    QString fname;
    for(fileInfo=p2kfiles.begin(); fileInfo!=p2kfiles.end(); ++fileInfo)
    {
        fname=(*fileInfo).name;
        if(!fname.contains(regexp) ) continue;
//         kDebug() << "P2KImpl Listing files: " << fname.replace(regexp, "").mid(1) << endl;
        if(fname.replace(regexp, "").mid(1).contains(QDir::separator() ) ) continue;
        kDebug() << "Listing files: `" << (*fileInfo).name << "`" << endl;
        KIO::UDSEntry entry;
        createEntry(entry, url.url() + QDir::separator() + KUrl((*fileInfo).name).fileName(), *fileInfo);
        list.append(entry);
    }

    /*
//     kDebug() << "P2KImpl::" << dirList << endl;
//     QStringList::ConstIterator dirpath = dirList.begin();
//     QStringList::ConstIterator end = dirList.end();
    QString s_entry;
    for(QStringList::Iterator dirpath=dirList.begin(); dirpath!=dirList.end(); ++dirpath)
    {
        s_entry=(*dirpath);
        if( ! s_entry.contains( url.path() ) ) continue;
        s_entry=s_entry.replace( 0,url.path().length(), "").mid(1);
        if(! s_entry.length() || s_entry.contains('/') ) continue;
        kDebug() << "************ Creating entry " << s_entry << endl;
        KIO::UDSEntry entry;
        entry.clear();
//         QString dir_found=directory.append("/").append(*dirpath);
//         if(!dir_found.find('/') ) dir_found=dir_found.mid(1);
        createEntry( entry, url.host().prepend("/") + "/" + *dirpath, s_entry);
        list.append(entry);
        names_found.append(*dirpath);
    }
    QValueList<p2k_fileInfo> p2kfiles=p2kWrapper->getFiles();
    QValueListIterator<p2k_fileInfo> fileInfo;

    for(fileInfo=p2kfiles.begin(); fileInfo!=p2kfiles.end(); ++fileInfo)
    {
        s_entry=(*fileInfo).name;
        if( ! s_entry.contains( url.path() ) ) continue;
        s_entry=s_entry.replace( 0,url.path().length(), "").mid(1);
        if(! s_entry.length() || s_entry.contains('/') ) continue;
        KIO::UDSEntry entry;
        entry.clear();
//         QString dir_found=directory.append("/").append(*dirpath);
//         if(!dir_found.find('/') ) dir_found=dir_found.mid(1);
        createEntry( entry, url.host().prepend("/") + "/" + url.path() , *fileInfo);
        list.append(entry);
        names_found.append((*fileInfo).name);
    }
    */
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


void P2KImpl::createTopLevelEntry(KIO::UDSEntry &entry) const
{
	entry.clear();
	addAtom(entry, KIO::UDS_NAME, 0, ".");
	addAtom(entry, KIO::UDS_FILE_TYPE, S_IFDIR);
	addAtom(entry, KIO::UDS_ACCESS, 0777);
	addAtom(entry, KIO::UDS_MIME_TYPE, 0, "inode/directory");
	addAtom(entry, KIO::UDS_ICON_NAME, 0, "p2k");
	addAtom(entry, KIO::UDS_USER, 0, "root");
	addAtom(entry, KIO::UDS_GROUP, 0, "root");
    addAtom(entry, KIO::UDS_NAME, 0, ".");
}

void P2KImpl::createEntry(KIO::UDSEntry &entry, const KUrl &url, p2k_fileInfo &file)
{
    kDebug() << "Using ***NEW*** createEntry `" << url.url() << "`\n";
    QString filepath=url.path();
    QString filename=url.fileName();
    QString directory = /*url.host() +*/ url.directory();
    kDebug() << "File name: `" << filename << "`; file path: `" << filepath << "`; directory: `" << directory << "`\n";
    
    KMimeType mimetype=(* KMimeType::findByURL( filepath, file.attr, false,  true));
    KDesktopFile desktop(filepath, true);
    kDebug() << "KDesktopFile:: " << desktop.fileName() << endl;

    entry.clear();

    addAtom(entry, KIO::UDS_NAME, 0, filename);
    addAtom(entry, KIO::UDS_SIZE, file.size);
    addAtom(entry, KIO::UDS_URL, 0, url.url() );
    //  kDebug() << "*******debug UDS_URL: obex:/" << directory << filename << endl;
    //  addAtom(entry, KIO::UDS_FILE_TYPE, (S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH) );
    addAtom(entry, KIO::UDS_FILE_TYPE, /*file.attr*/ S_IFREG );
//     addAtom(entry, KIO::UDS_CREATION_TIME, file.mtime );
    addAtom(entry, KIO::UDS_MIME_TYPE, 0, mimetype.name() );

    kDebug() << filepath << " mime type: " << mimetype.name() << "; file attributes: " << QString::number(file.attr) << endl;

    QString icon = desktop.readIcon();
    QString empty_icon = desktop.readEntry("EmptyIcon");

 /*   if (!empty_icon.isEmpty())
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
                                       */
    addAtom(entry, KIO::UDS_ICON_NAME, 0, icon);
}


void P2KImpl::createEntry(KIO::UDSEntry &entry,
                          const QString &directory,
                          const QString &file, bool isDrive)
{
    kDebug() << "P2KImpl::createEntry: " << directory << "<->" << file << endl;

    KDesktopFile desktop(directory+file, true);

    kDebug() << "path = " << directory << file << endl;

    entry.clear();

    addAtom(entry, KIO::UDS_NAME, 0, file);
    addAtom(entry, KIO::UDS_URL, 0,directory + QDir::separator() + file);

    addAtom(entry, KIO::UDS_FILE_TYPE, S_IFDIR);
    if(isDrive)
        addAtom(entry, KIO::UDS_MIME_TYPE, 0, "media/hdd_mounted");
    else addAtom(entry, KIO::UDS_MIME_TYPE, 0, "inode/directory");

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

void P2KImpl::createEntry(KIO::UDSEntry &entry,
                          const QString &directory,
                          p2k_fileInfo &file)
{
    QString filename=file.name;
    filename=filename.mid(filename.findRev('/') +1);
    KMimeType mimetype=(* KMimeType::findByPath(filename, 0, true));
    KDesktopFile desktop(directory+file.name, true);

    entry.clear();

    addAtom(entry, KIO::UDS_NAME, 0, filename);
    addAtom(entry, KIO::UDS_SIZE, file.size);
//     addAtom(entry, KIO::UDS_URL, 0, "p2k:/"+directory + filename);
//     kDebug() << "*******debug UDS_URL: p2k:/" << directory << filename << endl;
    addAtom(entry, KIO::UDS_FILE_TYPE, (S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH) );
    // Attributes: 0=nothing, 2=hidden, 4=system, 1=readonly
//     if (file.attr & 1 ) addAtom(entry, KIO::UDS_ACCESS, 0555);
//     if (file.attr & 2 ) addAtom( entry, KIO::UDS_HIDDEN, true);
//     if (file.attr & 4 );
    addAtom(entry, KIO::UDS_MIME_TYPE, 0, mimetype.name() );
    kDebug() << filename << " mime type: " << mimetype.name() << "; file attributes: " << file.attr << endl;
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

void P2KImpl::slotEntries(KIO::Job *job, const KIO::UDSEntryList &list)
{
	if (list.size()>0)
	{
		job->kill(true);
		m_lastListingEmpty = false;
		qApp->eventLoop()->exitLoop();
	}
}

void P2KImpl::slotResult(KIO::Job *)
{
	qApp->eventLoop()->exitLoop();
}


#include "p2kimpl.moc"


/*!
    \fn P2KImpl::fetchFilesList()
 */
void P2KImpl::fetchFilesList()
{
    p2kWrapper->fetchFileList();
}


/*!
    \fn P2KImpl::getFile(const KUrl &url)
 */
int P2KImpl::getFile(const KUrl &url, char* buffer)
{
    kDebug() << "kio_p2k::getFile " << url.path() << endl;
    return p2kWrapper->getFile( url.path(), buffer );
}


/*!
    \fn P2KImpl::statEntry(const KUrl &url, QValueList<KIO::UDSEntry> &list)
 */
bool P2KImpl::statEntry(const KUrl &url, KIO::UDSEntry &entry)
{
    kDebug() << "Stat for " << url.path() << endl;

    Q3ValueList<p2k_fileInfo> fileList=p2kWrapper->getFiles();
    Q3ValueListIterator<p2k_fileInfo> files_it;
    bool found=false;
    for(files_it=fileList.begin(); files_it!=fileList.end(); ++files_it)
    {
        if( url.path()==(*files_it).name )
        {
            found=true;
            break;
        }
    }
    if ( ! found ) return false;
    QString s_entry=(*files_it).name;
    kDebug() << "s_entry(1) == " << s_entry << endl;
    if( ! s_entry.contains( url.path() ) ) return false;
    /*s_entry=s_entry.replace( 0,url.path().length(), "").mid(1);
    kDebug() << "s_entry(2) == " << s_entry << endl;

    if(! s_entry.length() || s_entry.contains('/') ) return false;*/
    s_entry=QStringList::split('/', s_entry).last();
    kDebug() << "OK, creating entry " << s_entry << endl;
    entry.clear();
//         QString dir_found=directory.append("/").append(*dirpath);
//         if(!dir_found.find('/') ) dir_found=dir_found.mid(1);
    createEntry( entry, /*url.host().prepend("/") + "/" + url.path()*/ url , (*files_it) );
    return true;
}
